#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "interface/analog/Analog.h"
#include "interface/digital/output/leds/LEDs.h"
#include "interface/CInfo.h"
#include "database/Database.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include "stubs/database/DB_ReadWrite.h"
#include "board/Board.h"

namespace
{
    uint32_t              messageCounter = 0;
    MIDI::USBMIDIpacket_t midiPacket[MAX_NUMBER_OF_ANALOG];

    void resetReceived()
    {
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        {
            midiPacket[i].Event = 0;
            midiPacket[i].Data1 = 0;
            midiPacket[i].Data2 = 0;
            midiPacket[i].Data3 = 0;
        }

        messageCounter = 0;
    }

    bool midiDataHandler(MIDI::USBMIDIpacket_t& USBMIDIpacket)
    {
        midiPacket[messageCounter].Event = USBMIDIpacket.Event;
        midiPacket[messageCounter].Data1 = USBMIDIpacket.Data1;
        midiPacket[messageCounter].Data2 = USBMIDIpacket.Data2;
        midiPacket[messageCounter].Data3 = USBMIDIpacket.Data3;

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

    DBstorageMock dbStorageMock;
    Database      database = Database(dbHandlers, dbStorageMock);
    MIDI          midi;
    ComponentInfo cInfo;

#ifdef LEDS_SUPPORTED
    Interface::digital::output::LEDs leds = Interface::digital::output::LEDs(database);
#endif

#ifdef DISPLAY_SUPPORTED
    Interface::Display display(database);
#endif

#ifdef LEDS_SUPPORTED
#ifndef DISPLAY_SUPPORTED
    Interface::analog::Analog analog = Interface::analog::Analog(database, midi, leds, cInfo);
#else
    Interface::analog::Analog analog = Interface::analog::Analog(database, midi, leds, display, cInfo);
#endif
#else
#ifdef DISPLAY_SUPPORTED
    Interface::analog::Analog analog = Interface::analog::Analog(database, midi, display, cInfo);
#else
    Interface::analog::Analog analog = Interface::analog::Analog(database, midi, cInfo);
#endif
#endif
}    // namespace

namespace Board
{
    namespace detail
    {
        uint32_t adcReturnValue;
    }

    namespace io
    {
        int16_t getAnalogValue(uint8_t analogID)
        {
            return detail::adcReturnValue;
        }

        void continueAnalogReadout()
        {
        }

        bool isAnalogDataAvailable()
        {
            return true;
        }

        uint8_t getRGBID(uint8_t ledID)
        {
            return 0;
        }

        uint8_t getRGBaddress(uint8_t rgbID, Interface::digital::output::LEDs::rgbIndex_t index)
        {
            return 0;
        }

        bool setLEDfadeSpeed(uint8_t transitionSpeed)
        {
            return true;
        }

        void writeLEDstate(uint8_t ledID, bool state)
        {
        }
    }    // namespace io
}    // namespace Board

TEST_SETUP()
{
    //init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);
    TEST_ASSERT(database.isSignatureValid() == true);
    TEST_ASSERT(database.factoryReset(LESSDB::factoryResetType_t::full) == true);
    midi.handleUSBwrite(midiDataHandler);

    analog.disableExpFiltering();

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);
}

TEST_CASE(CCtest)
{
    using namespace Interface::analog;

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

    uint16_t expectedValue;

    Board::detail::adcReturnValue = 1000;

    resetReceived();
    expectedValue = core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
    analog.update();

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        uint8_t midiMessage = midiPacket[i].Event << 4;
        TEST_ASSERT(midiMessage == static_cast<uint8_t>(MIDI::messageType_t::controlChange));
        TEST_ASSERT(midiPacket[i].Data3 == expectedValue);
    }

    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ANALOG);

    //reset all values
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);

    //try with max value
    Board::detail::adcReturnValue = ADC_MAX_VALUE;

    resetReceived();
    analog.update();

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        TEST_ASSERT(midiPacket[i].Data3 == MIDI_7_BIT_VALUE_MAX);

    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ANALOG);
}

