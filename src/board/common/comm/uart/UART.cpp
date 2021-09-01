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
#include "MCU.h"

#ifndef USB_SUPPORTED
#include "board/common/comm/USBOverSerial/USBOverSerial.h"
#include "usb-link/Commands.h"
#endif

//generic UART driver, arch-independent

namespace
{
    /// Flag determining whether or not UART loopback functionality is enabled.
    /// When enabled, all incoming UART traffic is immediately passed on to UART TX.
    volatile bool loopbackEnabled[MAX_UART_INTERFACES];

    /// Flag signaling that the transmission is done.
    volatile bool txDone[MAX_UART_INTERFACES];

    /// Flag holding the state of UART interface (whether it's initialized or not).
    bool initialized[MAX_UART_INTERFACES];

    /// Buffer in which outgoing UART data is stored.
    core::RingBuffer<uint8_t, UART_TX_BUFFER_SIZE> txBuffer[MAX_UART_INTERFACES];

    /// Buffer in which incoming UART data is stored.
    core::RingBuffer<uint8_t, UART_RX_BUFFER_SIZE> rxBuffer[MAX_UART_INTERFACES];

#ifndef USB_SUPPORTED
    /// Holds the USB state received from USB link MCU
    bool usbConnectionState = false;
#endif

    /// Starts the process of transmitting the data from UART TX buffer to UART interface.
    /// param [in]: channel     UART channel on MCU.
    void uartTransmitStart(uint8_t channel)
    {
        if (channel >= MAX_UART_INTERFACES)
            return;

        txDone[channel] = false;

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

            loopbackEnabled[channel] = state;
        }

        bool deInit(uint8_t channel)
        {
            if (channel >= MAX_UART_INTERFACES)
                return false;

            if (Board::detail::UART::ll::deInit(channel))
            {
                setLoopbackState(channel, false);

                rxBuffer[channel].reset();
                txBuffer[channel].reset();

                txDone[channel]      = true;
                initialized[channel] = false;

                return true;
            }

            return false;
        }

        initStatus_t init(uint8_t channel, config_t& config, bool force)
        {
            if (channel >= MAX_UART_INTERFACES)
                return initStatus_t::error;

            if (isInitialized(channel) && !force)
                return initStatus_t::alreadyInit;    //interface already initialized

            if (deInit(channel))
            {
                if (Board::detail::UART::ll::init(channel, config))
                {
                    initialized[channel] = true;
                    return initStatus_t::ok;
                }
            }

            return initStatus_t::error;
        }

        bool isInitialized(uint8_t channel)
        {
            return initialized[channel];
        }

        bool read(uint8_t channel, uint8_t* buffer, size_t& size, const size_t maxSize)
        {
            size = 0;

            if (channel >= MAX_UART_INTERFACES)
                return false;

            while (rxBuffer[channel].remove(buffer[size++]))
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

            return rxBuffer[channel].remove(value);
        }

        bool write(uint8_t channel, uint8_t* buffer, size_t size)
        {
            if (channel >= MAX_UART_INTERFACES)
                return false;

            for (size_t i = 0; i < size; i++)
            {
                while (!txBuffer[channel].insert(buffer[i]))
                    ;

                uartTransmitStart(channel);
            }

            return true;
        }

        bool write(uint8_t channel, uint8_t value)
        {
            if (channel >= MAX_UART_INTERFACES)
                return false;

            while (!txBuffer[channel].insert(value))
                ;

            uartTransmitStart(channel);

            return true;
        }

