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
    void ledFlashStartup(bool fwUpdated)
    {
    }

    void reboot(Board::rebootType_t type)
    {
        testing::rebootType = type;
    }

    namespace UART
    {
        bool read(uint8_t channel, uint8_t& data)
        {
            EXPECT_TRUE(buffer.remove(data));
            return true;
        }

        bool write(uint8_t channel, uint8_t data)
        {
            EXPECT_TRUE(buffer.insert(data));
            return true;
        }

        uint8_t bytesAvailableRx(uint8_t channel)
        {
            return buffer.count();
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