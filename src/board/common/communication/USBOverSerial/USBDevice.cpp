/*

Copyright 2015-2022 Igor Petrovic

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

#ifdef HW_USB_OVER_SERIAL_DEVICE

// simulated USB interface via UART - make this transparent to the application

#include "board/Internal.h"
#include "usb-link/Commands.h"
#include "USBOverSerial.h"
#include "core/src/MCU.h"

namespace
{
    /// Holds the USB state received from USB link MCU
    bool                                _usbConnectionState = false;
    uint8_t                             _readBuffer[BUFFER_SIZE_USB_OVER_SERIAL];
    Board::USBOverSerial::USBReadPacket _readPacket(_readBuffer, BUFFER_SIZE_USB_OVER_SERIAL);
    core::mcu::uniqueID_t               _uidUSBDevice;
}    // namespace

namespace Board
{
    void uniqueID(core::mcu::uniqueID_t& uid)
    {
        for (size_t i = 0; i < CORE_MCU_UID_BITS / 8; i++)
        {
            uid[i] = _uidUSBDevice[i];
        }
    }

    namespace USB
    {
        bool isUSBconnected()
        {
            return _usbConnectionState;
        }

        bool writeMIDI(midiPacket_t& packet)
        {
            USBOverSerial::USBWritePacket writePacket(USBOverSerial::packetType_t::MIDI,
                                                      &packet[0],
                                                      sizeof(packet),
                                                      BUFFER_SIZE_USB_OVER_SERIAL);

            return USBOverSerial::write(HW_UART_CHANNEL_USB_LINK, writePacket);
        }

        bool readMIDI(midiPacket_t& packet)
        {
            bool retVal = false;

            if (USBOverSerial::read(HW_UART_CHANNEL_USB_LINK, _readPacket))
            {
                if (_readPacket.type() == USBOverSerial::packetType_t::MIDI)
                {
                    for (size_t i = 0; i < sizeof(packet); i++)
                    {
                        packet[i] = _readPacket[i];
                    }

                    _readPacket.reset();
                    retVal = true;
                }
                else
                {
                    USBLink::internalCMD_t cmd;
                    Board::detail::USB::checkInternal(cmd);
                }
            }

            return retVal;
        }

        bool writeCDC(uint8_t* buffer, size_t size)
        {
            USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::CDC,
                                                 buffer,
                                                 size,
                                                 BUFFER_SIZE_USB_OVER_SERIAL);

            return USBOverSerial::write(HW_UART_CHANNEL_USB_LINK, packet);
        }

        bool writeCDC(uint8_t value)
        {
            USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::CDC,
                                                 &value,
                                                 1,
                                                 BUFFER_SIZE_USB_OVER_SERIAL);

            return USBOverSerial::write(HW_UART_CHANNEL_USB_LINK, packet);
        }

        bool readCDC(uint8_t* buffer, size_t& size, const size_t maxSize)
        {
            if (USBOverSerial::read(HW_UART_CHANNEL_USB_LINK, _readPacket))
            {
                if (_readPacket.type() == USBOverSerial::packetType_t::CDC)
                {
                    size = _readPacket.size() > maxSize ? maxSize : _readPacket.size();

                    for (size_t i = 0; i < size; i++)
                    {
                        buffer[i] = _readPacket[i];
                    }

                    _readPacket.reset();
                    return true;
                }

                USBLink::internalCMD_t cmd;
                Board::detail::USB::checkInternal(cmd);
            }

            return false;
        }

        bool readCDC(uint8_t& value)
        {
            if (USBOverSerial::read(HW_UART_CHANNEL_USB_LINK, _readPacket))
            {
                if (_readPacket.type() == USBOverSerial::packetType_t::CDC)
                {
                    value = _readPacket[0];

                    _readPacket.reset();
                    return true;
                }

                USBLink::internalCMD_t cmd;
                Board::detail::USB::checkInternal(cmd);
            }

            return false;
        }
    }    // namespace USB

    namespace detail::USB
    {
        void init()
        {
        }

        void deInit()
        {
            uint8_t data[1] = {
                static_cast<uint8_t>(USBLink::internalCMD_t::DISCONNECT_USB),
            };

            USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::INTERNAL,
                                                 data,
                                                 1,
                                                 BUFFER_SIZE_USB_OVER_SERIAL);

            USBOverSerial::write(HW_UART_CHANNEL_USB_LINK, packet);
        }

        bool checkInternal(USBLink::internalCMD_t& cmd)
        {
            bool validCmd = true;

            if (_readPacket.type() == USBOverSerial::packetType_t::INTERNAL)
            {
                switch (_readPacket[0])
                {
                case static_cast<uint8_t>(USBLink::internalCMD_t::USB_STATE):
                {
                    _usbConnectionState = _readPacket[1];
                }
                break;

                case static_cast<uint8_t>(USBLink::internalCMD_t::BAUDRATE_CHANGE):
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

                case static_cast<uint8_t>(USBLink::internalCMD_t::UNIQUE_ID):
                {
                    for (size_t i = 0; i < CORE_MCU_UID_BITS / 8; i++)
                    {
                        _uidUSBDevice[i] = _readPacket[i + 1];
                    }
                }
                break;

                case static_cast<uint8_t>(USBLink::internalCMD_t::DISCONNECT_USB):
                {
                    Board::USB::deInit();
                }
                break;

                default:
                {
                    validCmd = false;
                }
                break;
                }

                if (validCmd)
                {
                    cmd = static_cast<USBLink::internalCMD_t>(_readPacket[0]);
                }

                _readPacket.reset();
            }

            return validCmd;
        }

        bool readInternal(USBLink::internalCMD_t& cmd)
        {
            if (USBOverSerial::read(HW_UART_CHANNEL_USB_LINK, _readPacket))
            {
                if (_readPacket.type() == USBOverSerial::packetType_t::INTERNAL)
                {
                    return checkInternal(cmd);
                }
            }

            return false;
        }
    }    // namespace detail::USB
}    // namespace Board

#endif