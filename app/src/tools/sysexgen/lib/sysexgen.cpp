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

#include "sysexgen.h"

#include "bootloader/updater/common.h"

#include <cstdlib>
#include <vector>

namespace
{
    constexpr size_t  BYTES_PER_FW_MESSAGE    = 32;
    constexpr uint8_t SYSEX_MANUFACTURER_ID_0 = 0x00;
    constexpr uint8_t SYSEX_MANUFACTURER_ID_1 = 0x53;
    constexpr uint8_t SYSEX_MANUFACTURER_ID_2 = 0x43;

    class Split14Bit
    {
        public:
        constexpr explicit Split14Bit(uint16_t value)
        {
            constexpr uint8_t  BITS_IN_BYTE       = 8;
            constexpr uint16_t BYTE_MASK          = 0xFF;
            constexpr uint8_t  MIDI_DATA_BYTE_MAX = 0x7F;

            uint8_t new_high = (value >> BITS_IN_BYTE) & BYTE_MASK;
            uint8_t new_low  = value & BYTE_MASK;

            new_high = (new_high << 1) & MIDI_DATA_BYTE_MAX;

            if ((new_low >> (BITS_IN_BYTE - 1)) & 0x01)
            {
                new_high |= 0x01;
            }
            else
            {
                new_high &= ~0x01;
            }

            new_low &= MIDI_DATA_BYTE_MAX;
            _high = new_high;
            _low  = new_low;
        }

        constexpr uint8_t high() const
        {
            return _high;
        }

        constexpr uint8_t low() const
        {
            return _low;
        }

        private:
        uint8_t _high = 0;
        uint8_t _low  = 0;
    };

    void appendSysExId(std::vector<uint8_t>& vec)
    {
        vec.push_back(SYSEX_MANUFACTURER_ID_0);
        vec.push_back(SYSEX_MANUFACTURER_ID_1);
        vec.push_back(SYSEX_MANUFACTURER_ID_2);
    }

    void appendCommand(uint64_t command, size_t bytes, std::vector<uint8_t>& output)
    {
        for (int i = 0; i < 2; i++)
        {
            output.push_back(0xF0);

            appendSysExId(output);

            std::vector<uint8_t> commandArray = {};

            if (bytes == 2)
            {
                uint64_t shiftAmount = 16 * i;

                commandArray.push_back((command >> (shiftAmount + 0)) & 0xFF);
                commandArray.push_back((command >> (shiftAmount + 8)) & 0xFF);
            }
            else if (bytes == 4)
            {
                uint64_t shiftAmount = 32 * i;

                commandArray.push_back((command >> (shiftAmount + 0)) & 0xFF);
                commandArray.push_back((command >> (shiftAmount + 8)) & 0xFF);
                commandArray.push_back((command >> (shiftAmount + 16)) & 0xFF);
                commandArray.push_back((command >> (shiftAmount + 24)) & 0xFF);
            }
            else
            {
                std::exit(1);
            }

            for (size_t byteIndex = 0; byteIndex < commandArray.size(); byteIndex++)
            {
                auto split = Split14Bit(commandArray.at(byteIndex));

                output.push_back(split.high());
                output.push_back(split.low());
            }

            output.push_back(0xF7);
        }
    }
}    // namespace

std::vector<uint8_t> sysexgen::generate(const std::vector<uint8_t>& firmware, uint32_t targetUid)
{
    std::vector<uint8_t> contents = firmware;
    std::vector<uint8_t> output   = {};

    while (contents.size() % 4)
    {
        contents.push_back(0xFF);
    }

    appendCommand(updater::START_COMMAND, 4, output);

    output.push_back(0xF0);
    appendSysExId(output);

    for (size_t i = 0; i < 4; i++)
    {
        auto split = Split14Bit((contents.size() >> (8 * i)) & 0xFF);

        output.push_back(split.high());
        output.push_back(split.low());
    }

    for (size_t i = 0; i < 4; i++)
    {
        auto split = Split14Bit((targetUid >> (8 * i)) & 0xFF);

        output.push_back(split.high());
        output.push_back(split.low());
    }

    output.push_back(0xF7);

    uint32_t byteCounter = 0;
    bool     lastByteSet = false;

    for (size_t i = 0; i < contents.size(); i++)
    {
        if (!byteCounter)
        {
            output.push_back(0xF0);
            appendSysExId(output);
            lastByteSet = false;
        }

        auto split = Split14Bit(contents.at(i));

        output.push_back(split.high());
        output.push_back(split.low());

        byteCounter++;

        if (byteCounter == BYTES_PER_FW_MESSAGE)
        {
            byteCounter = 0;
            output.push_back(0xF7);
            lastByteSet = true;
        }
    }

    if (!lastByteSet)
    {
        output.push_back(0xF7);
    }

    appendCommand(updater::END_COMMAND, 2, output);

    return output;
}
