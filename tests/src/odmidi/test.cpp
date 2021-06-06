#ifdef USE_UART

#include "unity/Framework.h"
#include "board/Board.h"
#include "board/common/comm/USBMIDIOverSerial/USBMIDIOverSerial.h"
#include "core/src/general/RingBuffer.h"

#define TEST_MIDI_CHANNEL 0
#define BUFFER_SIZE       50

namespace
{
    core::RingBuffer<uint8_t, BUFFER_SIZE> buffer;
}    // namespace

namespace Board
{
    namespace UART
    {
        bool read(uint8_t channel, uint8_t& data)
        {
            return buffer.remove(data);
        }

        bool write(uint8_t channel, uint8_t data)
        {
            TEST_ASSERT(buffer.insert(data) == true);
            return true;
        }
    }    // namespace UART
}    // namespace Board

TEST_CASE(MIDI)
{
    MIDI::USBMIDIpacket_t                  sending;
    MIDI::USBMIDIpacket_t                  receiving;
    Board::USBMIDIOverSerial::packetType_t receivedPacketType;

    sending.Event = static_cast<uint8_t>(MIDI::messageType_t::noteOn);
    sending.Data1 = 0x10;
    sending.Data2 = 0x20;
    sending.Data3 = 0x30;

    TEST_ASSERT(Board::USBMIDIOverSerial::write(TEST_MIDI_CHANNEL, sending, Board::USBMIDIOverSerial::packetType_t::midi) == true);
    TEST_ASSERT(Board::USBMIDIOverSerial::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType) == true);

    TEST_ASSERT(receivedPacketType == Board::USBMIDIOverSerial::packetType_t::midi);

    TEST_ASSERT(receiving.Event == static_cast<uint8_t>(MIDI::messageType_t::noteOn));
    TEST_ASSERT(receiving.Data1 == 0x10);
    TEST_ASSERT(receiving.Data2 == 0x20);
    TEST_ASSERT(receiving.Data3 == 0x30);

    //nothing left in the buffer - read should return false
    TEST_ASSERT(Board::USBMIDIOverSerial::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType) == false);
}

TEST_CASE(InternalCMD)
{
    MIDI::USBMIDIpacket_t                  sending;
    MIDI::USBMIDIpacket_t                  receiving;
    Board::USBMIDIOverSerial::packetType_t receivedPacketType;

    sending.Event = 0x00;
    sending.Data1 = 0x10;
    sending.Data2 = 0x20;
    sending.Data3 = 0x30;

    TEST_ASSERT(Board::USBMIDIOverSerial::write(TEST_MIDI_CHANNEL, sending, Board::USBMIDIOverSerial::packetType_t::internal) == true);
    TEST_ASSERT(Board::USBMIDIOverSerial::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType) == true);

    TEST_ASSERT(receivedPacketType == Board::USBMIDIOverSerial::packetType_t::internal);

    //nothing left in the buffer - read should return false
    TEST_ASSERT(Board::USBMIDIOverSerial::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType) == false);
}

#endif
