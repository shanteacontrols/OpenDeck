#ifdef ENCODERS_SUPPORTED

#include "unity/Framework.h"
#include "io/encoders/Encoders.h"
#include "io/common/CInfo.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"
#include "database/Database.h"
#include "stubs/database/DB_ReadWrite.h"

namespace
{
    class HWAMIDI : public MIDI::HWA
    {
        public:
        HWAMIDI() = default;

        bool init(MIDI::interface_t interface) override
        {
            return true;
        }

        bool deInit(MIDI::interface_t interface) override
        {
            return true;
        }

        bool dinRead(uint8_t& data) override
        {
            return false;
        }

        bool dinWrite(uint8_t data) override
        {
            return false;
        }

        bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            return false;
        }

        bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            midiPacket.push_back(USBMIDIpacket);
            return true;
        }

        std::vector<MIDI::USBMIDIpacket_t> midiPacket;
    } hwaMIDI;

    class HWAEncoders : public IO::Encoders::HWA
    {
        public:
        HWAEncoders()
        {}

        bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
        {
            numberOfReadings = 1;
            states           = 0;

            if (encoderPosition[index] == IO::Encoders::position_t::ccw)
            {
                states = stateArray[stateCounter[index]];

                if (++stateCounter[index] >= 4)
                    stateCounter[index] = 0;
            }
            else if (encoderPosition[index] == IO::Encoders::position_t::cw)
            {
                if (--stateCounter[index] < 0)
                    stateCounter[index] = 3;

                states = stateArray[stateCounter[index]];
            }

            if (states == lastState[index])
                return state(index, numberOfReadings, states);

            lastState[index] = states;

            return true;
        }

        void setEncoderState(size_t index, IO::Encoders::position_t position)
        {
            encoderPosition[index] = position;
        }

        IO::Encoders::position_t encoderPosition[MAX_NUMBER_OF_ENCODERS];

        int8_t  stateCounter[MAX_NUMBER_OF_ENCODERS] = {};
        uint8_t lastState[MAX_NUMBER_OF_ENCODERS]    = {};

        const uint8_t stateArray[4] = {
            0b01,
            0b11,
            0b10,
            0b00
        };
    } hwaEncoders;

    class EncodersFilter : public IO::Encoders::Filter
    {
        public:
        bool isFiltered(size_t                    index,
                        IO::Encoders::position_t  position,
                        IO::Encoders::position_t& filteredPosition,
                        uint32_t                  sampleTakenTime) override
        {
            return true;
        }

        void reset(size_t index) override
        {
        }

        uint32_t lastMovementTime(size_t index) override
        {
            return 0;
        }
    } encodersFilter;

    DBstorageMock dbStorageMock;
    Database      database = Database(dbStorageMock, true);
    MIDI          midi(hwaMIDI);
    ComponentInfo cInfo;

    class HWAU8X8 : public IO::U8X8::HWAI2C
    {
        public:
        HWAU8X8() {}

        bool init() override
        {
            return true;
        }

        bool deInit() override
        {
            return true;
        }

        bool write(uint8_t address, uint8_t* buffer, size_t size) override
        {
            return true;
        }
    } hwaU8X8;

    IO::U8X8     u8x8(hwaU8X8);
    IO::Display  display(u8x8, database);
    IO::Encoders encoders = IO::Encoders(hwaEncoders, encodersFilter, 1, database, midi, display, cInfo);
}    // namespace

TEST_SETUP()
{
    // init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);
    encoders.init();
    midi.init(MIDI::interface_t::usb);
}

TEST_CASE(StateDecoding)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
    {
        //enable all encoders
        TEST_ASSERT(database.update(Database::Section::encoder_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::encoder_t::invert, i, 0) == true);

        //set type of message to Encoders::type_t::t7Fh01h
        TEST_ASSERT(database.update(Database::Section::encoder_t::mode, i, static_cast<int32_t>(Encoders::type_t::t7Fh01h)) == true);

        //set single pulse per step
        TEST_ASSERT(database.update(Database::Section::encoder_t::pulsesPerStep, i, 1) == true);
    }

    uint8_t state;

    //test expected permutations for ccw and cw directions

    //clockwise: 00, 10, 11, 01

    encoders.init();

    state = 0b00;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    state = 0b10;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    state = 0b11;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    state = 0b01;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    encoders.init();

    state = 0b10;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    state = 0b11;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    state = 0b01;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    state = 0b00;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    encoders.init();

    state = 0b11;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    state = 0b01;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    state = 0b00;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    state = 0b10;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    encoders.init();

    state = 0b01;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    state = 0b00;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    state = 0b10;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    state = 0b11;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    //counter-clockwise: 00, 01, 11, 10
    encoders.init();

    state = 0b00;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    state = 0b01;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);

    state = 0b11;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);

    state = 0b10;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);

    encoders.init();

    state = 0b01;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    state = 0b11;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);

    state = 0b10;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);

    state = 0b00;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);

    encoders.init();

    state = 0b11;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    state = 0b10;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);

    state = 0b00;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);

    state = 0b01;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);

    encoders.init();

    state = 0b10;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    state = 0b00;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);

    state = 0b01;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);

    state = 0b11;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);

    //this time configure 4 pulses per step
    for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
        TEST_ASSERT(database.update(Database::Section::encoder_t::pulsesPerStep, i, 4) == true);

    encoders.init();

    //clockwise: 00, 10, 11, 01

    //initial state doesn't count as pulse, 4 more needed
    state = 0b00;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    //1
    state = 0b10;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    //2
    state = 0b11;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    //3
    state = 0b01;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    //4
    //pulse should be registered
    state = 0b00;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    //1
    state = 0b10;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    //2
    state = 0b11;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    //3
    state = 0b01;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    //4
    state = 0b00;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::cw);

    //now move to opposite direction
    //don't start from 0b00 state again
    //counter-clockwise: 01, 11, 10, 00

    state = 0b01;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    state = 0b11;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    state = 0b10;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::stopped);

    state = 0b00;
    TEST_ASSERT(encoders.read(0, state) == Encoders::position_t::ccw);
}

#endif