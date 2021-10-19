/*

Copyright 2015-2021 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifdef USE_UART

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/general/RingBuffer.h"
#include "core/src/general/Helpers.h"
#include <MCU.h>

#ifndef USB_SUPPORTED
#include "board/common/comm/USBOverSerial/USBOverSerial.h"
#include "usb-link/Commands.h"
#endif

// generic UART driver, arch-independent

namespace
{
    /// Flag determining whether or not UART loopback functionality is enabled.
    /// When enabled, all incoming UART traffic is immediately passed on to UART TX.
    volatile bool _loopbackEnabled[MAX_UART_INTERFACES];

    /// Flag signaling that the transmission is done.
    volatile bool _txDone[MAX_UART_INTERFACES];

    /// Flag holding the state of UART interface (whether it's _initialized or not).
    bool _initialized[MAX_UART_INTERFACES];

    /// Buffer in which outgoing UART data is stored.
    core::RingBuffer<uint8_t, UART_TX_BUFFER_SIZE> _txBuffer[MAX_UART_INTERFACES];

    /// Buffer in which incoming UART data is stored.
    core::RingBuffer<uint8_t, UART_RX_BUFFER_SIZE> _rxBuffer[MAX_UART_INTERFACES];

#ifndef USB_SUPPORTED
    /// Holds the USB state received from USB link MCU
    bool _usbConnectionState = false;
#endif

#ifdef DMX_SUPPORTED
    uint8_t _dmxBuffer[513];
#endif

    /// Starts the process of transmitting the data from UART TX buffer to UART interface.
    /// param [in]: channel     UART channel on MCU.
    void uartTransmitStart(uint8_t channel)
    {
        if (channel >= MAX_UART_INTERFACES)
            return;

        _txDone[channel] = false;

        Board::detail::UART::ll::enableDataEmptyInt(channel);
    }
}    // namespace

namespace Board
{
    namespace UART
    {
        void setLoopbackState(uint8_t channel, bool state)
        {
            if (channel >= MAX_UART_INTERFACES)
                return;

            _loopbackEnabled[channel] = state;
        }

        bool deInit(uint8_t channel)
        {
            if (channel >= MAX_UART_INTERFACES)
                return false;

            if (Board::detail::UART::ll::deInit(channel))
            {
                setLoopbackState(channel, false);

                _rxBuffer[channel].reset();
                _txBuffer[channel].reset();

                _txDone[channel]      = true;
                _initialized[channel] = false;

                return true;
            }

            return false;
        }

        initStatus_t init(uint8_t channel, config_t& config, bool force)
        {
            if (channel >= MAX_UART_INTERFACES)
                return initStatus_t::error;

            if (isInitialized(channel) && !force)
                return initStatus_t::alreadyInit;    // interface already initialized

            if (deInit(channel))
            {
                if (Board::detail::UART::ll::init(channel, config))
                {
                    _initialized[channel] = true;
                    return initStatus_t::ok;
                }
            }

            return initStatus_t::error;
        }

        bool isInitialized(uint8_t channel)
        {
            return _initialized[channel];
        }

        bool read(uint8_t channel, uint8_t* buffer, size_t& size, const size_t maxSize)
        {
            size = 0;

            if (channel >= MAX_UART_INTERFACES)
                return false;

            while (_rxBuffer[channel].remove(buffer[size++]))
            {
                if (size >= maxSize)
                    break;
            }

            return size > 0;
        }

        bool read(uint8_t channel, uint8_t& value)
        {
            value = 0;

            if (channel >= MAX_UART_INTERFACES)
                return false;

            return _rxBuffer[channel].remove(value);
        }

        bool write(uint8_t channel, uint8_t* buffer, size_t size)
        {
            if (channel >= MAX_UART_INTERFACES)
                return false;

            for (size_t i = 0; i < size; i++)
            {
                while (!_txBuffer[channel].insert(buffer[i]))
                    ;

                uartTransmitStart(channel);
            }

            return true;
        }

        bool write(uint8_t channel, uint8_t value)
        {
            if (channel >= MAX_UART_INTERFACES)
                return false;

            while (!_txBuffer[channel].insert(value))
                ;

            uartTransmitStart(channel);

            return true;
        }

        bool isTxEmpty(uint8_t channel)
        {
            if (channel >= MAX_UART_INTERFACES)
                return false;

            return _txDone[channel];
        }

#ifdef DMX_SUPPORTED
        void setDMXChannelValue(uint16_t channel, uint8_t value)
        {
            ATOMIC_SECTION
            {
                if (channel < 513)
                    _dmxBuffer[channel] = value;
            }
        }
#endif
    }    // namespace UART

    namespace detail
    {
        namespace UART
        {
            void storeIncomingData(uint8_t channel, uint8_t data)
            {
                if (!_loopbackEnabled[channel])
                {
                    _rxBuffer[channel].insert(data);
                }
                else
                {
                    if (_txBuffer[channel].insert(data))
                    {
                        Board::detail::UART::ll::enableDataEmptyInt(channel);

                        // indicate loopback here since it's run inside interrupt, ie. not visible to the user application
                        Board::io::indicateTraffic(Board::io::dataSource_t::uart, Board::io::dataDirection_t::outgoing);
                        Board::io::indicateTraffic(Board::io::dataSource_t::uart, Board::io::dataDirection_t::incoming);
                    }
                }
            }

            bool getNextByteToSend(uint8_t channel, uint8_t& data, size_t& remainingBytes)
            {
                if (_txBuffer[channel].remove(data))
                {
                    remainingBytes = _txBuffer[channel].count();
                    return true;
                }
                else
                {
                    remainingBytes = 0;
                    return false;
                }
            }

            bool bytesToSendAvailable(uint8_t channel)
            {
                return _txBuffer[channel].count();
            }

            void indicateTxComplete(uint8_t channel)
            {
                _txDone[channel] = true;
            }

#ifdef DMX_SUPPORTED
            uint8_t* dmxBuffer()
            {
                return _dmxBuffer;
            }
#endif
        }    // namespace UART
    }        // namespace detail

#ifndef USB_SUPPORTED
    namespace
    {
        uint8_t                             _readBuffer[USB_OVER_SERIAL_BUFFER_SIZE];
        Board::USBOverSerial::USBReadPacket _readPacket(_readBuffer, USB_OVER_SERIAL_BUFFER_SIZE);
        Board::uniqueID_t                   _uidUSBDevice;
    }    // namespace

    namespace USB
    {
        // simulated USB interface via UART - make this transparent to the application

        bool isUSBconnected()
        {
            return _usbConnectionState;
        }

        bool writeMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            uint8_t dataArray[4] = {
                USBMIDIpacket.Event,
                USBMIDIpacket.Data1,
                USBMIDIpacket.Data2,
                USBMIDIpacket.Data3
            };

            USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::midi,
                                                 dataArray,
                                                 4,
                                                 USB_OVER_SERIAL_BUFFER_SIZE);

            return USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
        }

        bool readMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            bool returnValue = false;

            if (USBOverSerial::read(UART_CHANNEL_USB_LINK, _readPacket))
            {
                if (_readPacket.type() == USBOverSerial::packetType_t::midi)
                {
                    USBMIDIpacket.Event = _readPacket[0];
                    USBMIDIpacket.Data1 = _readPacket[1];
                    USBMIDIpacket.Data2 = _readPacket[2];
                    USBMIDIpacket.Data3 = _readPacket[3];

                    _readPacket.reset();
                    returnValue = true;
                }
                else
                {
                    USBLink::internalCMD_t cmd;
                    Board::detail::USB::checkInternal(cmd);
                }
            }

            return returnValue;
        }

        bool writeCDC(uint8_t* buffer, size_t size)
        {
            USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::cdc,
                                                 buffer,
                                                 size,
                                                 USB_OVER_SERIAL_BUFFER_SIZE);

            return USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
        }

        bool writeCDC(uint8_t value)
        {
            USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::cdc,
                                                 &value,
                                                 1,
                                                 USB_OVER_SERIAL_BUFFER_SIZE);

            return USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
        }

        bool readCDC(uint8_t* buffer, size_t& size, const size_t maxSize)
        {
            if (USBOverSerial::read(UART_CHANNEL_USB_LINK, _readPacket))
            {
                if (_readPacket.type() == USBOverSerial::packetType_t::cdc)
                {
                    size = _readPacket.size() > maxSize ? maxSize : _readPacket.size();

                    for (size_t i = 0; i < size; i++)
                        buffer[i] = _readPacket[i];

                    _readPacket.reset();
                    return true;
                }
                else
                {
                    USBLink::internalCMD_t cmd;
                    Board::detail::USB::checkInternal(cmd);
                }
            }

            return false;
        }

        bool readCDC(uint8_t& value)
        {
            if (USBOverSerial::read(UART_CHANNEL_USB_LINK, _readPacket))
            {
                if (_readPacket.type() == USBOverSerial::packetType_t::cdc)
                {
                    value = _readPacket[0];

                    _readPacket.reset();
                    return true;
                }
                else
                {
                    USBLink::internalCMD_t cmd;
                    Board::detail::USB::checkInternal(cmd);
                }
            }

            return false;
        }
    }    // namespace USB

    namespace detail
    {
        namespace USB
        {
            bool checkInternal(USBLink::internalCMD_t& cmd)
            {
                bool validCmd = true;

                if (_readPacket.type() == USBOverSerial::packetType_t::internal)
                {
                    switch (_readPacket[0])
                    {
                    case static_cast<uint8_t>(USBLink::internalCMD_t::usbState):
                    {
                        _usbConnectionState = _readPacket[1];
                    }
                    break;

                    case static_cast<uint8_t>(USBLink::internalCMD_t::baudRateChange):
                    {
                        uint32_t baudRate = 0;

                        baudRate = _readPacket[4];
                        baudRate <<= 8;
                        baudRate |= _readPacket[3];
                        baudRate <<= 8;
                        baudRate |= _readPacket[2];
                        baudRate <<= 8;
                        baudRate |= _readPacket[1];

                        Board::USB::onCDCsetLineEncoding(baudRate);
                    }
                    break;

                    case static_cast<uint8_t>(USBLink::internalCMD_t::uniqueID):
                    {
                        for (size_t i = 0; i < UID_BITS / 8; i++)
                            _uidUSBDevice[i] = _readPacket[i + 1];
                    }
                    break;

                    default:
                    {
                        validCmd = false;
                    }
                    break;
                    }

                    if (validCmd)
                        cmd = static_cast<USBLink::internalCMD_t>(_readPacket[0]);

                    _readPacket.reset();
                }

                return validCmd;
            }

            bool readInternal(USBLink::internalCMD_t& cmd)
            {
                if (USBOverSerial::read(UART_CHANNEL_USB_LINK, _readPacket))
                {
                    if (_readPacket.type() == USBOverSerial::packetType_t::internal)
                    {
                        return checkInternal(cmd);
                    }
                }

                return false;
            }
        }    // namespace USB
    }        // namespace detail

    void uniqueID(uniqueID_t& uid)
    {
        for (size_t i = 0; i < UID_BITS / 8; i++)
            uid[i] = _uidUSBDevice[i];
    }
#endif
}    // namespace Board

#endif