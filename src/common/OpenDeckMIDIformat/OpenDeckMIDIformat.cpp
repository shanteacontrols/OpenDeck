/*

Copyright 2015-2019 Igor Petrovic

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

#include "OpenDeckMIDIformat.h"
#include "board/Board.h"

namespace OpenDeckMIDIformat
{
    bool write(uint8_t channel, MIDI::USBMIDIpacket_t& USBMIDIpacket, packetType_t packetType)
    {
        if (channel >= UART_INTERFACES)
            return false;

        Board::UART::write(channel, static_cast<uint8_t>(packetType));
        Board::UART::write(channel, USBMIDIpacket.Event);
        Board::UART::write(channel, USBMIDIpacket.Data1);
        Board::UART::write(channel, USBMIDIpacket.Data2);
        Board::UART::write(channel, USBMIDIpacket.Data3);
        Board::UART::write(channel, USBMIDIpacket.Event ^ USBMIDIpacket.Data1 ^ USBMIDIpacket.Data2 ^ USBMIDIpacket.Data3);

        return true;
    }

    bool read(uint8_t channel, MIDI::USBMIDIpacket_t& USBMIDIpacket, packetType_t& packetType)
    {
        packetType = packetType_t::invalid;

        if (channel >= UART_INTERFACES)
            return false;

        if (Board::UART::bytesAvailableRx(channel) >= 6)
        {
            uint8_t data = 0;
            Board::UART::read(channel, data);
            uint8_t dataXOR = 0;

            if ((data == static_cast<uint8_t>(packetType_t::midi)) || (data == static_cast<uint8_t>(packetType_t::midiDaisyChain)))
            {
                packetType = static_cast<packetType_t>(data);

                //start of frame, read rest of the packet
                for (int i = 0; i < 5; i++)
                {
                    Board::UART::read(channel, data);

                    switch (i)
                    {
                    case 0:
                        USBMIDIpacket.Event = data;
                        break;

                    case 1:
                        USBMIDIpacket.Data1 = data;
                        break;

                    case 2:
                        USBMIDIpacket.Data2 = data;
                        break;

                    case 3:
                        USBMIDIpacket.Data3 = data;
                        break;

                    case 4:
                        //xor byte, do nothing
                        break;
                    }
                }

                //error check
                dataXOR = USBMIDIpacket.Event ^ USBMIDIpacket.Data1 ^ USBMIDIpacket.Data2 ^ USBMIDIpacket.Data3;

                return (dataXOR == data);
            }
            else if (data == static_cast<uint8_t>(packetType_t::internalCommand))
            {
                uint8_t cmd = 0;
                Board::UART::read(channel, cmd);

                //ignore the rest of the buffer
                for (int i = 0; i < 4; i++)
                    Board::UART::read(channel, data);

                switch (static_cast<command_t>(cmd))
                {
                case command_t::fwUpdated:
                    Board::ledFlashStartup(true);
                    break;

                case command_t::fwNotUpdated:
                    Board::ledFlashStartup(false);
                    break;

                case command_t::btldrReboot:
                    Board::reboot(Board::rebootType_t::rebootBtldr);
                    break;

                default:
                    break;
                }

                packetType = packetType_t::internalCommand;
                return true;
            }
        }

        return false;
    }
}    // namespace OpenDeckMIDIformat