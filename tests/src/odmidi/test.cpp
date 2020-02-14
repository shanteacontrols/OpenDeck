#ifdef UART_INTERFACES
#if UART_INTERFACES > 0

#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "board/Board.h"
#include "common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"
#include "core/src/general/RingBuffer.h"

#define TEST_MIDI_CHANNEL 0
#define BUFFER_SIZE       50

namespace
{
    auto                                   rebootType = Board::rebootType_t::rebootApp;
    core::RingBuffer<uint8_t, BUFFER_SIZE> buffer;
}    // namespace

namespace Board
{
    namespace io
    {
        void ledFlashStartup(bool fwUpdated)
        {
        }

        void writeLEDstate(uint8_t ledID, bool state)
        {
        }
    }    // namespace io

    void reboot(Board::rebootType_t type)
    {
        rebootType = type;
    }

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
    MIDI::USBMIDIpacket_t            sending;
    MIDI::USBMIDIpacket_t            receiving;
    OpenDeckMIDIformat::packetType_t receivedPacketType;

    sending.Event = static_cast<uint8_t>(MIDI::messageType_t::noteOn);
    sending.Data1 = 0x10;
    sending.Data2 = 0x20;
    sending.Data3 = 0x30;

    TEST_ASSERT(OpenDeckMIDIformat::write(TEST_MIDI_CHANNEL, sending, OpenDeckMIDIformat::packetType_t::midi) == true);
    TEST_ASSERT(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType) == true);

    TEST_ASSERT(receivedPacketType == OpenDeckMIDIformat::packetType_t::midi);

    TEST_ASSERT(receiving.Event == static_cast<uint8_t>(MIDI::messageType_t::noteOn));
    TEST_ASSERT(receiving.Data1 == 0x10);
    TEST_ASSERT(receiving.Data2 == 0x20);
    TEST_ASSERT(receiving.Data3 == 0x30);

    //nothing left in the buffer - read should return false
    TEST_ASSERT(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType) == false);
}

TEST_CASE(InternalCMD)
{
    MIDI::USBMIDIpacket_t            sending;
    MIDI::USBMIDIpacket_t            receiving;
    OpenDeckMIDIformat::packetType_t receivedPacketType;

    sending.Event = static_cast<uint8_t>(OpenDeckMIDIformat::command_t::btldrReboot);
    sending.Data1 = 0x10;
    sending.Data2 = 0x20;
    sending.Data3 = 0x30;

    //change the state to app reboot first - after reading this should be switched to rebootBtldr
    rebootType = Board::rebootType_t::rebootApp;

    TEST_ASSERT(OpenDeckMIDIformat::write(TEST_MIDI_CHANNEL, sending, OpenDeckMIDIformat::packetType_t::internalCommand) == true);
    TEST_ASSERT(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType) == true);

    TEST_ASSERT(receivedPacketType == OpenDeckMIDIformat::packetType_t::internalCommand);

    //in case of internal commands, Event, Data1, Data2 and Data3 are fully ignored - only the
    //specific function is called
    TEST_ASSERT(rebootType == Board::rebootType_t::rebootBtldr);

    //nothing left in the buffer - read should return false
    TEST_ASSERT(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType) == false);
}

TEST_CASE(ManualPacketBuilding)
{
    MIDI::USBMIDIpacket_t            sending;
    MIDI::USBMIDIpacket_t            receiving;
    OpenDeckMIDIformat::packetType_t receivedPacketType;

    sending.Event = static_cast<uint8_t>(OpenDeckMIDIformat::command_t::btldrReboot);
    sending.Data1 = 0x10;
    sending.Data2 = 0x20;
    sending.Data3 = 0x30;

    uint8_t dataXor = sending.Event ^ sending.Data1 ^ sending.Data2 ^ sending.Data3;

    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, static_cast<uint8_t>(OpenDeckMIDIformat::packetType_t::internalCommand)));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Event));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Data1));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Data2));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Data3));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, dataXor));

    TEST_ASSERT(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType) == true);

    //now send the same packet but with XOR byte being wrong
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, static_cast<uint8_t>(OpenDeckMIDIformat::packetType_t::internalCommand)));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Event));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Data1));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Data2));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Data3));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, 0x00));

    rebootType = Board::rebootType_t::rebootApp;

    //read should return false and btldr reboot function shouldn't be called
    TEST_ASSERT(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType) == false);
    TEST_ASSERT(rebootType == Board::rebootType_t::rebootApp);

    //try building valid packet, and then start another one in the middle without first one being finished properly
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, static_cast<uint8_t>(OpenDeckMIDIformat::packetType_t::internalCommand)));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Event));
    //now start another one
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, static_cast<uint8_t>(OpenDeckMIDIformat::packetType_t::internalCommand)));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Event));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Data1));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Data2));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, sending.Data3));
    TEST_ASSERT(true == Board::UART::write(TEST_MIDI_CHANNEL, dataXor));

    //there are several reads now which should return failure:
    //5 more failed reads until wrong XOR packet is cleared out of memory
    //2 more failed reads for first two bytes
    for (int i = 0; i < 7; i++)
        TEST_ASSERT(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType) == false);

    //next read should return success
    TEST_ASSERT(OpenDeckMIDIformat::read(TEST_MIDI_CHANNEL, receiving, receivedPacketType) == true);
    TEST_ASSERT(receivedPacketType == OpenDeckMIDIformat::packetType_t::internalCommand);
    rebootType = Board::rebootType_t::rebootBtldr;
}

#endif
#endif
