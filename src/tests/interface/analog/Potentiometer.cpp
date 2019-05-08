#include <gtest/gtest.h>
#include "../../../../application/interface/analog/Analog.h"
#include "../../../../application/interface/CInfo.h"
#include "../../../../modules/midi/src/MIDI.h"
#include "../../../../modules/core/src/general/Timing.h"
#include "../../../../modules/core/src/general/Misc.h"
#include "../../../../application/database/Database.h"
#include "../../stubs/database/DB_ReadWrite.h"

//this represents theoretical maximum amount of analog components
//this test is run for all board variants
//make sure that the default is largest amount of analog components
#define MAX_MIDI_MESSAGES   96

namespace MIDIstub
{
    namespace detail
    {
        uint32_t ccMessageCounter = 0;
        uint32_t pbMessageCounter = 0;
        uint8_t ccValue[MAX_MIDI_MESSAGES] = {};
        uint16_t pbValue[MAX_MIDI_MESSAGES] = {};
    }

    void reset()
    {
        using namespace detail;

        ccMessageCounter = 0;
        pbMessageCounter = 0;
    }
}

void MIDI::sendControlChange(uint8_t inControlNumber, uint8_t inControlValue, uint8_t inChannel)
{
    MIDIstub::detail::ccValue[MIDIstub::detail::ccMessageCounter] = inControlValue;
    MIDIstub::detail::ccMessageCounter++;
}

void MIDI::sendPitchBend(uint16_t inPitchValue, uint8_t inChannel)
{
    MIDIstub::detail::pbValue[MIDIstub::detail::pbMessageCounter] = inPitchValue;
    MIDIstub::detail::pbMessageCounter++;
}

void MIDI::sendNoteOn(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel)
{
    
}

void MIDI::sendNoteOff(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel)
{
    
}

class PotentiometerTest : public ::testing::Test
{
    protected:
    virtual void SetUp()
    {
        cinfoHandler = nullptr;

        //init checks - no point in running further tests if these conditions fail
        EXPECT_TRUE(database.init());
        EXPECT_TRUE(database.getDBsize() < LESSDB_SIZE);
        EXPECT_TRUE(database.isSignatureValid());
    }

    virtual void TearDown()
    {
        
    }

    Database database = Database(DatabaseStub::memoryRead, DatabaseStub::memoryWrite);
    MIDI midi;
    Analog analog = Analog(database, midi);
};

namespace Board
{
    namespace detail
    {
        uint32_t adcReturnValue;
    }

    int16_t getAnalogValue(uint8_t analogID)
    {
        return detail::adcReturnValue;
    }

    void continueADCreadout()
    {

    }

    bool analogDataAvailable()
    {
        return true;
    }
}

TEST_F(PotentiometerTest, CCtest)
{
    //set known state
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_enable, i, 1), true);

        //disable invert state
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 0), true);

        //configure all analog components as potentiometers with CC MIDI message
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_type, i, aType_potentiometer_cc), true);

        //set all lower limits to 0
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_lowerLimit, i, 0), true);

        //set all upper limits to MIDI_7_BIT_VALUE_MAX
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i, MIDI_7_BIT_VALUE_MAX), true);
    }

    uint16_t expectedValue;

    Board::detail::adcReturnValue = 1000;

    MIDIstub::reset();
    expectedValue = mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(MIDIstub::detail::ccValue[i], expectedValue);

    EXPECT_EQ(MIDIstub::detail::ccMessageCounter, MAX_NUMBER_OF_ANALOG);

    //try again with the same values
    //no values should be sent
    MIDIstub::reset();
    analog.update();

    EXPECT_EQ(MIDIstub::detail::ccMessageCounter, 0);

    //reset all values
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);

    //try with max value
    Board::detail::adcReturnValue = ADC_MAX_VALUE;

    MIDIstub::reset();
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(MIDIstub::detail::ccValue[i], MIDI_7_BIT_VALUE_MAX);

    EXPECT_EQ(MIDIstub::detail::ccMessageCounter, MAX_NUMBER_OF_ANALOG);

    //test the following scenario:
    //1) record new value
    //2) change direction
    //3) verify that the new value cannot be recorded unless difference between them is ANALOG_STEP_MIN_DIFF_7_BIT*2

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);

    MIDIstub::reset();
    Board::detail::adcReturnValue = 512;
    expectedValue = mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(0), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
    analog.update();
    EXPECT_EQ(MIDIstub::detail::ccMessageCounter, MAX_NUMBER_OF_ANALOG);

    //no values should be sent for now
    MIDIstub::reset();
    Board::detail::adcReturnValue = 512-ANALOG_STEP_MIN_DIFF_7_BIT;
    expectedValue = mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(0), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
    analog.update();
    EXPECT_EQ(MIDIstub::detail::ccMessageCounter, 0);

    //enough difference has been reached -> new values should be sent
    MIDIstub::reset();
    Board::detail::adcReturnValue = 512-(ANALOG_STEP_MIN_DIFF_7_BIT*2);
    expectedValue = mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(0), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
    analog.update();
    EXPECT_EQ(MIDIstub::detail::ccMessageCounter, MAX_NUMBER_OF_ANALOG);
}