TEST_CASE(PitchBendTest)
{
    using namespace Interface::analog;

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

    uint16_t expectedValue;

    Board::detail::adcReturnValue = 1000;

    resetReceived();
    expectedValue = core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_14_BIT_VALUE_MAX));
    analog.update();

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        uint8_t midiMessage = midiPacket[i].Event << 4;
        TEST_ASSERT(midiMessage == static_cast<uint8_t>(MIDI::messageType_t::pitchBend));
        MIDI::encDec_14bit_t pitchBendValue;
        pitchBendValue.low  = midiPacket[i].Data2;
        pitchBendValue.high = midiPacket[i].Data3;
        pitchBendValue.mergeTo14bit();
        TEST_ASSERT(pitchBendValue.value == expectedValue);
    }

    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ANALOG);

    //call update again, verify no values have been sent
    resetReceived();

    analog.update();
    TEST_ASSERT(messageCounter == 0);

    //try with max value
    Board::detail::adcReturnValue = ADC_MAX_VALUE;
    analog.update();

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        MIDI::encDec_14bit_t pitchBendValue;
        pitchBendValue.low  = midiPacket[i].Data2;
        pitchBendValue.high = midiPacket[i].Data3;
        pitchBendValue.mergeTo14bit();
        TEST_ASSERT(pitchBendValue.value == MIDI_14_BIT_VALUE_MAX);
    }
}

TEST_CASE(ScalingAndInversion)
{
    using namespace Interface::analog;

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

    for (uint32_t i = ADC_MAX_VALUE + 1; i-- > 0;)
    {
        resetReceived();
        Board::detail::adcReturnValue = i;
        uint16_t expectedValue        = core::misc::mapRange(i, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));

        analog.update();
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ANALOG);

        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            TEST_ASSERT(midiPacket[j].Data3 == expectedValue);
            //reset debouncing state for easier testing - otherwise Board::detail::adcReturnValue
            //won't be accepted in next analog.update() call
            analog.debounceReset(j);
        }
    }

    //now enable inversion
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable inversion for all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 1) == true);
        analog.debounceReset(i);
    }

    for (uint32_t i = ADC_MAX_VALUE + 1; i-- > 0;)
    {
        resetReceived();
        Board::detail::adcReturnValue = i;
        uint32_t expectedValue        = static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX) - core::misc::mapRange(i, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));

        analog.update();
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ANALOG);

        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            TEST_ASSERT(midiPacket[j].Data3 == expectedValue);
            analog.debounceReset(j);
        }
    }

    //now do the same thing for pitch bend analog type
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with Pitch Bend MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::pitchBend)) == true);

        analog.debounceReset(i);
    }

    for (uint32_t i = ADC_MAX_VALUE + 1; i-- > 0;)
    {
        resetReceived();
        Board::detail::adcReturnValue = i;
        auto expectedValue            = core::misc::mapRange(i, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_14_BIT_VALUE_MAX));

        analog.update();
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ANALOG);

        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            MIDI::encDec_14bit_t pitchBendValue;
            pitchBendValue.low  = midiPacket[j].Data2;
            pitchBendValue.high = midiPacket[j].Data3;
            pitchBendValue.mergeTo14bit();
            TEST_ASSERT(pitchBendValue.value == expectedValue);
            analog.debounceReset(j);
        }
    }

    //now enable inversion
    for (uint32_t i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 1) == true);
        analog.debounceReset(i);
    }

    for (uint32_t i = ADC_MAX_VALUE + 1; i-- > 0;)
    {
        resetReceived();
        Board::detail::adcReturnValue = i;
        uint16_t expectedValue        = MIDI_14_BIT_VALUE_MAX - core::misc::mapRange(i, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_14_BIT_VALUE_MAX));

        analog.update();
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ANALOG);

        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            MIDI::encDec_14bit_t pitchBendValue;
            pitchBendValue.low  = midiPacket[j].Data2;
            pitchBendValue.high = midiPacket[j].Data3;
            pitchBendValue.mergeTo14bit();
            TEST_ASSERT(pitchBendValue.value == expectedValue);
            analog.debounceReset(j);
        }
    }

    //try with scaled upper value
    //still using pitch bend message type

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);

    const uint32_t scaledUpperValue = 100;

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, scaledUpperValue) == true);
    }

    resetReceived();
    Board::detail::adcReturnValue = ADC_MAX_VALUE;
    auto expectedValue            = scaledUpperValue;
    analog.update();
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ANALOG);

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        MIDI::encDec_14bit_t pitchBendValue;
        pitchBendValue.low  = midiPacket[i].Data2;
        pitchBendValue.high = midiPacket[i].Data3;
        pitchBendValue.mergeTo14bit();
        TEST_ASSERT(pitchBendValue.value == expectedValue);
    }
}

