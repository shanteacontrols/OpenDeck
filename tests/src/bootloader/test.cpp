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

#ifndef PROJECT_TARGET_USB_OVER_SERIAL_HOST
#ifdef PROJECT_MCU_SUPPORT_BOOTLOADER

#include "tests/common.h"
#include "tests/helpers/midi.h"
#include "sysex_parser/sysex_parser.h"
#include "bootloader/updater/builder_test.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <iterator>
#include <string>
#include <cstddef>

namespace
{
    const std::string fw_build_type_subdir = "release/";
    const std::string FW_UPDATE_FILE_SYSEX = "firmware.syx";
    const std::string FW_UPDATE_FILE_BIN   = "firmware.bin";

    sysex_parser::SysExParser sysExParser;
    updater::BuilderTest      builderUpdater;
    test::MIDIHelper          helper;
}    // namespace

TEST(Bootloader, FwUpdate)
{
    if (!std::filesystem::exists(FW_UPDATE_FILE_SYSEX))
    {
        LOG(ERROR) << FW_UPDATE_FILE_SYSEX << " doesn't exist";
        ASSERT_TRUE(true == false);
    }

    if (!std::filesystem::exists(FW_UPDATE_FILE_BIN))
    {
        LOG(ERROR) << FW_UPDATE_FILE_SYSEX << " doesn't exist";
        ASSERT_TRUE(true == false);
    }

    std::ifstream        sysExStream(FW_UPDATE_FILE_SYSEX, std::ios::in | std::ios::binary);
    std::vector<uint8_t> sysExVector((std::istreambuf_iterator<char>(sysExStream)), std::istreambuf_iterator<char>());
    std::ifstream        binaryStream(FW_UPDATE_FILE_BIN, std::ios::in | std::ios::binary);
    std::vector<uint8_t> binaryVector((std::istreambuf_iterator<char>(binaryStream)), std::istreambuf_iterator<char>());

    std::vector<uint8_t>         singleSysExMsg = {};
    std::vector<midi::UsbPacket> packets        = {};

    // Go over the entire SysEx file.
    // Upon reaching the end of single SysEx message, convert it
    // into series of USB MIDI packets.
    for (size_t i = 0; i < sysExVector.size(); i++)
    {
        singleSysExMsg.push_back(sysExVector.at(i));

        if (sysExVector.at(i) == 0xF7)
        {
            auto converted = helper.rawSysExToUSBPackets(singleSysExMsg);
            packets.insert(std::end(packets), std::begin(converted), std::end(converted));
            singleSysExMsg.clear();
        }
    }

    // Now we have the entire file in form of USB MIDI packets.
    // Parse each message and once parsing passes, feed the parsed data into FW updater.
    for (size_t packet = 0; packet < packets.size(); packet++)
    {
        if (sysExParser.isValidMessage(packets.at(packet)))
        {
            size_t  dataSize = sysExParser.dataBytes();
            uint8_t data     = 0;

            if (dataSize)
            {
                for (size_t i = 0; i < dataSize; i++)
                {
                    if (sysExParser.value(i, data))
                    {
                        builderUpdater.instance().feed(data);
                    }
                }
            }
        }
    }

    // once all data has been fed into updater, firmware update procedure should be complete
    ASSERT_TRUE(builderUpdater._hwa._updated);

    // written content should also match the original binary file from which SysEx file has been created
    // verify only until binaryVector.size() -> writtenBytes vector could be slightly larger due to padding
    for (size_t i = 0; i < binaryVector.size(); i++)
    {
        if (builderUpdater._hwa._writtenBytes.at(i) != binaryVector.at(i))
        {
            LOG(ERROR) << "Difference on byte " << i;
            ASSERT_TRUE(true == false);
        }
    }

    if (binaryVector.size() != builderUpdater._hwa._writtenBytes.size())
    {
        LOG(INFO) << "Expecting padding in firmware file";

        // now verify padding if present
        for (size_t i = binaryVector.size(); i < builderUpdater._hwa._writtenBytes.size(); i++)
        {
            ASSERT_EQ(0xFF, builderUpdater._hwa._writtenBytes.at(i));
        }
    }
}

#endif
#endif