TEST_F(PotentiometerTest, PitchBendTest)
{
    //set known state
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_enable, i, 1), true);

        //disable invert state
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 0), true);

        //configure all analog components as potentiometers with Pitch Bend MIDI message
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_type, i, aType_PitchBend), true);

        //set all lower limits to 0
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_lowerLimit, i, 0), true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i, MIDI_14_BIT_VALUE_MAX), true);
    }

    uint16_t expectedValue;

    Board::detail::adcReturnValue = 1000;

    MIDIstub::reset();
    expectedValue = mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_14_BIT_VALUE_MAX));
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(MIDIstub::detail::pbValue[i], expectedValue);

    EXPECT_EQ(MIDIstub::detail::pbMessageCounter, MAX_NUMBER_OF_ANALOG);

    //call update again, verify no values have been sent
    MIDIstub::reset();

    analog.update();
    EXPECT_EQ(MIDIstub::detail::pbMessageCounter, 0);

    //try with max value
    Board::detail::adcReturnValue = ADC_MAX_VALUE;
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(MIDIstub::detail::pbValue[i], MIDI_14_BIT_VALUE_MAX);
}

TEST_F(PotentiometerTest, ScalingAndInversion)
{
    //set known state
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_enable, i, 1), true);

        //disable invert state
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 0), true);

        //configure all analog components as potentiometers with CC MIDI message
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_type, i, aType_potentiometer_cc), true);

        //set all lower limits to 0
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_lowerLimit, i, 0), true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i, MIDI_14_BIT_VALUE_MAX), true);
    }

    for (uint32_t i=ADC_MAX_VALUE+1; i-- > 0; )
    {
        MIDIstub::reset();
        Board::detail::adcReturnValue = i;
        uint16_t expectedValue = mapRange(i, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));

        analog.update();
        EXPECT_EQ(MIDIstub::detail::ccMessageCounter, MAX_NUMBER_OF_ANALOG);

        for (int j=0; j<MAX_NUMBER_OF_ANALOG; j++)
        {
            EXPECT_EQ(MIDIstub::detail::ccValue[j], expectedValue);
            //reset debouncing state for easier testing - otherwise Board::detail::adcReturnValue
            //won't be accepted in next analog.update() call
            analog.debounceReset(j);
        }
    }

    //now enable inversion
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable inversion for all analog components
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 1), true);
        analog.debounceReset(i);
    }

    for (uint32_t i=ADC_MAX_VALUE+1; i-- > 0; )
    {
        MIDIstub::reset();
        Board::detail::adcReturnValue = i;
        uint32_t expectedValue = static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX) - mapRange(i, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));

        analog.update();
        EXPECT_EQ(MIDIstub::detail::ccMessageCounter, MAX_NUMBER_OF_ANALOG);

        for (int j=0; j<MAX_NUMBER_OF_ANALOG; j++)
        {
            EXPECT_EQ(MIDIstub::detail::ccValue[j], expectedValue);
            analog.debounceReset(j);
        }
    }

    //now do the same thing for pitch bend analog type
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //disable invert state
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 0), true);

        //configure all analog components as potentiometers with Pitch Bend MIDI message
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_type, i, aType_PitchBend), true);

        analog.debounceReset(i);
    }

    for (uint32_t i=ADC_MAX_VALUE+1; i-- > 0; )
    {
        MIDIstub::reset();
        Board::detail::adcReturnValue = i;
        auto expectedValue = mapRange(i, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_14_BIT_VALUE_MAX));

        analog.update();
        EXPECT_EQ(MIDIstub::detail::pbMessageCounter, MAX_NUMBER_OF_ANALOG);

        for (int j=0; j<MAX_NUMBER_OF_ANALOG; j++)
        {
            EXPECT_EQ(MIDIstub::detail::pbValue[j], expectedValue);
            analog.debounceReset(j);
        }
    }

    //now enable inversion
    for (uint32_t i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 1), true);
        analog.debounceReset(i);
    }

    for (uint32_t i=ADC_MAX_VALUE+1; i-- > 0; )
    {
        MIDIstub::reset();
        Board::detail::adcReturnValue = i;
        uint16_t expectedValue = MIDI_14_BIT_VALUE_MAX - mapRange(i, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_14_BIT_VALUE_MAX));

        analog.update();
        EXPECT_EQ(MIDIstub::detail::pbMessageCounter, MAX_NUMBER_OF_ANALOG);

        for (int j=0; j<MAX_NUMBER_OF_ANALOG; j++)
        {
            EXPECT_EQ(MIDIstub::detail::pbValue[j], expectedValue);
            analog.debounceReset(j);
        }
    }

    //try with scaled upper value
    //still using pitch bend message type

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);

    const uint32_t scaledUpperValue = 100;

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 0), true);
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i, scaledUpperValue), true);
    }

    MIDIstub::reset();
    Board::detail::adcReturnValue = ADC_MAX_VALUE;
    auto expectedValue = scaledUpperValue;
    analog.update();
    EXPECT_EQ(MIDIstub::detail::pbMessageCounter, MAX_NUMBER_OF_ANALOG);

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(MIDIstub::detail::pbValue[i], expectedValue);
}