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
#include "board/common/communication/USBOverSerial/USBOverSerial.h"
#include "Commands.h"
#include "core/src/Timing.h"
#include "core/src/MCU.h"
#include "protocol/midi/MIDI.h"

using namespace Board;
using namespace Protocol;

namespace
{
    /// Time in milliseconds after which USB connection state should be checked
    constexpr uint32_t           USB_CONN_CHECK_TIME = 2000;
    uint8_t                      uartReadBuffer[USB_OVER_SERIAL_BUFFER_SIZE];
    uint8_t                      cdcReadBuffer[USB_OVER_SERIAL_BUFFER_SIZE];
    USBOverSerial::USBReadPacket readPacket(uartReadBuffer, USB_OVER_SERIAL_BUFFER_SIZE);
    MIDI::usbMIDIPacket_t        usbMIDIPacket;
    size_t                       cdcPacketSize;

    void checkUSBconnection()
    {
        static uint32_t lastCheckTime       = 0;
        static bool     lastConnectionState = false;

        if (core::timing::ms() - lastCheckTime > USB_CONN_CHECK_TIME)
        {
            bool newState = USB::isUSBconnected();

            if (lastConnectionState != newState)
            {
                uint8_t data[2] = {
                    static_cast<uint8_t>(USBLink::internalCMD_t::USB_STATE),
                    newState,
                };

                USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::INTERNAL,
                                                     data,
                                                     2,
                                                     USB_OVER_SERIAL_BUFFER_SIZE);
                USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);

                lastConnectionState = newState;
            }

            lastCheckTime = core::timing::ms();
        }
    }

    void sendUniqueID()
    {
        core::mcu::uniqueID_t uniqueID;
        core::mcu::uniqueID(uniqueID);

        uint8_t data[11] = {
            static_cast<uint8_t>(USBLink::internalCMD_t::UNIQUE_ID),
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

        USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::INTERNAL,
                                             data,
                                             11,
                                             USB_OVER_SERIAL_BUFFER_SIZE);
        USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
    }
}    // namespace

namespace Board::USB
{
    void onCDCsetLineEncoding(uint32_t baudRate)
    {
        uint8_t data[5] = {
            static_cast<uint8_t>(USBLink::internalCMD_t::BAUDRATE_CHANGE),
            static_cast<uint8_t>((baudRate) >> 0 & 0xFF),
            static_cast<uint8_t>((baudRate >> 8) & 0xFF),
            static_cast<uint8_t>((baudRate >> 16) & 0xFF),
            static_cast<uint8_t>((baudRate >> 24) & 0xFF),
        };

        USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::INTERNAL,
                                             data,
                                             5,
                                             USB_OVER_SERIAL_BUFFER_SIZE);
        USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
    }
}    // namespace Board::USB

int main()
{
    Board::init();

    // make sure device is ready before sending unique id
    core::timing::waitMs(50);
    sendUniqueID();

    while (1)
    {
        // USB MIDI -> UART
        if (USB::readMIDI(usbMIDIPacket))
        {
            USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::MIDI,
                                                 &usbMIDIPacket[0],
                                                 4,
                                                 USB_OVER_SERIAL_BUFFER_SIZE);

            if (USBOverSerial::write(UART_CHANNEL_USB_LINK, packet))
            {
                Board::IO::indicators::indicateTraffic(Board::IO::indicators::dataSource_t::USB,
                                                       Board::IO::indicators::dataDirection_t::INCOMING);
            }
        }

        // USB CDC -> UART
        if (USB::readCDC(cdcReadBuffer, cdcPacketSize, USB_OVER_SERIAL_BUFFER_SIZE))
        {
            USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::CDC,
                                                 cdcReadBuffer,
                                                 cdcPacketSize,
                                                 USB_OVER_SERIAL_BUFFER_SIZE);

            if (USBOverSerial::write(UART_CHANNEL_USB_LINK, packet))
            {
                Board::IO::indicators::indicateTraffic(Board::IO::indicators::dataSource_t::USB,
                                                       Board::IO::indicators::dataDirection_t::INCOMING);
            }
        }

        // UART -> USB
        if (USBOverSerial::read(UART_CHANNEL_USB_LINK, readPacket))
        {
            if (readPacket.type() == USBOverSerial::packetType_t::MIDI)
            {
                for (size_t i = 0; i < sizeof(usbMIDIPacket); i++)
                {
                    usbMIDIPacket[i] = readPacket[i];
                }

                if (USB::writeMIDI(usbMIDIPacket))
                {
                    Board::IO::indicators::indicateTraffic(Board::IO::indicators::dataSource_t::USB,
                                                           Board::IO::indicators::dataDirection_t::OUTGOING);
                }
            }
            else if (readPacket.type() == USBOverSerial::packetType_t::INTERNAL)
            {
                // internal command
                if (readPacket[0] == static_cast<uint8_t>(USBLink::internalCMD_t::REBOOT_BTLDR))
                {
                    // use received data as the magic bootloader value
                    bootloader::setMagicBootValue(readPacket[1]);
                    reboot();
                }
            }
            else if (readPacket.type() == USBOverSerial::packetType_t::CDC)
            {
                if (USB::writeCDC(readPacket.buffer(), readPacket.size()))
                {
                    Board::IO::indicators::indicateTraffic(Board::IO::indicators::dataSource_t::USB,
                                                           Board::IO::indicators::dataDirection_t::OUTGOING);
                }
            }

            // clear out any stored information in packet since we are reusing it
            readPacket.reset();
        }

        checkUSBconnection();
    }
}