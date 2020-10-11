/*

Copyright 2015-2020 Igor Petrovic

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

#include "OpenDeckMIDIformat.h"
#include "board/Board.h"

namespace OpenDeckMIDIformat
{
    namespace
    {
        ///
        /// \brief List of all bytes contained within OpenDeck packet.
        ///
        enum class packet_t : uint8_t
        {
            packetType,
            event,
            data1,
            data2,
            data3,
            dataXor
        };

        uint8_t incomingBytesCount = 0;
        uint8_t readBytes[6]       = {};
        uint8_t tempArray[5];

        ///
        /// \brief Appends read byte into internal storage.
        /// If the buffer is full and valid packet still hasn't been
        /// found, this function will shift all stored bytes by one
        /// position to the left and write current byte to the
        /// last position.
        ///
        void appendIncoming(uint8_t byte)
        {
            if (incomingBytesCount >= 6)
            {
                //shift data
                for (int i = 0; i < 5; i++)
                    tempArray[i] = readBytes[i + 1];

                for (int i = 0; i < 5; i++)
                    readBytes[i] = tempArray[i];

                readBytes[5] = byte;
            }
            else
            {
                readBytes[incomingBytesCount++] = byte;
            }
        }

        ///
        /// \brief Clears the entire incoming buffer.
        /// Should be called when valid packet has been processed.
        ///
        void clearIncomingCount()
        {
            incomingBytesCount = 0;

            for (int i = 0; i < 6; i++)
                readBytes[i] = 0;
        }
    }    // namespace

    bool write(uint8_t channel, MIDI::USBMIDIpacket_t& USBMIDIpacket, packetType_t packetType)
    {
        if (!Board::UART::write(channel, static_cast<uint8_t>(packetType)))
            return false;

        if (!Board::UART::write(channel, USBMIDIpacket.Event))
            return false;

        if (!Board::UART::write(channel, USBMIDIpacket.Data1))
            return false;

        if (!Board::UART::write(channel, USBMIDIpacket.Data2))
            return false;

        if (!Board::UART::write(channel, USBMIDIpacket.Data3))
            return false;

        if (!Board::UART::write(channel, USBMIDIpacket.Event ^ USBMIDIpacket.Data1 ^ USBMIDIpacket.Data2 ^ USBMIDIpacket.Data3))
            return false;

        return true;
    }

    bool read(uint8_t channel, MIDI::USBMIDIpacket_t& USBMIDIpacket, packetType_t& packetType)
    {
        for (int i = 0; i < 6; i++)
        {
            uint8_t value;

            if (!Board::UART::read(channel, value))
                break;
            else
                appendIncoming(value);

            if (incomingBytesCount == 6)
                break;
        }

        if (incomingBytesCount != 6)
            return false;

        bool packetTypeValid = true;

        if (readBytes[static_cast<uint8_t>(packet_t::packetType)] == static_cast<uint8_t>(packetType_t::midi))
            packetType = static_cast<packetType_t>(readBytes[static_cast<uint8_t>(packet_t::packetType)]);
        else if (readBytes[static_cast<uint8_t>(packet_t::packetType)] == static_cast<uint8_t>(packetType_t::internalCommand))
            packetType = packetType_t::internalCommand;
        else
            packetTypeValid = false;

        if (packetTypeValid)
        {
            //read the rest of the packet
            for (int i = 0; i < 5; i++)
            {
                switch (i)
                {
                case 0:
                    USBMIDIpacket.Event = readBytes[static_cast<uint8_t>(packet_t::event)];
                    break;

                case 1:
                    USBMIDIpacket.Data1 = readBytes[static_cast<uint8_t>(packet_t::data1)];
                    break;

                case 2:
                    USBMIDIpacket.Data2 = readBytes[static_cast<uint8_t>(packet_t::data2)];
                    break;

                case 3:
                    USBMIDIpacket.Data3 = readBytes[static_cast<uint8_t>(packet_t::data3)];
                    break;

                case 4:
                    //xor byte - do nothing
                    break;
                }
            }

            if (readBytes[static_cast<uint8_t>(packet_t::dataXor)] == (USBMIDIpacket.Event ^ USBMIDIpacket.Data1 ^ USBMIDIpacket.Data2 ^ USBMIDIpacket.Data3))
            {
                clearIncomingCount();

                if (packetType == packetType_t::internalCommand)
                {
                    switch (static_cast<command_t>(USBMIDIpacket.Event))
                    {
                    case command_t::btldrReboot:
                        Board::reboot(Board::rebootType_t::rebootBtldr);
                        break;

                    case command_t::appReboot:
                        Board::reboot(Board::rebootType_t::rebootApp);
                        break;

                    default:
                        return false;
                    }
                }

                return true;
            }
        }

        return false;
    }
}    // namespace OpenDeckMIDIformat

#endif