#ifdef PROJECT_TARGET_USB_OVER_SERIAL

#include "framework/Framework.h"
#include "board/Board.h"
#include "core/util/RingBuffer.h"
#include "protocol/midi/MIDI.h"

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

    dataBufSend.push_back(static_cast<uint8_t>(MIDI::messageType_t::NOTE_ON));
    dataBufSend.push_back(0x10);
    dataBufSend.push_back(0x7D);
    dataBufSend.push_back(0x30);

    usbOverSerial::USBWritePacket sending(usbOverSerial::packetType_t::MIDI, &dataBufSend[0], dataBufSend.size(), dataBufSend.size());
    usbOverSerial::USBReadPacket  receiving(&dataBufRecv[0], dataBufRecv.size());

    ASSERT_TRUE(usbOverSerial::write(TEST_MIDI_CHANNEL, sending));
    ASSERT_TRUE(usbOverSerial::read(TEST_MIDI_CHANNEL, receiving));

    ASSERT_TRUE(receiving.type() == usbOverSerial::packetType_t::MIDI);

    ASSERT_EQ(static_cast<uint8_t>(MIDI::messageType_t::NOTE_ON), receiving[0]);
    ASSERT_EQ(0x10, receiving[1]);
    ASSERT_EQ(0x7D, receiving[2]);
    ASSERT_EQ(0x30, receiving[3]);

    // packet is done - read should return true and not cause any changes in the data until
    //.reset() is called
    ASSERT_TRUE(usbOverSerial::read(TEST_MIDI_CHANNEL, receiving));
    ASSERT_EQ(static_cast<uint8_t>(MIDI::messageType_t::NOTE_ON), receiving[0]);
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

    ASSERT_TRUE(usbOverSerial::write(TEST_MIDI_CHANNEL, sending));
    ASSERT_TRUE(usbOverSerial::read(TEST_MIDI_CHANNEL, receiving));

    ASSERT_EQ(receiving.type(), usbOverSerial::packetType_t::MIDI);
    ASSERT_EQ(static_cast<uint8_t>(MIDI::messageType_t::NOTE_ON), receiving[0]);
    ASSERT_EQ(0x10, receiving[1]);
    ASSERT_EQ(0x7D, receiving[2]);
    ASSERT_EQ(0x30, receiving[3]);

    receiving.reset();
    buffer.reset();

    // actual case: send boundary, command type and then restart the packet again
    ASSERT_TRUE(buffer.insert(0x7E));
    ASSERT_TRUE(buffer.insert(static_cast<uint8_t>(usbOverSerial::packetType_t::MIDI)));
    ASSERT_TRUE(buffer.insert(0x7E));
    ASSERT_TRUE(buffer.insert(static_cast<uint8_t>(usbOverSerial::packetType_t::MIDI)));
    ASSERT_TRUE(buffer.insert(0x01));
    ASSERT_TRUE(buffer.insert(0x01));

    ASSERT_TRUE(usbOverSerial::read(TEST_MIDI_CHANNEL, receiving));
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

    usbOverSerial::USBWritePacket sending(usbOverSerial::packetType_t::INTERNAL, &dataBufSend[0], dataBufSend.size(), dataBufSend.size());
    usbOverSerial::USBReadPacket  receiving(&dataBufRecv[0], dataBufRecv.size());

    ASSERT_TRUE(usbOverSerial::write(TEST_MIDI_CHANNEL, sending));
    ASSERT_TRUE(board::usbOverSerial::read(TEST_MIDI_CHANNEL, receiving));

    ASSERT_EQ(receiving.type(), usbOverSerial::packetType_t::INTERNAL);

    ASSERT_EQ(0x7D, receiving[0]);
    ASSERT_EQ(0x7E, receiving[1]);
    ASSERT_EQ(0x7E, receiving[2]);
    ASSERT_EQ(0x30, receiving[3]);

    // packet is done - read should return true and not cause any changes in the data until
    //.reset() is called
    ASSERT_TRUE(usbOverSerial::read(TEST_MIDI_CHANNEL, receiving));
    ASSERT_EQ(0x7D, receiving[0]);
    ASSERT_EQ(0x7E, receiving[1]);
    ASSERT_EQ(0x7E, receiving[2]);
    ASSERT_EQ(0x30, receiving[3]);
}