        bool isTxEmpty(uint8_t channel)
        {
            if (channel >= MAX_UART_INTERFACES)
                return false;

            return txDone[channel];
        }
    }    // namespace UART

    namespace detail
    {
        namespace UART
        {
            void storeIncomingData(uint8_t channel, uint8_t data)
            {
                if (!loopbackEnabled[channel])
                {
                    rxBuffer[channel].insert(data);
                }
                else
                {
                    if (txBuffer[channel].insert(data))
                    {
                        Board::detail::UART::ll::enableDataEmptyInt(channel);

                        //indicate loopback here since it's run inside interrupt, ie. not visible to the user application
                        Board::io::indicateTraffic(Board::io::dataSource_t::uart, Board::io::dataDirection_t::outgoing);
                        Board::io::indicateTraffic(Board::io::dataSource_t::uart, Board::io::dataDirection_t::incoming);
                    }
                }
            }

            bool getNextByteToSend(uint8_t channel, uint8_t& data, size_t& remainingBytes)
            {
                if (txBuffer[channel].remove(data))
                {
                    remainingBytes = txBuffer[channel].count();
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
                return txBuffer[channel].count();
            }

            void indicateTxComplete(uint8_t channel)
            {
                txDone[channel] = true;
            }
        }    // namespace UART
    }        // namespace detail

#ifndef USB_SUPPORTED
    namespace
    {
        uint8_t                             readBuffer[USB_OVER_SERIAL_BUFFER_SIZE];
        Board::USBOverSerial::USBReadPacket readPacket(readBuffer, USB_OVER_SERIAL_BUFFER_SIZE);
        Board::uniqueID_t                   uidUSBDevice;
    }    // namespace

    namespace USB
    {
        //simulated USB interface via UART - make this transparent to the application

        bool isUSBconnected()
        {
            return usbConnectionState;
        }

        void checkInternal()
        {
            if (readPacket.type() == USBOverSerial::packetType_t::internal)
            {
                //internal command
                if (readPacket[0] == static_cast<uint8_t>(USBLink::internalCMD_t::usbState))
                {
                    usbConnectionState = readPacket[1];

                    //this command also includes unique ID for USB link master
                    //use it for non-usb MCU as well
                    for (size_t i = 0; i < UID_BITS / 8; i++)
                        uidUSBDevice[i] = readPacket[i + 2];
                }
                else if (readPacket[0] == static_cast<uint8_t>(USBLink::internalCMD_t::baudRateChange))
                {
                    uint32_t baudRate = 0;

                    baudRate = readPacket[4];
                    baudRate <<= 8;
                    baudRate |= readPacket[3];
                    baudRate <<= 8;
                    baudRate |= readPacket[2];
                    baudRate <<= 8;
                    baudRate |= readPacket[1];

                    Board::USB::onCDCsetLineEncoding(baudRate);
                }

                readPacket.reset();
            }
        }

        bool writeMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            uint8_t dataArray[4] = {
                USBMIDIpacket.Event,
                USBMIDIpacket.Data1,
                USBMIDIpacket.Data2,
                USBMIDIpacket.Data3
            };

            USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::midi, dataArray, 4);

            return USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
        }

        bool readMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            bool returnValue = false;

            if (USBOverSerial::read(UART_CHANNEL_USB_LINK, readPacket))
            {
                if (readPacket.type() == USBOverSerial::packetType_t::midi)
                {
                    USBMIDIpacket.Event = readPacket[0];
                    USBMIDIpacket.Data1 = readPacket[1];
                    USBMIDIpacket.Data2 = readPacket[2];
                    USBMIDIpacket.Data3 = readPacket[3];

                    readPacket.reset();
                    returnValue = true;
                }
                else
                {
                    checkInternal();
                }
            }

            return returnValue;
        }

        bool writeCDC(uint8_t* buffer, size_t size)
        {
            USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::cdc, buffer, size);

            return USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
        }

        bool writeCDC(uint8_t value)
        {
            USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::cdc, &value, 1);

            return USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
        }

        bool readCDC(uint8_t* buffer, size_t& size, const size_t maxSize)
        {
            if (USBOverSerial::read(UART_CHANNEL_USB_LINK, readPacket))
            {
                if (readPacket.type() == USBOverSerial::packetType_t::cdc)
                {
                    size = readPacket.size() > maxSize ? maxSize : readPacket.size();

                    for (size_t i = 0; i < size; i++)
                        buffer[i] = readPacket[i];

                    readPacket.reset();
                    return true;
                }
                else
                {
                    checkInternal();
                }
            }

            return false;
        }

        bool readCDC(uint8_t& value)
        {
            if (USBOverSerial::read(UART_CHANNEL_USB_LINK, readPacket))
            {
                if (readPacket.type() == USBOverSerial::packetType_t::cdc)
                {
                    value = readPacket[0];

                    readPacket.reset();
                    return true;
                }
                else
                {
                    checkInternal();
                }
            }

            return false;
        }
    }    // namespace USB

    void uniqueID(uniqueID_t& uid)
    {
        for (size_t i = 0; i < UID_BITS / 8; i++)
            uid[i] = uidUSBDevice[i];
    }
#endif
}    // namespace Board

#endif