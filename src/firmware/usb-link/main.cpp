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

#include "board/board.h"
#include "application/protocol/midi/midi.h"

#include "core/mcu.h"

using namespace board;
using namespace protocol;

namespace
{
    /// Time in milliseconds after which USB connection state should be checked
    constexpr uint32_t USB_CONN_CHECK_TIME = 2000;
    constexpr size_t   READ_BUFFER_SIZE    = 16;

    uint8_t                        uartReadBuffer[READ_BUFFER_SIZE];
    usb_over_serial::UsbReadPacket readPacket(uartReadBuffer, READ_BUFFER_SIZE);
    midi::UsbPacket                usbMIDIPacket;

    void checkUSBconnection()
    {
        static uint32_t lastCheckTime       = 0;
        static bool     lastConnectionState = false;

        if (core::mcu::timing::ms() - lastCheckTime > USB_CONN_CHECK_TIME)
        {
            using namespace board;
            bool newState = usb::isUsbConnected();

            if (lastConnectionState != newState)
            {
                uint8_t data[2] = {
                    static_cast<uint8_t>(usb_over_serial::internalCmd_t::USB_STATE),
                    newState,
                };

                usb_over_serial::UsbWritePacket packet(usb_over_serial::packetType_t::INTERNAL,
                                                       data,
                                                       2,
                                                       2);
                usb_over_serial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);

                lastConnectionState = newState;
            }

            lastCheckTime = core::mcu::timing::ms();
        }
    }

    void sendUniqueId()
    {
        using namespace board;

        core::mcu::uniqueID_t uniqueId;
        core::mcu::uniqueID(uniqueId);

        uint8_t data[11] = {
            static_cast<uint8_t>(usb_over_serial::internalCmd_t::UNIQUE_ID),
            uniqueId[0],
            uniqueId[1],
            uniqueId[2],
            uniqueId[3],
            uniqueId[4],
            uniqueId[5],
            uniqueId[6],
            uniqueId[7],
            uniqueId[8],
            uniqueId[9],
        };

        usb_over_serial::UsbWritePacket packet(usb_over_serial::packetType_t::INTERNAL,
                                               data,
                                               11,
                                               11);
        usb_over_serial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
    }

    void sendLinkReady()
    {
        using namespace board;

        uint8_t data[1] = {
            static_cast<uint8_t>(usb_over_serial::internalCmd_t::LINK_READY),
        };

        usb_over_serial::UsbWritePacket packet(usb_over_serial::packetType_t::INTERNAL,
                                               data,
                                               1,
                                               1);
        usb_over_serial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);
    }
}    // namespace

int main()
{
    board::init();

    while (1)
    {
        // USB MIDI -> UART
        if (usb::readMidi(usbMIDIPacket))
        {
            usb_over_serial::UsbWritePacket packet(usb_over_serial::packetType_t::MIDI,
                                                   &usbMIDIPacket.data[0],
                                                   sizeof(usbMIDIPacket.data),
                                                   sizeof(usbMIDIPacket.data));

            if (usb_over_serial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet))
            {
                board::io::indicators::indicateTraffic(board::io::indicators::source_t::USB,
                                                       board::io::indicators::direction_t::INCOMING);
            }
        }

        // UART -> USB
        if (usb_over_serial::read(PROJECT_TARGET_UART_CHANNEL_USB_LINK, readPacket))
        {
            if (readPacket.type() == usb_over_serial::packetType_t::MIDI)
            {
                for (size_t i = 0; i < sizeof(usbMIDIPacket); i++)
                {
                    usbMIDIPacket.data[i] = readPacket[i];
                }

                if (usb::writeMidi(usbMIDIPacket))
                {
                    board::io::indicators::indicateTraffic(board::io::indicators::source_t::USB,
                                                           board::io::indicators::direction_t::OUTGOING);
                }
            }
            else if (readPacket.type() == usb_over_serial::packetType_t::INTERNAL)
            {
                using namespace board;
                auto cmd = static_cast<usb_over_serial::internalCmd_t>(readPacket[0]);

                switch (cmd)
                {
                case usb_over_serial::internalCmd_t::REBOOT_BTLDR:
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

                case usb_over_serial::internalCmd_t::DISCONNECT_USB:
                {
                    board::usb::deInit();
                }
                break;

                case usb_over_serial::internalCmd_t::CONNECT_USB:
                {
                    if (!usb::isUsbConnected())
                    {
                        board::usb::init();
                    }
                }
                break;

                case usb_over_serial::internalCmd_t::UNIQUE_ID:
                {
                    sendUniqueId();
                }
                break;

                case usb_over_serial::internalCmd_t::LINK_READY:
                {
                    sendLinkReady();
                }
                break;

                case usb_over_serial::internalCmd_t::FACTORY_RESET:
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