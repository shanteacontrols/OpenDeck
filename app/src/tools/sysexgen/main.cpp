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

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

namespace
{
    constexpr const char* INPUT_FILE_PATH  = SYSEXGEN_INPUT_FILE;
    constexpr const char* OUTPUT_FILE_PATH = SYSEXGEN_OUTPUT_FILE;
}    // namespace

int main()
{
    int                  ret = 0;
    std::ifstream        stream(INPUT_FILE_PATH, std::ios::in | std::ios::binary);
    std::vector<uint8_t> contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    std::fstream         outputFile = {};

    if (!stream.is_open())
    {
        std::cout << "ERROR: Unable to open input file " << INPUT_FILE_PATH << std::endl;
        return -1;
    }

    std::cout << "Firmware update file size is "
              << contents.size() << " bytes. Generating SysEx file, please wait..."
              << std::endl;

    std::vector<uint8_t> output = sysexgen::generate(contents, PROJECT_TARGET_UID);

    outputFile.open(OUTPUT_FILE_PATH, std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
    outputFile.unsetf(std::ios::skipws);

    if (outputFile.is_open())
    {
        outputFile.write(reinterpret_cast<char*>(&output[0]), output.size() * sizeof(uint8_t));
        outputFile.close();
        return ret;
    }

    std::cout << "ERROR: Unable to open output file " << OUTPUT_FILE_PATH << std::endl;
    return -1;
}
