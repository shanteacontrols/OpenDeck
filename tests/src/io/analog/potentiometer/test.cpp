#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "io/analog/Analog.h"
#include "io/leds/LEDs.h"
#include "io/common/CInfo.h"
#include "database/Database.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include "stubs/database/DB_ReadWrite.h"
#include <vector>

#ifdef ANALOG_SUPPORTED

namespace
{
    uint32_t                           messageCounter = 0;
    std::vector<MIDI::USBMIDIpacket_t> midiPacket;

    void resetReceived()
    {
        midiPacket.clear();
        messageCounter = 0;
    }

    bool midiDataHandler(MIDI::USBMIDIpacket_t& USBMIDIpacket)
    {
        midiPacket.push_back(USBMIDIpacket);
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

    class HWALEDs : public IO::LEDs::HWA
    {
        public:
        HWALEDs() {}

        void setState(size_t index, bool state) override
        {
        }

        size_t rgbSingleComponentIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
        {
            return 0;
        }

        size_t rgbIndex(size_t singleLEDindex) override
        {
            return 0;
        }

        void setFadeSpeed(size_t transitionSpeed) override
        {
        }
    } ledsHWA;

    class HWAAnalog : public IO::Analog::HWA
    {
        public:
        HWAAnalog() {}

        uint16_t state(size_t index) override
        {
            return adcReturnValue;
        }

        uint32_t adcReturnValue;
    } hwaAnalog;

    DBstorageMock dbStorageMock;
    Database      database = Database(dbHandlers, dbStorageMock, true);
    MIDI          midi;
    ComponentInfo cInfo;

    IO::LEDs leds(ledsHWA, database);

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

        bool write(uint8_t address, uint8_t* data, size_t size) override
        {
            return true;
        }
    } hwaU8X8;

    class AnalogFilterStub : public IO::Analog::Filter
    {
        public:
        AnalogFilterStub() {}

        bool isFiltered(size_t index, uint16_t value, uint16_t& filteredValue) override
        {
            filteredValue = value;
            return true;
        }

        void reset(size_t index) override
        {
        }
    } analogFilter;

    IO::U8X8    u8x8(hwaU8X8);
    IO::Display display(u8x8, database);

#ifdef ADC_12_BIT
#define ADC_RESOLUTION IO::Analog::adcType_t::adc12bit
#else
#define ADC_RESOLUTION IO::Analog::adcType_t::adc10bit
#endif

    IO::Analog analog(hwaAnalog, ADC_RESOLUTION, analogFilter, database, midi, leds, display, cInfo);
}    // namespace

TEST_SETUP()
{
    //init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);
    TEST_ASSERT(database.isSignatureValid() == true);
    TEST_ASSERT(database.factoryReset(LESSDB::factoryResetType_t::full) == true);
    midi.handleUSBwrite(midiDataHandler);

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);

    resetReceived();
}

TEST_CASE(CCtest)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::potentiometerControlChange)) == true);

        //set all lower limits to 0
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to MIDI_7_BIT_VALUE_MAX
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, MIDI_7_BIT_VALUE_MAX) == true);

        //midi channel
        TEST_ASSERT(database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    //feed all the values from minimum to maximum
    //expect the following:
    //first value is 0
    //last value is 127
    auto adcConfig = analog.config();

    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    //all received messages should be control change
    for (int i = 0; i < midiPacket.size(); i++)
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint8_t>(MIDI::messageType_t::controlChange), midiPacket.at(i).Event << 4);

    for (int i = 0; i < midiPacket.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;

            if (!i)
            {
                TEST_ASSERT_EQUAL_UINT32(0, midiPacket.at(index).Data3);
            }
            else if (i == (midiPacket.size() - 1))
            {
                TEST_ASSERT_EQUAL_UINT32(127, midiPacket.at(index).Data3);
            }
        }
    }

    //now go backward

    resetReceived();

    for (int i = adcConfig.adcMaxValue; i >= 0; i--)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    //verify that the last value is 0
    for (int i = 0; i < midiPacket.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;

            if (i == (midiPacket.size() - 1))
            {
                TEST_ASSERT_EQUAL_UINT32(0, midiPacket.at(index).Data3);
            }
        }
    }
}

