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

#include "board/Board.h"
#include "board/common/comm/USBOverSerial/USBOverSerial.h"
#include "Commands.h"
#include "core/src/general/Timing.h"

/// Time in milliseconds after which USB connection state should be checked
#define USB_CONN_CHECK_TIME 2000

namespace
{
    uint8_t                             readBuffer[USB_OVER_SERIAL_BUFFER_SIZE];
    Board::USBOverSerial::USBReadPacket readPacket(readBuffer, USB_OVER_SERIAL_BUFFER_SIZE);
    MIDI::USBMIDIpacket_t               USBMIDIpacket;
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
                    static_cast<uint8_t>(USBLink::internalCMD_t::usbState),
                    newState,
                };

                Board::USBOverSerial::USBWritePacket packet(Board::USBOverSerial::packetType_t::internal,
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
            static_cast<uint8_t>(USBLink::internalCMD_t::uniqueID),
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

        Board::USBOverSerial::USBWritePacket packet(Board::USBOverSerial::packetType_t::internal,
                                                    data,
                                                    11,
                                                    USB_OVER_SERIAL_BUFFER_SIZE);
        Board::USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
    }
}    // namespace

namespace Board
{
    namespace USB
    {
        void onCDCsetLineEncoding(uint32_t baudRate)
        {
            uint8_t data[5] = {
                static_cast<uint8_t>(USBLink::internalCMD_t::baudRateChange),
                static_cast<uint8_t>((baudRate) >> 0 & 0xFF),
                static_cast<uint8_t>((baudRate >> 8) & 0xFF),
                static_cast<uint8_t>((baudRate >> 16) & 0xFF),
                static_cast<uint8_t>((baudRate >> 24) & 0xFF),
            };

            Board::USBOverSerial::USBWritePacket packet(Board::USBOverSerial::packetType_t::internal,
                                                        data,
                                                        5,
                                                        USB_OVER_SERIAL_BUFFER_SIZE);
            Board::USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
        }
    }    // namespace USB
}    // namespace Board

int main(void)
{
    Board::init();

    // make sure device is ready before sending unique id
    core::timing::waitMs(50);
    sendUniqueID();

    while (1)
    {
        // USB MIDI -> UART
        if (Board::USB::readMIDI(USBMIDIpacket))
        {
            uint8_t                              data[4] = { USBMIDIpacket.Event, USBMIDIpacket.Data1, USBMIDIpacket.Data2, USBMIDIpacket.Data3 };
            Board::USBOverSerial::USBWritePacket packet(Board::USBOverSerial::packetType_t::midi,
                                                        data,
                                                        4,
                                                        USB_OVER_SERIAL_BUFFER_SIZE);

            if (Board::USBOverSerial::write(UART_CHANNEL_USB_LINK, packet))
                Board::io::indicateTraffic(Board::io::dataSource_t::usb, Board::io::dataDirection_t::incoming);
        }

        // USB CDC -> UART
        if (Board::USB::readCDC(readBuffer, cdcPacketSize, USB_OVER_SERIAL_BUFFER_SIZE))
        {
            Board::USBOverSerial::USBWritePacket packet(Board::USBOverSerial::packetType_t::cdc,
                                                        readBuffer,
                                                        cdcPacketSize,
                                                        USB_OVER_SERIAL_BUFFER_SIZE);

            if (Board::USBOverSerial::write(UART_CHANNEL_USB_LINK, packet))
                Board::io::indicateTraffic(Board::io::dataSource_t::usb, Board::io::dataDirection_t::incoming);
        }

        // UART -> USB
        if (Board::USBOverSerial::read(UART_CHANNEL_USB_LINK, readPacket))
        {
            if (readPacket.type() == Board::USBOverSerial::packetType_t::midi)
            {
                USBMIDIpacket.Event = readPacket[0];
                USBMIDIpacket.Data1 = readPacket[1];
                USBMIDIpacket.Data2 = readPacket[2];
                USBMIDIpacket.Data3 = readPacket[3];

                if (Board::USB::writeMIDI(USBMIDIpacket))
                    Board::io::indicateTraffic(Board::io::dataSource_t::usb, Board::io::dataDirection_t::outgoing);
            }
            else if (readPacket.type() == Board::USBOverSerial::packetType_t::internal)
            {
                // internal command
                if (readPacket[0] == static_cast<uint8_t>(USBLink::internalCMD_t::rebootBTLDR))
                {
                    // use received data as the magic bootloader value
                    Board::bootloader::setMagicBootValue(readPacket[1]);
                    Board::reboot();
                }
            }
            else if (readPacket.type() == Board::USBOverSerial::packetType_t::cdc)
            {
                if (Board::USB::writeCDC(readPacket.buffer(), readPacket.size()))
                    Board::io::indicateTraffic(Board::io::dataSource_t::usb, Board::io::dataDirection_t::outgoing);
            }

            // clear out any stored information in packet since we are reusing it
            readPacket.reset();
        }

        checkUSBconnection();
    }
}