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

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <iterator>
#include <string>
#include <cstddef>
#include "midi/src/MIDI.h"

#define BYTES_PER_FW_MESSAGE 32

void appendCommand(uint64_t command, size_t bytes, std::vector<uint8_t>& output)
{
    for (int i = 0; i < 2; i++)
    {
        output.push_back(0xF0);
        output.push_back(SYSEX_MANUFACTURER_ID_0);
        output.push_back(SYSEX_MANUFACTURER_ID_1);
        output.push_back(SYSEX_MANUFACTURER_ID_2);

        std::vector<uint8_t> commandArray;

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
            printf("Incorrect number of bytes per command message specified\n");
            exit(1);
        }

        for (size_t i = 0; i < commandArray.size(); i++)
        {
            MIDI::Split14bit split14bit;
            split14bit.split(commandArray.at(i));

            output.push_back(split14bit.high());
            output.push_back(split14bit.low());
        }

        output.push_back(0xF7);
    }
}

int main(int argc, char* argv[])
{
    //first argument should be path to the binary file
    //second argument should be path of the output file
    if (argc <= 2)
    {
        std::cout << argv[0] << "ERROR: Input and output filenames not provided" << std::endl;
        return -1;
    }

    std::ifstream        stream(argv[1], std::ios::in | std::ios::binary);
    std::vector<uint8_t> contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    std::vector<uint8_t> output;
    std::fstream         outputFile;

    printf("Firmware size is %lu bytes. Generating SysEx file, please wait...\n", contents.size());

    appendCommand(COMMAND_FW_UPDATE_START, 4, output);

    output.push_back(0xF0);
    output.push_back(SYSEX_MANUFACTURER_ID_0);
    output.push_back(SYSEX_MANUFACTURER_ID_1);
    output.push_back(SYSEX_MANUFACTURER_ID_2);

    for (size_t i = 0; i < 4; i++)
    {
        MIDI::Split14bit split14bit;
        split14bit.split(contents.size() >> (8 * i) & 0xFF);

        output.push_back(split14bit.high());
        output.push_back(split14bit.low());
    }

    for (size_t i = 0; i < 4; i++)
    {
        MIDI::Split14bit split14bit;
        split14bit.split(FW_UID >> (8 * i) & 0xFF);

        output.push_back(split14bit.high());
        output.push_back(split14bit.low());
    }

    output.push_back(0xF7);

    uint32_t byteCounter = 0;
    bool     lastByteSet = false;

    for (size_t i = 0; i < contents.size(); i++)
    {
        if (!byteCounter)
        {
            output.push_back(0xF0);
            output.push_back(SYSEX_MANUFACTURER_ID_0);
            output.push_back(SYSEX_MANUFACTURER_ID_1);
            output.push_back(SYSEX_MANUFACTURER_ID_2);
            lastByteSet = 0;
        }

        MIDI::Split14bit split14bit;
        split14bit.split(contents.at(i));

        output.push_back(split14bit.high());
        output.push_back(split14bit.low());

        byteCounter++;

        if (byteCounter == BYTES_PER_FW_MESSAGE)
        {
            byteCounter = 0;
            output.push_back(0xF7);
            lastByteSet = true;
        }
    }

    if (!lastByteSet)
        output.push_back(0xF7);

    appendCommand(COMMAND_FW_UPDATE_END, 2, output);

    outputFile.open(argv[2], std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
    outputFile.unsetf(std::ios::skipws);

    if (outputFile.is_open())
    {
        outputFile.write(reinterpret_cast<char*>(&output[0]), output.size() * sizeof(uint8_t));
        outputFile.close();
    }

    return 0;
}