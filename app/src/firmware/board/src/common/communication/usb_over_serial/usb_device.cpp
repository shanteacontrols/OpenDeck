/*

Copyright Igor Petrovic

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

#include "board/board.h"
#include "internal.h"

#include "core/mcu.h"

namespace
{
    /// Holds the USB state received from USB link MCU
    constexpr size_t                      READ_BUFFER_SIZE = 16;
    bool                                  usbConnectionState;
    bool                                  uniqueIdReceived;
    uint8_t                               readBuffer[READ_BUFFER_SIZE];
    board::usb_over_serial::UsbReadPacket readPacket(readBuffer, READ_BUFFER_SIZE);
    core::mcu::uniqueID_t                 uidUsbDevice;
}    // namespace

namespace board
{
    void uniqueID(core::mcu::uniqueID_t& uid)
    {
        if (!uniqueIdReceived)
        {
            uint8_t data[1] = {
                static_cast<uint8_t>(usb_over_serial::internalCmd_t::UNIQUE_ID),
            };

            usb_over_serial::UsbWritePacket packet(usb_over_serial::packetType_t::INTERNAL,
                                                   data,
                                                   1,
                                                   1);

            usb_over_serial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);

            usb_over_serial::internalCmd_t cmd;

            while (!detail::usb::readInternal(cmd))
            {
                ;
            }

            if (cmd == usb_over_serial::internalCmd_t::UNIQUE_ID)
            {
                uniqueIdReceived = true;
            }
        }

        for (size_t i = 0; i < CORE_MCU_UID_BITS / 8; i++)
        {
            uid[i] = uidUsbDevice[i];
        }
    }

#ifndef PROJECT_TARGET_SUPPORT_USB_INDICATORS
    namespace io::indicators
    {
        void indicateFactoryReset()
        {
            uint8_t data[1] = {
                static_cast<uint8_t>(usb_over_serial::internalCmd_t::FACTORY_RESET),
            };

            usb_over_serial::UsbWritePacket packet(usb_over_serial::packetType_t::INTERNAL,
                                                   data,
                                                   1,
                                                   1);

            usb_over_serial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
        }
    }    // namespace io::indicators
#endif

    namespace usb
    {
        bool isUsbConnected()
        {
            return usbConnectionState;
        }

        bool writeMidi(lib::midi::usb::Packet& packet)
        {
            usb_over_serial::UsbWritePacket writePacket(usb_over_serial::packetType_t::MIDI,
                                                        &packet.data[0],
                                                        sizeof(packet.data),
                                                        sizeof(packet.data));

            return usb_over_serial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, writePacket);
        }

        bool readMidi(lib::midi::usb::Packet& packet)
        {
            bool retVal = false;

            if (usb_over_serial::read(PROJECT_TARGET_UART_CHANNEL_USB_LINK, readPacket))
            {
                if (readPacket.type() == usb_over_serial::packetType_t::MIDI)
                {
                    for (size_t i = 0; i < sizeof(packet.data); i++)
                    {
                        packet.data[i] = readPacket[i];
                    }

                    readPacket.reset();
                    retVal = true;
                }
                else
                {
                    usb_over_serial::internalCmd_t cmd;
                    board::detail::usb::checkInternal(cmd);
                }
            }

            return retVal;
        }
    }    // namespace usb

    namespace detail::usb
    {
        void init()
        {
            uint8_t data[1] = {
                static_cast<uint8_t>(usb_over_serial::internalCmd_t::CONNECT_USB),
            };

            usb_over_serial::UsbWritePacket packet(usb_over_serial::packetType_t::INTERNAL,
                                                   data,
                                                   1,
                                                   1);

            usb_over_serial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
        }

        void deInit()
        {
            uint8_t data[1] = {
                static_cast<uint8_t>(usb_over_serial::internalCmd_t::DISCONNECT_USB),
            };

            usb_over_serial::UsbWritePacket packet(usb_over_serial::packetType_t::INTERNAL,
                                                   data,
                                                   1,
                                                   1);

            usb_over_serial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
        }

        bool checkInternal(usb_over_serial::internalCmd_t& cmd)
        {
            bool validCmd = true;

            if (readPacket.type() == usb_over_serial::packetType_t::INTERNAL)
            {
                switch (readPacket[0])
                {
                case static_cast<uint8_t>(usb_over_serial::internalCmd_t::USB_STATE):
                {
                    usbConnectionState = readPacket[1];
                }
                break;

                case static_cast<uint8_t>(usb_over_serial::internalCmd_t::UNIQUE_ID):
                {
                    for (size_t i = 0; i < CORE_MCU_UID_BITS / 8; i++)
                    {
                        uidUsbDevice[i] = readPacket[i + 1];
                    }
                }
                break;

                case static_cast<uint8_t>(usb_over_serial::internalCmd_t::DISCONNECT_USB):
                {
                    board::usb::deInit();
                }
                break;

                case static_cast<uint8_t>(usb_over_serial::internalCmd_t::LINK_READY):
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
                    cmd = static_cast<usb_over_serial::internalCmd_t>(readPacket[0]);
                }

                readPacket.reset();
            }

            return validCmd;
        }

        bool readInternal(usb_over_serial::internalCmd_t& cmd)
        {
            if (usb_over_serial::read(PROJECT_TARGET_UART_CHANNEL_USB_LINK, readPacket))
            {
                if (readPacket.type() == usb_over_serial::packetType_t::INTERNAL)
                {
                    return checkInternal(cmd);
                }
            }

            return false;
        }
    }    // namespace detail::usb
}    // namespace board

#endif