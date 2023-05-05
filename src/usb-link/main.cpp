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

#include "board/Board.h"
#include "board/src/common/communication/USBOverSerial/USBOverSerial.h"
#include "core/Timing.h"
#include "core/MCU.h"
#include "protocol/midi/MIDI.h"

using namespace board;
using namespace protocol;

namespace
{
    /// Time in milliseconds after which USB connection state should be checked
    constexpr uint32_t USB_CONN_CHECK_TIME = 2000;
    constexpr size_t   READ_BUFFER_SIZE    = 16;

    uint8_t                      uartReadBuffer[READ_BUFFER_SIZE];
    usbOverSerial::USBReadPacket readPacket(uartReadBuffer, READ_BUFFER_SIZE);
    MIDI::usbMIDIPacket_t        usbMIDIPacket;

    void checkUSBconnection()
    {
        static uint32_t lastCheckTime       = 0;
        static bool     lastConnectionState = false;

        if (core::timing::ms() - lastCheckTime > USB_CONN_CHECK_TIME)
        {
            using namespace board;
            bool newState = usb::isUSBconnected();

            if (lastConnectionState != newState)
            {
                uint8_t data[2] = {
                    static_cast<uint8_t>(usbOverSerial::internalCMD_t::USB_STATE),
                    newState,
                };

                usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::INTERNAL,
                                                     data,
                                                     2,
                                                     2);
                usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);

                lastConnectionState = newState;
            }

            lastCheckTime = core::timing::ms();
        }
    }

    void sendUniqueID()
    {
        using namespace board;
        core::mcu::uniqueID_t uniqueID;
        core::mcu::uniqueID(uniqueID);

        uint8_t data[11] = {
            static_cast<uint8_t>(usbOverSerial::internalCMD_t::UNIQUE_ID),
            uniqueID[0],
            uniqueID[1],
            uniqueID[2],
            uniqueID[3],
            uniqueID[4],
            uniqueID[5],
            uniqueID[6],
            uniqueID[7],
            uniqueID[8],
            uniqueID[9],
        };

        usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::INTERNAL,
                                             data,
                                             11,
                                             11);
        usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
    }

    void sendLinkReady()
    {
        using namespace board;
        uint8_t data[1] = {
            static_cast<uint8_t>(usbOverSerial::internalCMD_t::LINK_READY),
        };

        usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::INTERNAL,
                                             data,
                                             1,
                                             1);
        usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
    }
}    // namespace

int main()
{
    board::init();

    while (1)
    {
        // USB MIDI -> UART
        if (usb::readMIDI(usbMIDIPacket))
        {
            usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::MIDI,
                                                 &usbMIDIPacket[0],
                                                 4,
                                                 4);

            if (usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet))
            {
                board::io::indicators::indicateTraffic(board::io::indicators::source_t::USB,
                                                       board::io::indicators::direction_t::INCOMING);
            }
        }

        // UART -> USB
        if (usbOverSerial::read(PROJECT_TARGET_UART_CHANNEL_USB_LINK, readPacket))
        {
            if (readPacket.type() == usbOverSerial::packetType_t::MIDI)
            {
                for (size_t i = 0; i < sizeof(usbMIDIPacket); i++)
                {
                    usbMIDIPacket[i] = readPacket[i];
                }

                if (usb::writeMIDI(usbMIDIPacket))
                {
                    board::io::indicators::indicateTraffic(board::io::indicators::source_t::USB,
                                                           board::io::indicators::direction_t::OUTGOING);
                }
            }
            else if (readPacket.type() == usbOverSerial::packetType_t::INTERNAL)
            {
                using namespace board;
                auto cmd = static_cast<usbOverSerial::internalCMD_t>(readPacket[0]);

                switch (cmd)
                {
                case usbOverSerial::internalCMD_t::REBOOT_BTLDR:
                {
                    // use received data as the magic bootloader value
                    uint32_t magicVal = readPacket[1];
                    magicVal <<= 8;
                    magicVal |= readPacket[2];
                    magicVal <<= 8;
                    magicVal |= readPacket[3];
                    magicVal <<= 8;
                    magicVal |= readPacket[4];

                    bootloader::setMagicBootValue(magicVal);
                    reboot();
                }
                break;

                case usbOverSerial::internalCMD_t::DISCONNECT_USB:
                {
                    board::usb::deInit();
                }
                break;

                case usbOverSerial::internalCMD_t::CONNECT_USB:
                {
                    if (!usb::isUSBconnected())
                    {
                        board::usb::init();
                    }
                }
                break;

                case usbOverSerial::internalCMD_t::UNIQUE_ID:
                {
                    sendUniqueID();
                }
                break;

                case usbOverSerial::internalCMD_t::LINK_READY:
                {
                    sendLinkReady();
                }
                break;

                case usbOverSerial::internalCMD_t::FACTORY_RESET:
                {
                    board::io::indicators::indicateFactoryReset();
                }
                break;

                default:
                    break;
                }
            }

            // clear out any stored information in packet since we are reusing it
            readPacket.reset();
        }

        checkUSBconnection();
    }
}