TEST_CASE(PitchBendTest)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with Pitch Bend MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::pitchBend)) == true);

        //set all lower limits to 0
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, MIDI_14_BIT_VALUE_MAX) == true);

        //midi channel
        TEST_ASSERT(database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    auto adcConfig = analog.config();

    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    //number of pitch bend messages can vary depending on ADC resolution and debounce
    //techniques used
    //just verify that number of messages per component is larger than 128 (amount of messages in 7-bit mode)
    TEST_ASSERT(messageCounter >= (MAX_NUMBER_OF_ANALOG * 128));

    //all received messages should be pitch bend
    for (int i = 0; i < midiPacket.size(); i++)
        TEST_ASSERT_EQUAL_UINT32(midiPacket.at(i).Event << 4, static_cast<uint8_t>(MIDI::messageType_t::pitchBend));

    //since numbers in this case are scaled from lower to upper range,
    //verify that each next value is larger from previous
    uint32_t previousPitchBendValue = 0;

    uint32_t receivedMessages = midiPacket.size() / MAX_NUMBER_OF_ANALOG;

    for (int i = 0; i < receivedMessages; i++)
    {
        MIDI::encDec_14bit_t pitchBendValue;

        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i * MAX_NUMBER_OF_ANALOG + j;

            pitchBendValue.low  = midiPacket.at(index).Data2;
            pitchBendValue.high = midiPacket.at(index).Data3;
            pitchBendValue.mergeTo14bit();

            if (i)
            {
                TEST_ASSERT(pitchBendValue.value > previousPitchBendValue);
            }
        }

        previousPitchBendValue = pitchBendValue.value;
    }

    // try to update it again without changing values, nothing should change
    uint32_t previousCount = messageCounter;
    analog.update();
    TEST_ASSERT_EQUAL_UINT32(previousCount, messageCounter);

    //now go backward
    resetReceived();

    for (int i = adcConfig.adcMaxValue; i >= 0; i--)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    //similar to previous case, verify that every next value is smaller than previous

    receivedMessages       = midiPacket.size() / MAX_NUMBER_OF_ANALOG;
    previousPitchBendValue = 16383;

    for (int i = 0; i < receivedMessages; i++)
    {
        MIDI::encDec_14bit_t pitchBendValue;

        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            volatile size_t index = i * MAX_NUMBER_OF_ANALOG + j;

            pitchBendValue.low  = midiPacket.at(index).Data2;
            pitchBendValue.high = midiPacket.at(index).Data3;
            pitchBendValue.mergeTo14bit();

            TEST_ASSERT(pitchBendValue.value < previousPitchBendValue);
        }

        previousPitchBendValue = pitchBendValue.value;
    }

    // try to update it again without changing values, nothing should change
    previousCount = messageCounter;
    analog.update();
    TEST_ASSERT_EQUAL_UINT32(previousCount, messageCounter);
}

