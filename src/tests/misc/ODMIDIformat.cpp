#include <gtest/gtest.h>
#include "board/Board.h"
#include "common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"
#include "core/src/general/RingBuffer.h"

#define TEST_MIDI_CHANNEL 0
#define BUFFER_SIZE 50

core::RingBuffer<uint8_t, BUFFER_SIZE> buffer;

namespace testing
{
    auto rebootType = Board::rebootType_t::rebootApp;
}

namespace Board
{
    namespace io
    {
        void ledFlashStartup(bool fwUpdated)
        {
        }
    }    // namespace io

    void reboot(Board::rebootType_t type)
    {
        testing::rebootType = type;
    }

    namespace UART
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
    }    // namespace UART
}    // namespace Board

TEST(ODMIDIformat, MIDI)
{
    MIDI::USBMIDIpacket_t            sending;
    MIDI::USBMIDIpacket_t            receiving;
    OpenDeckMIDIformat::packetType_t receivedPacketType;

    sending.Event = static_cast<uint8_t>(MIDI::messageType_t::noteOn);
    sending.Data1 = 0x10;
    sending.Data2 = 0x20;
    sending.Data3 = 0x30;

    EXPECT_TRUE(OpenDeckMIDIformat::write(TEST_MIDI_CHANNEL, sending, OpenDeckMIDIformat::packetType_t::midi));
    EXPECT_TRUE(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType));

    EXPECT_EQ(receivedPacketType, OpenDeckMIDIformat::packetType_t::midi);

    EXPECT_EQ(receiving.Event, static_cast<uint8_t>(MIDI::messageType_t::noteOn));
    EXPECT_EQ(receiving.Data1, 0x10);
    EXPECT_EQ(receiving.Data2, 0x20);
    EXPECT_EQ(receiving.Data3, 0x30);

    //nothing left in the buffer - read should return false
    EXPECT_FALSE(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType));
}

TEST(ODMIDIformat, InternalCMD)
{
    MIDI::USBMIDIpacket_t            sending;
    MIDI::USBMIDIpacket_t            receiving;
    OpenDeckMIDIformat::packetType_t receivedPacketType;

    sending.Event = static_cast<uint8_t>(OpenDeckMIDIformat::command_t::btldrReboot);
    sending.Data1 = 0x10;
    sending.Data2 = 0x20;
    sending.Data3 = 0x30;

    //change the state to app reboot first - after reading this should be switched to rebootBtldr
    testing::rebootType = Board::rebootType_t::rebootApp;

    EXPECT_TRUE(OpenDeckMIDIformat::write(TEST_MIDI_CHANNEL, sending, OpenDeckMIDIformat::packetType_t::internalCommand));
    EXPECT_TRUE(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType));

    EXPECT_EQ(receivedPacketType, OpenDeckMIDIformat::packetType_t::internalCommand);

    //in case of internal commands, Event, Data1, Data2 and Data3 are fully ignored - only the
    //specific function is called
    EXPECT_EQ(testing::rebootType, Board::rebootType_t::rebootBtldr);

    //nothing left in the buffer - read should return false
    EXPECT_FALSE(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType));
}

TEST(ODMIDIformat, ManualPacketBuilding)
{
    MIDI::USBMIDIpacket_t            sending;
    MIDI::USBMIDIpacket_t            receiving;
    OpenDeckMIDIformat::packetType_t receivedPacketType;

    sending.Event = static_cast<uint8_t>(OpenDeckMIDIformat::command_t::btldrReboot);
    sending.Data1 = 0x10;
    sending.Data2 = 0x20;
    sending.Data3 = 0x30;

    uint8_t dataXor = sending.Event ^ sending.Data1 ^ sending.Data2 ^ sending.Data3;

    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, static_cast<uint8_t>(OpenDeckMIDIformat::packetType_t::internalCommand)));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Event));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Data1));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Data2));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Data3));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, dataXor));

    EXPECT_TRUE(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType));

    //now send the same packet but with XOR byte being wrong
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, static_cast<uint8_t>(OpenDeckMIDIformat::packetType_t::internalCommand)));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Event));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Data1));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Data2));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Data3));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, 0x00));

    testing::rebootType = Board::rebootType_t::rebootApp;

    //read should return false and btldr reboot function shouldn't be called
    EXPECT_FALSE(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType));
    EXPECT_EQ(testing::rebootType, Board::rebootType_t::rebootApp);

    //try building valid packet, and then start another one in the middle without first one being finished properly
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, static_cast<uint8_t>(OpenDeckMIDIformat::packetType_t::internalCommand)));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Event));
    //now start another one
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, static_cast<uint8_t>(OpenDeckMIDIformat::packetType_t::internalCommand)));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Event));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Data1));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Data2));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, sending.Data3));
    EXPECT_TRUE(Board::UART::write(TEST_MIDI_CHANNEL, dataXor));

    //there are several reads now which should return failure:
    //5 more failed reads until wrong XOR packet is cleared out of memory
    //2 more failed reads for first two bytes
    for (int i = 0; i < 7; i++)
        EXPECT_FALSE(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType));

    //next read should return success
    EXPECT_TRUE(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType));
    EXPECT_EQ(receivedPacketType, OpenDeckMIDIformat::packetType_t::internalCommand);
    testing::rebootType = Board::rebootType_t::rebootBtldr;
}