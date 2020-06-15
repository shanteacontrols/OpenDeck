#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "io/encoders/Encoders.h"
#include "io/leds/LEDs.h"
#include "io/common/CInfo.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"
#include "database/Database.h"
#include "stubs/database/DB_ReadWrite.h"

#ifdef ENCODERS_SUPPORTED

namespace
{
    uint8_t controlValue[MAX_NUMBER_OF_ENCODERS];
    uint8_t messageCounter;

    bool midiDataHandler(MIDI::USBMIDIpacket_t& USBMIDIpacket)
    {
        controlValue[messageCounter] = USBMIDIpacket.Data3;
        messageCounter++;
        return true;
    }

    class DBhandlers : public Database::Handlers
    {
        public:
        DBhandlers() {}

        void presetChange(uint8_t preset) override
        {
            if (presetChangeHandler != nullptr)
                presetChangeHandler(preset);
        }

        void factoryResetStart() override
        {
            if (factoryResetStartHandler != nullptr)
                factoryResetStartHandler();
        }

        void factoryResetDone() override
        {
            if (factoryResetDoneHandler != nullptr)
                factoryResetDoneHandler();
        }

        void initialized() override
        {
            if (initHandler != nullptr)
                initHandler();
        }

        //actions which these handlers should take depend on objects making
        //up the entire system to be initialized
        //therefore in interface we are calling these function pointers which
        // are set in application once we have all objects ready
        void (*presetChangeHandler)(uint8_t preset) = nullptr;
        void (*factoryResetStartHandler)()          = nullptr;
        void (*factoryResetDoneHandler)()           = nullptr;
        void (*initHandler)()                       = nullptr;
    } dbHandlers;

    class HWAEncoders : public IO::Encoders::HWA
    {
        public:
        HWAEncoders()
        {}

        uint8_t state(size_t index) override
        {
            uint8_t returnValue = 0;

            if (encoderPosition[index] == IO::Encoders::position_t::ccw)
            {
                returnValue = stateArray[stateCounter[index]];

                if (++stateCounter[index] >= 4)
                    stateCounter[index] = 0;
            }
            else if (encoderPosition[index] == IO::Encoders::position_t::cw)
            {
                if (--stateCounter[index] < 0)
                    stateCounter[index] = 3;

                returnValue = stateArray[stateCounter[index]];
            }

            if (returnValue == lastState[index])
                return state(index);

            lastState[index] = returnValue;
            return returnValue;
        }