TEST_CASE(Inversion)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::potentiometerControlChange)) == true);

        //set all lower limits to 0
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, MIDI_14_BIT_VALUE_MAX) == true);

        //midi channel
        TEST_ASSERT(database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    auto adcConfig           = analog.config();
    hwaAnalog.adcReturnValue = adcConfig.adcMaxValue;
    uint32_t previousValue;

    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    previousValue = 0;

    //verify that every received value is larger than the previous
    //first value should be 0
    //last value should be 127
    for (int i = 0; i < 128; i++)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i * MAX_NUMBER_OF_ANALOG + j;

            if (!i)
                TEST_ASSERT_EQUAL_UINT32(0, midiPacket.at(index).Data3);
            else
                TEST_ASSERT(midiPacket.at(index).Data3 > previousValue);

            if (i == 127)
                TEST_ASSERT_EQUAL_UINT32(127, midiPacket.at(index).Data3);
        }

        previousValue = i;
    }

    //now enable inversion
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 1) == true);
        analog.debounceReset(i);
    }

    resetReceived();

    //feed all the values again
    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    //verify that every received value is smaller than the previous
    //first value should be 127
    //last value should be 0
    for (int i = 0; i < 128; i++)
    {
        size_t index;

        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            index = i * MAX_NUMBER_OF_ANALOG + j;

            if (!i)
                TEST_ASSERT_EQUAL_UINT32(127, midiPacket.at(index).Data3);
            else
                TEST_ASSERT(midiPacket.at(index).Data3 < previousValue);

            if (i == 127)
                TEST_ASSERT_EQUAL_UINT32(0, midiPacket.at(index).Data3);
        }

        previousValue = midiPacket.at(index).Data3;
    }

    resetReceived();

    //funky setup: set lower limit to 127, upper to 0 while inversion is enabled
    //result should be the same as when default setup is used (no inversion/ 0 as lower limit, 127 as upper limit)

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 1) == true);
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 127) == true);
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, 0) == true);
        analog.debounceReset(i);
    }

    //feed all the values again
    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    previousValue = 0;

    //verify that every received value is larger than the previous
    //first value should be 0
    //last value should be 127
    for (int i = 0; i < 128; i++)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i * MAX_NUMBER_OF_ANALOG + j;

            if (!i)
                TEST_ASSERT_EQUAL_UINT32(0, midiPacket.at(index).Data3);
            else
                TEST_ASSERT(midiPacket.at(index).Data3 > previousValue);

            if (i == 127)
                TEST_ASSERT_EQUAL_UINT32(127, midiPacket.at(index).Data3);
        }

        previousValue = i;
    }

    //now disable inversion
    resetReceived();

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);
        analog.debounceReset(i);
    }

    //feed all the values again
    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    previousValue = 0;

    //verify that every received value is smaller than the previous
    //first value should be 127
    //last value should be 0
    for (int i = 0; i < 128; i++)
    {
        size_t index;

        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            index = i * MAX_NUMBER_OF_ANALOG + j;

            if (!i)
                TEST_ASSERT_EQUAL_UINT32(127, midiPacket.at(index).Data3);
            else
                TEST_ASSERT(midiPacket.at(index).Data3 < previousValue);

            if (i == 127)
                TEST_ASSERT_EQUAL_UINT32(0, midiPacket.at(index).Data3);
        }

        previousValue = midiPacket.at(index).Data3;
    }
}

TEST_CASE(Scaling)
{
    using namespace IO;

    const uint32_t scaledLower = 11;
    const uint32_t scaledUpper = 100;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::potentiometerControlChange)) == true);

        //set all lower limits to 0
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to 100
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, scaledUpper) == true);

        //midi channel
        TEST_ASSERT(database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    auto adcConfig           = analog.config();
    hwaAnalog.adcReturnValue = adcConfig.adcMaxValue;
    uint32_t previousValue;

    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    previousValue = 0;

    //first values should be 0
    //last value should match the configured scaled value (scaledUpper)
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT_EQUAL_UINT32(0, midiPacket.at(i).Data3);
        TEST_ASSERT_EQUAL_UINT32(scaledUpper, midiPacket.at(midiPacket.size() - MAX_NUMBER_OF_ANALOG + i).Data3);
    }

    //now scale minimum value as well
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, scaledLower) == true);

    resetReceived();

    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(midiPacket.at(i).Data3 >= scaledLower);
        TEST_ASSERT(midiPacket.at(midiPacket.size() - MAX_NUMBER_OF_ANALOG + i).Data3 <= scaledUpper);
    }

    //now enable inversion
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 1) == true);
        analog.debounceReset(i);
    }

    resetReceived();

    for (int i = 0; i <= adcConfig.adcMaxValue; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(midiPacket.at(i).Data3 >= scaledUpper);
        TEST_ASSERT(midiPacket.at(midiPacket.size() - MAX_NUMBER_OF_ANALOG + i).Data3 <= scaledLower);
    }
}

#endif