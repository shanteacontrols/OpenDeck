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
#include "board/common/comm/USBOverSerial/USBOverSerial.h"
#include "Commands.h"
#include "core/src/general/Timing.h"
#include "protocol/midi/MIDI.h"

using namespace Protocol;

/// Time in milliseconds after which USB connection state should be checked
#define USB_CONN_CHECK_TIME 2000

namespace
{
    uint8_t                             uartReadBuffer[USB_OVER_SERIAL_BUFFER_SIZE];
    uint8_t                             cdcReadBuffer[USB_OVER_SERIAL_BUFFER_SIZE];
    Board::USBOverSerial::USBReadPacket readPacket(uartReadBuffer, USB_OVER_SERIAL_BUFFER_SIZE);
    MIDI::usbMIDIPacket_t               usbMIDIPacket;
    size_t                              cdcPacketSize;

    void checkUSBconnection()
    {
        static uint32_t lastCheckTime       = 0;
        static bool     lastConnectionState = false;

        if (core::timing::currentRunTimeMs() - lastCheckTime > USB_CONN_CHECK_TIME)
        {
            bool newState = Board::USB::isUSBconnected();

            if (lastConnectionState != newState)
            {
                uint8_t data[2] = {
                    static_cast<uint8_t>(USBLink::internalCMD_t::USB_STATE),
                    newState,
                };

                Board::USBOverSerial::USBWritePacket packet(Board::USBOverSerial::packetType_t::INTERNAL,
                                                            data,
                                                            2,
                                                            USB_OVER_SERIAL_BUFFER_SIZE);
                Board::USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);

                lastConnectionState = newState;
            }

            lastCheckTime = core::timing::currentRunTimeMs();
        }
    }

    void sendUniqueID()
    {
        Board::uniqueID_t uniqueID;
        Board::uniqueID(uniqueID);

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

        Board::USBOverSerial::USBWritePacket packet(Board::USBOverSerial::packetType_t::INTERNAL,
                                                    data,
                                                    11,
                                                    USB_OVER_SERIAL_BUFFER_SIZE);
        Board::USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
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

        Board::USBOverSerial::USBWritePacket packet(Board::USBOverSerial::packetType_t::INTERNAL,
                                                    data,
                                                    5,
                                                    USB_OVER_SERIAL_BUFFER_SIZE);
        Board::USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
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
        if (Board::USB::readMIDI(usbMIDIPacket))
        {
            Board::USBOverSerial::USBWritePacket packet(Board::USBOverSerial::packetType_t::MIDI,
                                                        &usbMIDIPacket[0],
                                                        4,
                                                        USB_OVER_SERIAL_BUFFER_SIZE);

            if (Board::USBOverSerial::write(UART_CHANNEL_USB_LINK, packet))
            {
                Board::io::indicateTraffic(Board::io::dataSource_t::USB, Board::io::dataDirection_t::INCOMING);
            }
        }

        // USB CDC -> UART
        if (Board::USB::readCDC(cdcReadBuffer, cdcPacketSize, USB_OVER_SERIAL_BUFFER_SIZE))
        {
            Board::USBOverSerial::USBWritePacket packet(Board::USBOverSerial::packetType_t::CDC,
                                                        cdcReadBuffer,
                                                        cdcPacketSize,
                                                        USB_OVER_SERIAL_BUFFER_SIZE);

            if (Board::USBOverSerial::write(UART_CHANNEL_USB_LINK, packet))
            {
                Board::io::indicateTraffic(Board::io::dataSource_t::USB, Board::io::dataDirection_t::INCOMING);
            }
        }

        // UART -> USB
        if (Board::USBOverSerial::read(UART_CHANNEL_USB_LINK, readPacket))
        {
            if (readPacket.type() == Board::USBOverSerial::packetType_t::MIDI)
            {
                for (size_t i = 0; i < sizeof(usbMIDIPacket); i++)
                {
                    usbMIDIPacket[i] = readPacket[i];
                }

                if (Board::USB::writeMIDI(usbMIDIPacket))
                {
                    Board::io::indicateTraffic(Board::io::dataSource_t::USB, Board::io::dataDirection_t::OUTGOING);
                }
            }
            else if (readPacket.type() == Board::USBOverSerial::packetType_t::INTERNAL)
            {
                // internal command
                if (readPacket[0] == static_cast<uint8_t>(USBLink::internalCMD_t::REBOOT_BTLDR))
                {
                    // use received data as the magic bootloader value
                    Board::bootloader::setMagicBootValue(readPacket[1]);
                    Board::reboot();
                }
            }
            else if (readPacket.type() == Board::USBOverSerial::packetType_t::CDC)
            {
                if (Board::USB::writeCDC(readPacket.buffer(), readPacket.size()))
                {
                    Board::io::indicateTraffic(Board::io::dataSource_t::USB, Board::io::dataDirection_t::OUTGOING);
                }
            }

            // clear out any stored information in packet since we are reusing it
            readPacket.reset();
        }

        checkUSBconnection();
    }
}