        void setEncoderState(uint8_t encoderID, IO::Encoders::position_t position)
        {
            controlValue[encoderID]    = 0;
            encoderPosition[encoderID] = position;
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

    DBstorageMock dbStorageMock;
    Database      database = Database(dbHandlers, dbStorageMock);
    MIDI          midi;
    ComponentInfo cInfo;

    class HWAU8X8 : public IO::U8X8::HWAI2C
    {
        public:
        HWAU8X8() {}

        bool init() override
        {
            return true;
        }

        bool write(uint8_t address, uint8_t* data, size_t size) override
        {
            return true;
        }
    } hwaU8X8;

    IO::U8X8     u8x8(hwaU8X8);
    IO::Display  display(u8x8, database);
    IO::Encoders encoders = IO::Encoders(hwaEncoders, database, midi, display, cInfo);
}    // namespace

TEST_SETUP()
{
    // init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);
    TEST_ASSERT(database.isSignatureValid() == true);
    encoders.init();
    midi.handleUSBwrite(midiDataHandler);
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

TEST_CASE(Debounce)
{
    using namespace IO;

    auto debounceTest = [&](uint8_t pulsesPerStep) {
        //set known state
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
        {
            //enable all encoders
            TEST_ASSERT(database.update(Database::Section::encoder_t::enable, i, 1) == true);

            //disable invert state
            TEST_ASSERT(database.update(Database::Section::encoder_t::invert, i, 0) == true);

            //set type of message to Encoders::type_t::t7Fh01h
            TEST_ASSERT(database.update(Database::Section::encoder_t::mode, i, static_cast<int32_t>(Encoders::type_t::t7Fh01h)) == true);

            //pulses per step
            TEST_ASSERT(database.update(Database::Section::encoder_t::pulsesPerStep, i, pulsesPerStep) == true);

            //midi channel
            TEST_ASSERT(database.update(Database::Section::encoder_t::midiChannel, i, 1) == true);
        }

        auto encValue = [](Encoders::type_t type, Encoders::position_t position) {
            switch (type)
            {
            case Encoders::type_t::t7Fh01h:
                return position == Encoders::position_t::ccw ? 127 : position == Encoders::position_t::cw ? 1 : 0;
                break;

            case Encoders::type_t::t3Fh41h:
                return position == Encoders::position_t::ccw ? 63 : position == Encoders::position_t::cw ? 65 : 0;
                break;

            default:
                return 0;
            }
        };

        auto verifyValue = [&](Encoders::position_t* value) {
            bool success;

            for (int i = 0; i < 4 + 1; i++)
            {
                success = false;
                encoders.update();

                //verify that all received values are correct
                for (int j = 0; j < MAX_NUMBER_OF_ENCODERS; j++)
                {
                    if (controlValue[j] != encValue(Encoders::type_t::t7Fh01h, value[j]))
                    {
                        success = false;
                        break;
                    }
                    else
                    {
                        success = true;
                    }
                }

                if (success)
                    break;
            }

            return success;
        };

        core::timing::detail::rTime_ms = 0;
        Encoders::position_t testValue[MAX_NUMBER_OF_ENCODERS];

        messageCounter = 0;
        encoders.init();

        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
        {
            hwaEncoders.setEncoderState(i, Encoders::position_t::ccw);
            testValue[i] = Encoders::position_t::ccw;
        }

        TEST_ASSERT(verifyValue(testValue) == true);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ENCODERS);

        //reset values
        messageCounter = 0;

        // set new direction
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
        {
            hwaEncoders.setEncoderState(i, Encoders::position_t::cw);
            testValue[i] = Encoders::position_t::cw;
        }

        TEST_ASSERT(verifyValue(testValue) == true);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ENCODERS);

        //test the scenario where the values alternate between last position
        //(right) and different position (left)
        //after four consecutive values debouncer should ignore all futher
        //changes in direction until encoder slows down

#define NUMBER_OF_TEST_VALUES 32

        Encoders::position_t encoderPositionTest1[NUMBER_OF_TEST_VALUES] = {
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw
        };

        core::timing::detail::rTime_ms += ENCODERS_DEBOUNCE_RESET_TIME + 1;

        // all values should be right position
        // call ::update n times (MAX_NUMBER_OF_ENCODERS)
        // each call retrieves encoder position from test array
        // after each call switch to next value from test array
        for (int i = 0; i < NUMBER_OF_TEST_VALUES; i++)
        {
            messageCounter = 0;

            for (int j = 0; j < MAX_NUMBER_OF_ENCODERS; j++)
            {
                hwaEncoders.setEncoderState(j, encoderPositionTest1[i]);
                testValue[j] = Encoders::position_t::cw;
            }

            TEST_ASSERT(verifyValue(testValue) == true);
        }

        //perform the same test again but with time difference
        //returned values should match the values in test array:
        //debouncer should not be initiated
        core::timing::detail::rTime_ms = ENCODERS_DEBOUNCE_RESET_TIME + 1;

        for (int i = 0; i < NUMBER_OF_TEST_VALUES; i++)
        {
            messageCounter = 0;

            for (int j = 0; j < MAX_NUMBER_OF_ENCODERS; j++)
            {
                hwaEncoders.setEncoderState(j, encoderPositionTest1[i]);
                testValue[j] = encoderPositionTest1[i];
            }

            TEST_ASSERT(verifyValue(testValue) == true);
            core::timing::detail::rTime_ms += ENCODERS_DEBOUNCE_RESET_TIME + 1;
        }

        //test the following scenario:
        //1) debouncer becomes initated and starts ignoring the changes in direction
        //2) after n values (16) encoder starts spinning in opposite direction more than
        // DEBOUNCE_COUNT times
        //test whether the debouncer registered the valid change

        Encoders::position_t encoderPositionTest2[NUMBER_OF_TEST_VALUES] = {
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::ccw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::cw,
            Encoders::position_t::ccw,
        };

        core::timing::detail::rTime_ms += ENCODERS_DEBOUNCE_RESET_TIME + 1;

        for (int i = 0; i < 16; i++)
        {
            messageCounter = 0;

            for (int j = 0; j < MAX_NUMBER_OF_ENCODERS; j++)
            {
                hwaEncoders.setEncoderState(j, encoderPositionTest2[i]);
                testValue[j] = Encoders::position_t::cw;
            }

            TEST_ASSERT(verifyValue(testValue) == true);
        }

        for (int i = 16; i < 19; i++)
        {
            //first three values should still be right direction
            messageCounter = 0;

            for (int j = 0; j < MAX_NUMBER_OF_ENCODERS; j++)
            {
                hwaEncoders.setEncoderState(j, encoderPositionTest2[i]);
                testValue[j] = Encoders::position_t::cw;
            }

            TEST_ASSERT(verifyValue(testValue) == true);
        }

        //from now on, debouncer should be initiated and all other values should be left
        for (int i = 19; i < MAX_NUMBER_OF_ENCODERS; i++)
        {
            messageCounter = 0;

            for (int j = 0; j < MAX_NUMBER_OF_ENCODERS; j++)
            {
                hwaEncoders.setEncoderState(j, encoderPositionTest2[i]);
                testValue[j] = Encoders::position_t::ccw;
            }

            TEST_ASSERT(verifyValue(testValue) == true);
        }
    };

