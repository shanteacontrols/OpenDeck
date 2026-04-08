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

#include "application/util/conversion/conversion.h"
#include "bootloader/updater/updater.h"
#include "application/system/config.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <iterator>
#include <string>
#include <cstddef>

namespace
{
    constexpr size_t BYTES_PER_FW_MESSAGE = 32;

    void appendSysExId(std::vector<uint8_t>& vec)
    {
        using namespace sys;

        vec.push_back(Config::SYSEX_MANUFACTURER_ID_0);
        vec.push_back(Config::SYSEX_MANUFACTURER_ID_1);
        vec.push_back(Config::SYSEX_MANUFACTURER_ID_2);
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

                commandArray.push_back((command >> (shiftAmount + 0) & 0xFF));
                commandArray.push_back((command >> (shiftAmount + 8) & 0xFF));
            }
            else if (bytes == 4)
            {
                uint64_t shiftAmount = 32 * i;

                commandArray.push_back((command >> (shiftAmount + 0) & 0xFF));
                commandArray.push_back((command >> (shiftAmount + 8) & 0xFF));
                commandArray.push_back((command >> (shiftAmount + 16) & 0xFF));
                commandArray.push_back((command >> (shiftAmount + 24) & 0xFF));
            }
            else
            {
                std::cout << "Incorrect number of bytes per command message specified"
                          << std::endl;
                exit(1);
            }

            for (size_t i = 0; i < commandArray.size(); i++)
            {
                auto split = util::Conversion::Split14Bit(commandArray.at(i));

                output.push_back(split.high());
                output.push_back(split.low());
            }

            output.push_back(0xF7);
        }
    }
}    // namespace

int main(int argc, char* argv[])
{
    // first argument should be path to the binary file
    // second argument should be path of the output file
    if (argc <= 2)
    {
        std::cout << argv[0] << "ERROR: Input and output filenames not provided" << std::endl;
        return -1;
    }

    std::ifstream        stream(argv[1], std::ios::in | std::ios::binary);
    std::vector<uint8_t> contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    std::vector<uint8_t> output     = {};
    std::fstream         outputFile = {};

    // make sure firmware size is multiple of 4 (parser operates on 4-byte values)
    while (contents.size() % 4)
    {
        contents.push_back(0xFF);
    }

    std::cout << "Firmware update file size is "
              << contents.size() << " bytes. Generating SysEx file, please wait..."
              << std::endl;

    appendCommand(updater::START_COMMAND, 4, output);

    output.push_back(0xF0);
    appendSysExId(output);

    for (size_t i = 0; i < 4; i++)
    {
        auto split = util::Conversion::Split14Bit(contents.size() >> (8 * i) & 0xFF);

        output.push_back(split.high());
        output.push_back(split.low());
    }

    for (size_t i = 0; i < 4; i++)
    {
        auto split = util::Conversion::Split14Bit(PROJECT_TARGET_UID >> (8 * i) & 0xFF);

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
            lastByteSet = 0;
        }

        auto split = util::Conversion::Split14Bit(contents.at(i));

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

    outputFile.open(argv[2], std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
    outputFile.unsetf(std::ios::skipws);

    if (outputFile.is_open())
    {
        outputFile.write(reinterpret_cast<char*>(&output[0]), output.size() * sizeof(uint8_t));
        outputFile.close();
    }

    return 0;
}