TEST_F(USBOverSerialTest, LargePacket)
{
    using namespace board;

    constexpr size_t                MAX_SIZE   = 5;
    constexpr size_t                TOTAL_SIZE = 13;
    std::array<uint8_t, TOTAL_SIZE> dataBufRecv;
    std::vector<uint8_t>            dataBufSend;

    for (size_t i = 0; i < TOTAL_SIZE; i++)
    {
        dataBufSend.push_back(i);
    }

    usbOverSerial::USBWritePacket sending(usbOverSerial::packetType_t::CDC, &dataBufSend[0], dataBufSend.size(), MAX_SIZE);
    usbOverSerial::USBReadPacket  receiving(&dataBufRecv[0], MAX_SIZE);

    ASSERT_TRUE(usbOverSerial::write(TEST_MIDI_CHANNEL, sending));
    ASSERT_TRUE(usbOverSerial::read(TEST_MIDI_CHANNEL, receiving));
    ASSERT_EQ(receiving.type(), usbOverSerial::packetType_t::CDC);
    ASSERT_EQ(MAX_SIZE, receiving.size());

    for (size_t i = 0; i < MAX_SIZE; i++)
    {
        ASSERT_EQ(dataBufSend[i], receiving[i]);
    }

    receiving.reset();

    // second part
    ASSERT_TRUE(usbOverSerial::read(TEST_MIDI_CHANNEL, receiving));
    ASSERT_EQ(receiving.type(), usbOverSerial::packetType_t::CDC);
    ASSERT_EQ(MAX_SIZE, receiving.size());

    for (size_t i = 0; i < MAX_SIZE; i++)
    {
        ASSERT_EQ(dataBufSend[i + MAX_SIZE], receiving[i]);
    }

    receiving.reset();

    // third part
    ASSERT_TRUE(usbOverSerial::read(TEST_MIDI_CHANNEL, receiving));
    ASSERT_EQ(receiving.type(), usbOverSerial::packetType_t::CDC);
    ASSERT_EQ(TOTAL_SIZE % MAX_SIZE, receiving.size());

    for (size_t i = 0; i < TOTAL_SIZE % MAX_SIZE; i++)
    {
        ASSERT_EQ(dataBufSend[i + (MAX_SIZE * 2)], receiving[i]);
    }

    receiving.reset();
    ASSERT_FALSE(usbOverSerial::read(TEST_MIDI_CHANNEL, receiving));
}

TEST_F(USBOverSerialTest, CDC)
{
    using namespace board;

    std::array<uint8_t, 5> dataBufRecv;
    std::vector<uint8_t>   dataBufSend;

    // enttec widget api packet - get widget params
    dataBufSend.push_back(0x7E);    // start
    dataBufSend.push_back(0x0A);    // label 10
    dataBufSend.push_back(0x00);    // length lsb
    dataBufSend.push_back(0x00);    // length msb
    dataBufSend.push_back(0xE7);    // end

    usbOverSerial::USBWritePacket sending(usbOverSerial::packetType_t::CDC, &dataBufSend[0], dataBufSend.size(), dataBufSend.size());
    usbOverSerial::USBReadPacket  receiving(&dataBufRecv[0], dataBufRecv.size());

    ASSERT_TRUE(usbOverSerial::write(TEST_MIDI_CHANNEL, sending));
    ASSERT_TRUE(usbOverSerial::read(TEST_MIDI_CHANNEL, receiving));
    ASSERT_EQ(receiving.type(), usbOverSerial::packetType_t::CDC);

    ASSERT_EQ(0x7E, receiving[0]);
    ASSERT_EQ(0x0A, receiving[1]);
    ASSERT_EQ(0x00, receiving[2]);
    ASSERT_EQ(0x00, receiving[3]);
    ASSERT_EQ(0xE7, receiving[4]);
}

#endif