TEST_CASE(DebouncingSetup)
{
    //verify that the step diff is properly configured
    //don't allow step difference which would result in scenario where total number
    //of generated values would be less than 127
    TEST_ASSERT((ADC_MAX_VALUE / ANALOG_STEP_MIN_DIFF_7_BIT) >= 127);
}

TEST_CASE(Debouncing)
{
    using namespace Interface::analog;

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

    uint32_t expectedValue;
    uint32_t newMIDIvalue;

    resetReceived();
    Board::detail::adcReturnValue = 0;
    expectedValue                 = 0;
    analog.update();

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        TEST_ASSERT(midiPacket[i].Data3 == expectedValue);

    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ANALOG);

    //try again with the same values
    //no values should be sent
    resetReceived();
    analog.update();

    TEST_ASSERT(messageCounter == 0);

    //now test using different raw adc value which results in same MIDI value
    resetReceived();

    if ((ADC_MAX_VALUE / ANALOG_STEP_MIN_DIFF_7_BIT) > 127)
    {
        //it is possible that internally, ANALOG_STEP_MIN_DIFF_7_BIT is defined in a way
        //which would still result in the same MIDI value being generated
        //analog class shouldn't sent the same MIDI value even in this case
        Board::detail::adcReturnValue += ANALOG_STEP_MIN_DIFF_7_BIT;
        //verify that the newly generated value is indeed the same as the last one
        TEST_ASSERT(expectedValue == core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX)));
        analog.update();
        //no values should be sent since the resulting midi value is the same
        TEST_ASSERT(messageCounter == 0);

        //now increase the raw value until the newly generated midi value differs from the last one
        uint32_t newMIDIvalue;
        do
        {
            Board::detail::adcReturnValue += 1;
            newMIDIvalue = core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
        } while (newMIDIvalue == expectedValue);

        expectedValue = newMIDIvalue;

        //verify that the values have been sent now
        resetReceived();
        analog.update();
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ANALOG);

        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT(midiPacket[i].Data3 == expectedValue);
    }
    else
    {
        Board::detail::adcReturnValue += (ANALOG_STEP_MIN_DIFF_7_BIT - 1);
        //verify that the newly generated value is indeed the same as the last one
        TEST_ASSERT(expectedValue == core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX)));
        analog.update();
        //no values should be sent since the resulting midi value is the same
        TEST_ASSERT(messageCounter == 0);

        Board::detail::adcReturnValue += 1;
        //verify that the newly generated value differs from the last one
        newMIDIvalue = core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
        TEST_ASSERT(expectedValue != newMIDIvalue);
        expectedValue = newMIDIvalue;
        analog.update();
        //values should be sent now
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ANALOG);

        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT(midiPacket[i].Data3 == expectedValue);
    }

    //verify that the debouncing works in both ways
    //change the direction

    for (int i = 0; i < ANALOG_STEP_MIN_DIFF_7_BIT - 1; i++)
    {
        resetReceived();
        Board::detail::adcReturnValue -= 1;
        analog.update();
        TEST_ASSERT(messageCounter == 0);
    }

    //this time, values should be sent
    resetReceived();
    Board::detail::adcReturnValue -= 1;
    analog.update();
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_ANALOG);
}