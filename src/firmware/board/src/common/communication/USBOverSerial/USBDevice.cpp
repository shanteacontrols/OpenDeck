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

#ifdef PROJECT_TARGET_USB_OVER_SERIAL_DEVICE

// simulated USB interface via UART - make this transparent to the application

#include "board/Board.h"
#include "Internal.h"
#include "core/MCU.h"

namespace
{
    /// Holds the USB state received from USB link MCU
    constexpr size_t                    READ_BUFFER_SIZE    = 16;
    bool                                _usbConnectionState = false;
    bool                                _uniqueIDReceived;
    uint8_t                             _readBuffer[READ_BUFFER_SIZE];
    board::usbOverSerial::USBReadPacket _readPacket(_readBuffer, READ_BUFFER_SIZE);
    core::mcu::uniqueID_t               _uidUSBDevice;
}    // namespace

namespace board
{
    void uniqueID(core::mcu::uniqueID_t& uid)
    {
        if (!_uniqueIDReceived)
        {
            uint8_t data[1] = {
                static_cast<uint8_t>(usbOverSerial::internalCMD_t::UNIQUE_ID),
            };

            usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::INTERNAL,
                                                 data,
                                                 1,
                                                 1);

            usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);

            usbOverSerial::internalCMD_t cmd;

            while (!detail::usb::readInternal(cmd))
            {
                ;
            }

            if (cmd == usbOverSerial::internalCMD_t::UNIQUE_ID)
            {
                _uniqueIDReceived = true;
            }
        }

        for (size_t i = 0; i < CORE_MCU_UID_BITS / 8; i++)
        {
            uid[i] = _uidUSBDevice[i];
        }
    }

#ifndef PROJECT_TARGET_SUPPORT_USB_INDICATORS
    namespace io::indicators
    {
        void indicateFactoryReset()
        {
            uint8_t data[1] = {
                static_cast<uint8_t>(usbOverSerial::internalCMD_t::FACTORY_RESET),
            };

            usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::INTERNAL,
                                                 data,
                                                 1,
                                                 1);

            usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
        }
    }    // namespace io::indicators
#endif

    namespace usb
    {
        bool isUSBconnected()
        {
            return _usbConnectionState;
        }

        bool writeMIDI(midiPacket_t& packet)
        {
            usbOverSerial::USBWritePacket writePacket(usbOverSerial::packetType_t::MIDI,
                                                      &packet[0],
                                                      sizeof(packet),
                                                      sizeof(packet));

            return usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, writePacket);
        }

        bool readMIDI(midiPacket_t& packet)
        {
            bool retVal = false;

            if (usbOverSerial::read(PROJECT_TARGET_UART_CHANNEL_USB_LINK, _readPacket))
            {
                if (_readPacket.type() == usbOverSerial::packetType_t::MIDI)
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
                    usbOverSerial::internalCMD_t cmd;
                    board::detail::usb::checkInternal(cmd);
                }
            }

            return retVal;
        }

        bool writeCDC(uint8_t* buffer, size_t size)
        {
            usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::CDC,
                                                 buffer,
                                                 size,
                                                 size);

            return usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
        }

        bool writeCDC(uint8_t value)
        {
            usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::CDC,
                                                 &value,
                                                 1,
                                                 1);

            return usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
        }

        bool readCDC(uint8_t* buffer, size_t& size, const size_t maxSize)
        {
            if (usbOverSerial::read(PROJECT_TARGET_UART_CHANNEL_USB_LINK, _readPacket))
            {
                if (_readPacket.type() == usbOverSerial::packetType_t::CDC)
                {
                    size = _readPacket.size() > maxSize ? maxSize : _readPacket.size();

                    for (size_t i = 0; i < size; i++)
                    {
                        buffer[i] = _readPacket[i];
                    }

                    _readPacket.reset();
                    return true;
                }

                usbOverSerial::internalCMD_t cmd;
                board::detail::usb::checkInternal(cmd);
            }

            return false;
        }

        bool readCDC(uint8_t& value)
        {
            if (usbOverSerial::read(PROJECT_TARGET_UART_CHANNEL_USB_LINK, _readPacket))
            {
                if (_readPacket.type() == usbOverSerial::packetType_t::CDC)
                {
                    value = _readPacket[0];

                    _readPacket.reset();
                    return true;
                }

                usbOverSerial::internalCMD_t cmd;
                board::detail::usb::checkInternal(cmd);
            }

            return false;
        }
    }    // namespace usb

    namespace detail::usb
    {
        void init()
        {
            uint8_t data[1] = {
                static_cast<uint8_t>(usbOverSerial::internalCMD_t::CONNECT_USB),
            };

            usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::INTERNAL,
                                                 data,
                                                 1,
                                                 1);

            usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
        }

        void deInit()
        {
            uint8_t data[1] = {
                static_cast<uint8_t>(usbOverSerial::internalCMD_t::DISCONNECT_USB),
            };

            usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::INTERNAL,
                                                 data,
                                                 1,
                                                 1);

            usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
        }

        bool checkInternal(usbOverSerial::internalCMD_t& cmd)
        {
            bool validCmd = true;

            if (_readPacket.type() == usbOverSerial::packetType_t::INTERNAL)
            {
                switch (_readPacket[0])
                {
                case static_cast<uint8_t>(usbOverSerial::internalCMD_t::USB_STATE):
                {
                    _usbConnectionState = _readPacket[1];
                }
                break;

                case static_cast<uint8_t>(usbOverSerial::internalCMD_t::BAUDRATE_CHANGE):
                {
                    uint32_t baudRate = 0;

                    baudRate = _readPacket[4];
                    baudRate <<= 8;
                    baudRate |= _readPacket[3];
                    baudRate <<= 8;
                    baudRate |= _readPacket[2];
                    baudRate <<= 8;
                    baudRate |= _readPacket[1];

                    board::usb::onCDCsetLineEncoding(baudRate);
                }
                break;

                case static_cast<uint8_t>(usbOverSerial::internalCMD_t::UNIQUE_ID):
                {
                    for (size_t i = 0; i < CORE_MCU_UID_BITS / 8; i++)
                    {
                        _uidUSBDevice[i] = _readPacket[i + 1];
                    }
                }
                break;

                case static_cast<uint8_t>(usbOverSerial::internalCMD_t::DISCONNECT_USB):
                {
                    board::usb::deInit();
                }
                break;

                case static_cast<uint8_t>(usbOverSerial::internalCMD_t::LINK_READY):
                {
                    // nothing to do
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
                    cmd = static_cast<usbOverSerial::internalCMD_t>(_readPacket[0]);
                }

                _readPacket.reset();
            }

            return validCmd;
        }

        bool readInternal(usbOverSerial::internalCMD_t& cmd)
        {
            if (usbOverSerial::read(PROJECT_TARGET_UART_CHANNEL_USB_LINK, _readPacket))
            {
                if (_readPacket.type() == usbOverSerial::packetType_t::INTERNAL)
                {
                    return checkInternal(cmd);
                }
            }

            return false;
        }
    }    // namespace detail::usb
}    // namespace board

#endif