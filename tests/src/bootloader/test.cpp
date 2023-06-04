#ifndef PROJECT_TARGET_USB_OVER_SERIAL_HOST
#ifdef PROJECT_MCU_SUPPORT_BOOTLOADER

#include "tests/Common.h"
#include "SysExParser/SysExParser.h"
#include "bootloader/updater/Updater.h"
#include "tests/helpers/MIDI.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <iterator>
#include <string>
#include <cstddef>

// Force the generated flash functions into test namespace to avoid clashes with
// stub functions (since stub MCU is used for tests)
namespace test
{
#include <CoreMCUGenerated.h>
}

namespace
{
    class BTLDRWriter : public Updater::BTLDRWriter
    {
        public:
        uint32_t pageSize(size_t index) override
        {
            return test::core::mcu::flash::pageSize(index);
        }

        void erasePage(size_t index) override
        {
        }

        void fillPage(size_t index, uint32_t address, uint32_t value) override
        {
            writtenBytes.push_back(value >> 0 & static_cast<uint32_t>(0xFF));
            writtenBytes.push_back(value >> 8 & static_cast<uint32_t>(0xFF));
            writtenBytes.push_back(value >> 16 & static_cast<uint32_t>(0xFF));
            writtenBytes.push_back(value >> 24 & static_cast<uint32_t>(0xFF));
        }

        void commitPage(size_t index) override
        {
        }

        void apply() override
        {
            updated = true;
        }

        void onFirmwareUpdateStart() override
        {
        }

        std::vector<uint8_t> writtenBytes = {};
        bool                 updated      = false;
    };

    const std::string fw_build_type_subdir = "release/";
    const std::string FW_UPDATE_FILE_SYSEX = "firmware.syx";
    const std::string FW_UPDATE_FILE_BIN   = "firmware.bin";

    SysExParser _sysExParser;
    BTLDRWriter _btldrWriter;
    Updater     _updater = Updater(_btldrWriter, PROJECT_TARGET_UID);
    MIDIHelper  _helper;
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

    std::vector<uint8_t>               singleSysExMsg = {};
    std::vector<MIDI::usbMIDIPacket_t> packets        = {};

    // Go over the entire SysEx file.
    // Upon reaching the end of single SysEx message, convert it
    // into series of USB MIDI packets.
    for (size_t i = 0; i < sysExVector.size(); i++)
    {
        singleSysExMsg.push_back(sysExVector.at(i));

        if (sysExVector.at(i) == 0xF7)
        {
            auto converted = _helper.rawSysExToUSBPackets(singleSysExMsg);
            packets.insert(std::end(packets), std::begin(converted), std::end(converted));
            singleSysExMsg.clear();
        }
    }

    // Now we have the entire file in form of USB MIDI packets.
    // Parse each message and once parsing passes, feed the parsed data into FW updater.
    for (size_t packet = 0; packet < packets.size(); packet++)
    {
        if (_sysExParser.isValidMessage(packets.at(packet)))
        {
            size_t  dataSize = _sysExParser.dataBytes();
            uint8_t data     = 0;

            if (dataSize)
            {
                for (size_t i = 0; i < dataSize; i++)
                {
                    if (_sysExParser.value(i, data))
                    {
                        _updater.feed(data);
                    }
                }
            }
        }
    }

    // once all data has been fed into updater, firmware update procedure should be complete
    ASSERT_TRUE(_btldrWriter.updated);

    // written content should also match the original binary file from which SysEx file has been created
    // verify only until binaryVector.size() -> writtenBytes vector could be slightly larger due to padding
    for (size_t i = 0; i < binaryVector.size(); i++)
    {
        if (_btldrWriter.writtenBytes.at(i) != binaryVector.at(i))
        {
            LOG(ERROR) << "Difference on byte " << i;
            ASSERT_TRUE(true == false);
        }
    }

    if (binaryVector.size() != _btldrWriter.writtenBytes.size())
    {
        LOG(INFO) << "Expecting padding in firmware file";

        // now verify padding if present
        for (size_t i = binaryVector.size(); i < _btldrWriter.writtenBytes.size(); i++)
        {
            ASSERT_EQ(0xFF, _btldrWriter.writtenBytes.at(i));
        }
    }
}

#endif
#endif