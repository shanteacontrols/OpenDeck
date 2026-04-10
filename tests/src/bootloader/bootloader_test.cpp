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

#ifdef PROJECT_MCU_SUPPORT_BOOTLOADER

#include "tests/common.h"
#include "bootloader/sysex_parser/sysex_parser.h"
#include "bootloader/updater/builder.h"
#include "sysexgen.h"

#include <vector>
#include <string>

using namespace protocol;

namespace
{
    sysex_parser::SysExParser sysExParser;
    updater::Builder          builderUpdater;

    std::vector<midi::UsbPacket> rawSysExToUSBPackets(const std::vector<uint8_t>& raw)
    {
        std::vector<midi::UsbPacket> packets = {};
        std::vector<uint8_t>         message = {};

        auto appendPacket = [&](uint8_t cin, uint8_t data1, uint8_t data2 = 0, uint8_t data3 = 0)
        {
            midi::UsbPacket packet       = {};
            packet.data[midi::USB_EVENT] = cin;
            packet.data[midi::USB_DATA1] = data1;
            packet.data[midi::USB_DATA2] = data2;
            packet.data[midi::USB_DATA3] = data3;
            packets.push_back(packet);
        };

        auto flushMessage = [&]()
        {
            size_t index = 0;

            while (index < message.size())
            {
                size_t remaining = message.size() - index;

                if (remaining > 3)
                {
                    appendPacket(0x04, message[index], message[index + 1], message[index + 2]);
                    index += 3;
                }
                else if (remaining == 3)
                {
                    appendPacket(0x07, message[index], message[index + 1], message[index + 2]);
                    index += 3;
                }
                else if (remaining == 2)
                {
                    appendPacket(0x06, message[index], message[index + 1]);
                    index += 2;
                }
                else
                {
                    appendPacket(0x05, message[index]);
                    index += 1;
                }
            }

            message.clear();
        };

        for (size_t i = 0; i < raw.size(); i++)
        {
            message.push_back(raw.at(i));

            if (raw.at(i) == 0xF7)
            {
                flushMessage();
            }
        }

        return packets;
    }
}    // namespace

TEST(Bootloader, FwUpdate)
{
    const std::string    firmwareString = "OpenDeck bootloader test fixture payload 1234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::vector<uint8_t> binaryVector(firmwareString.begin(), firmwareString.end());
    auto                 sysExVector = sysexgen::generate(binaryVector, PROJECT_TARGET_UID);
    auto                 packets     = rawSysExToUSBPackets(sysExVector);

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
            ADD_FAILURE() << "Difference on byte " << i;
            break;
        }
    }

    if (binaryVector.size() != builderUpdater._hwa._writtenBytes.size())
    {
        // now verify padding if present
        for (size_t i = binaryVector.size(); i < builderUpdater._hwa._writtenBytes.size(); i++)
        {
            ASSERT_EQ(0xFF, builderUpdater._hwa._writtenBytes.at(i));
        }
    }
}

#endif