    // perform the test for all supported pulses per step values
    debounceTest(4);
    debounceTest(3);
    debounceTest(2);
}

TEST_CASE(Acceleration)
{
    using namespace IO;

    auto accelerationTest = [&](uint8_t pulsesPerStep) {
#define ENCODER_SPEED_CHANGE 3
        //set known state
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
        {
            //enable all encoders
            TEST_ASSERT(database.update(Database::Section::encoder_t::enable, i, 1) == true);

            //disable invert state
            TEST_ASSERT(database.update(Database::Section::encoder_t::invert, i, 0) == true);

            //set type of message to Encoders::type_t::tControlChange
            TEST_ASSERT(database.update(Database::Section::encoder_t::mode, i, static_cast<int32_t>(Encoders::type_t::tControlChange)) == true);

            //enable acceleration
            TEST_ASSERT(database.update(Database::Section::encoder_t::acceleration, i, ENCODER_SPEED_CHANGE) == true);

            //pulses per step
            TEST_ASSERT(database.update(Database::Section::encoder_t::pulsesPerStep, i, pulsesPerStep) == true);

            //midi channel
            TEST_ASSERT(database.update(Database::Section::encoder_t::midiChannel, i, 1) == true);
        }

        encoders.init();

        //all encoders should move in the same direction
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            hwaEncoders.setEncoderState(i, Encoders::position_t::cw);

        core::timing::detail::rTime_ms = 1;

        uint16_t lastValue = 0;

        auto update = [&](uint8_t compareValue) {
            bool success;

            for (int j = 0; j < 4 + 1; j++)
            {
                success = false;
                encoders.update();

                if (compareValue > 127)
                    compareValue = 127;

                if (messageCounter == MAX_NUMBER_OF_ENCODERS)
                {
                    //verify that all received values are correct
                    for (int k = 0; k < MAX_NUMBER_OF_ENCODERS; k++)
                    {
                        if (controlValue[k] != compareValue)
                        {
                            success = false;
                            break;
                        }
                        else
                        {
                            success = true;
                        }
                    }

                    if (success)
                    {
                        lastValue = controlValue[0];
                        break;
                    }
                }
            }

            TEST_ASSERT(success == true);
        };

        //run update several times, each time increasing the run time by 1
        //encoder state is same every time (cw direction)
        //this will cause acceleration to kick in
        //verify the outputed values
        //acceleration works by appending ENCODER_SPEED_CHANGE+previous value to
        //to current encoder midi value
        for (int i = 1; i <= 3; i++)
        {
            messageCounter = 0;
            core::timing::detail::rTime_ms++;
            uint16_t compareValue = lastValue + (ENCODER_SPEED_CHANGE * i);
            update(compareValue);
        }

        //now run update again but with time difference between movements being
        //just enough so that encoder doesn't accelerate
        //in this case, existing values should be increased only by 1
        for (int i = 1; i <= 3; i++)
        {
            messageCounter = 0;
            core::timing::detail::rTime_ms += ENCODERS_SPEED_TIMEOUT;
            uint16_t compareValue = lastValue + 1;
            update(compareValue);
        }
    };

    accelerationTest(4);
    accelerationTest(3);
    accelerationTest(2);
}

#endif