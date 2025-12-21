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

#ifdef PROJECT_TARGET_USB_OVER_SERIAL

#include "tests/common.h"
#include "application/protocol/midi/midi.h"
#include "board/board.h"

#include "core/util/ring_buffer.h"

using namespace protocol;

namespace
{
    static constexpr size_t                      BUFFER_SIZE       = 64;
    static constexpr size_t                      TEST_MIDI_CHANNEL = 0;
    core::util::RingBuffer<uint8_t, BUFFER_SIZE> buffer;

    class USBOverSerialTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            buffer.reset();
        }
    };
}    // namespace

namespace board::uart
{
    bool read(uint8_t channel, uint8_t& data)
    {
        return buffer.remove(data);
    }

    bool write(uint8_t channel, uint8_t data)
    {
        EXPECT_TRUE(buffer.insert(data));
        return true;
    }
}    // namespace board::uart

TEST_F(USBOverSerialTest, MIDIData)
{
    using namespace board;

    std::array<uint8_t, 4> dataBufRecv;
    std::vector<uint8_t>   dataBufSend;

    dataBufSend.push_back(static_cast<uint8_t>(midi::messageType_t::NOTE_ON));
    dataBufSend.push_back(0x10);
    dataBufSend.push_back(0x7D);
    dataBufSend.push_back(0x30);

    usb_over_serial::UsbWritePacket sending(usb_over_serial::packetType_t::MIDI, &dataBufSend[0], dataBufSend.size(), dataBufSend.size());
    usb_over_serial::UsbReadPacket  receiving(&dataBufRecv[0], dataBufRecv.size());

    ASSERT_TRUE(usb_over_serial::write(TEST_MIDI_CHANNEL, sending));
    ASSERT_TRUE(usb_over_serial::read(TEST_MIDI_CHANNEL, receiving));

    ASSERT_TRUE(receiving.type() == usb_over_serial::packetType_t::MIDI);

    ASSERT_EQ(static_cast<uint8_t>(midi::messageType_t::NOTE_ON), receiving[0]);
    ASSERT_EQ(0x10, receiving[1]);
    ASSERT_EQ(0x7D, receiving[2]);
    ASSERT_EQ(0x30, receiving[3]);

    // packet is done - read should return true and not cause any changes in the data until
    //.reset() is called
    ASSERT_TRUE(usb_over_serial::read(TEST_MIDI_CHANNEL, receiving));
    ASSERT_EQ(static_cast<uint8_t>(midi::messageType_t::NOTE_ON), receiving[0]);
    ASSERT_EQ(0x10, receiving[1]);
    ASSERT_EQ(0x7D, receiving[2]);
    ASSERT_EQ(0x30, receiving[3]);

    // now do the same, but this time add some bytes of junk before writing the actual packet
    // verify that this doesn't cause any issues and that the packet is read again correctly
    ASSERT_TRUE(buffer.insert(0x7E));
    ASSERT_TRUE(buffer.insert(0x7D));
    ASSERT_TRUE(buffer.insert(0x7D));
    ASSERT_TRUE(buffer.insert(0x7D));
    ASSERT_TRUE(buffer.insert(0x7D));
    ASSERT_TRUE(buffer.insert(0x7E));

    receiving.reset();

    ASSERT_TRUE(usb_over_serial::write(TEST_MIDI_CHANNEL, sending));
    ASSERT_TRUE(usb_over_serial::read(TEST_MIDI_CHANNEL, receiving));

    ASSERT_EQ(receiving.type(), usb_over_serial::packetType_t::MIDI);
    ASSERT_EQ(static_cast<uint8_t>(midi::messageType_t::NOTE_ON), receiving[0]);
    ASSERT_EQ(0x10, receiving[1]);
    ASSERT_EQ(0x7D, receiving[2]);
    ASSERT_EQ(0x30, receiving[3]);

    receiving.reset();
    buffer.reset();

    // actual case: send boundary, command type and then restart the packet again
    ASSERT_TRUE(buffer.insert(0x7E));
    ASSERT_TRUE(buffer.insert(static_cast<uint8_t>(usb_over_serial::packetType_t::MIDI)));
    ASSERT_TRUE(buffer.insert(0x7E));
    ASSERT_TRUE(buffer.insert(static_cast<uint8_t>(usb_over_serial::packetType_t::MIDI)));
    ASSERT_TRUE(buffer.insert(0x01));
    ASSERT_TRUE(buffer.insert(0x01));

    ASSERT_TRUE(usb_over_serial::read(TEST_MIDI_CHANNEL, receiving));
}

TEST_F(USBOverSerialTest, InternalCMD)
{
    using namespace board;

    std::array<uint8_t, 4> dataBufRecv;
    std::vector<uint8_t>   dataBufSend;

    dataBufSend.push_back(0x7D);
    dataBufSend.push_back(0x7E);
    dataBufSend.push_back(0x7E);
    dataBufSend.push_back(0x30);

    usb_over_serial::UsbWritePacket sending(usb_over_serial::packetType_t::INTERNAL, &dataBufSend[0], dataBufSend.size(), dataBufSend.size());
    usb_over_serial::UsbReadPacket  receiving(&dataBufRecv[0], dataBufRecv.size());

    ASSERT_TRUE(usb_over_serial::write(TEST_MIDI_CHANNEL, sending));
    ASSERT_TRUE(board::usb_over_serial::read(TEST_MIDI_CHANNEL, receiving));

    ASSERT_EQ(receiving.type(), usb_over_serial::packetType_t::INTERNAL);

    ASSERT_EQ(0x7D, receiving[0]);
    ASSERT_EQ(0x7E, receiving[1]);
    ASSERT_EQ(0x7E, receiving[2]);
    ASSERT_EQ(0x30, receiving[3]);

    // packet is done - read should return true and not cause any changes in the data until
    //.reset() is called
    ASSERT_TRUE(usb_over_serial::read(TEST_MIDI_CHANNEL, receiving));
    ASSERT_EQ(0x7D, receiving[0]);
    ASSERT_EQ(0x7E, receiving[1]);
    ASSERT_EQ(0x7E, receiving[2]);
    ASSERT_EQ(0x30, receiving[3]);
}

#endif
