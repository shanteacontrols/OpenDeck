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
#define MAX_MIDI_MESSAGES   64

uint8_t ccValue[MAX_MIDI_MESSAGES];
uint8_t ccMessageCounter;

uint16_t pbValue[MAX_MIDI_MESSAGES];
uint8_t pbMessageCounter;

void MIDI::sendControlChange(uint8_t inControlNumber, uint8_t inControlValue, uint8_t inChannel)
{
    ccValue[ccMessageCounter] = inControlValue;
    ccMessageCounter++;
}

void MIDI::sendPitchBend(uint16_t inPitchValue, uint8_t inChannel)
{
    pbValue[pbMessageCounter] = inPitchValue;
    pbMessageCounter++;
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
        uint16_t adcReturnValue;
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

        //set all potentiometer to aType_potentiometer_cc type
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_type, i, aType_potentiometer_cc), true);

        //set all lower limits to 0
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_lowerLimit, i, 0), true);

        //set all upper limits to MIDI_7_BIT_VALUE_MAX
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i, MIDI_7_BIT_VALUE_MAX), true);
    }

    uint16_t expectedValue;

    Board::detail::adcReturnValue = 1000;

    ccMessageCounter = 0;
    expectedValue = mapRange_uint32(Board::detail::adcReturnValue, 0, ADC_MAX_VALUE, 0, MIDI_7_BIT_VALUE_MAX);
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(ccValue[i], expectedValue);

    EXPECT_EQ(ccMessageCounter, MAX_NUMBER_OF_ANALOG);

    //try again with the same values
    //no values should be sent
    ccMessageCounter = 0;
    analog.update();

    EXPECT_EQ(ccMessageCounter, 0);

    //reset all values
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);

    //try with max value
    Board::detail::adcReturnValue = ADC_MAX_VALUE;

    ccMessageCounter = 0;
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(ccValue[i], MIDI_7_BIT_VALUE_MAX);

    EXPECT_EQ(ccMessageCounter, MAX_NUMBER_OF_ANALOG);

    //reset all values
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);

    //try with scaled upper value
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i, 100), true);

    expectedValue = mapRange_uint32(Board::detail::adcReturnValue, 0, ADC_MAX_VALUE, 0, 100);
    ccMessageCounter = 0;
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(ccValue[i], expectedValue);
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

        //set all potentiometer to aType_PitchBend type
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_type, i, aType_PitchBend), true);

        //set all lower limits to 0
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_lowerLimit, i, 0), true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i, MIDI_14_BIT_VALUE_MAX), true);
    }

    uint16_t expectedValue;

    Board::detail::adcReturnValue = 1000;

    pbMessageCounter = 0;
    expectedValue = mapRange_uint32(Board::detail::adcReturnValue, 0, ADC_MAX_VALUE, 0, MIDI_14_BIT_VALUE_MAX);
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(pbValue[i], expectedValue);

    EXPECT_EQ(pbMessageCounter, MAX_NUMBER_OF_ANALOG);

    //call update again, verify no values have been sent
    pbMessageCounter = 0;

    analog.update();
    EXPECT_EQ(pbMessageCounter, 0);

    //try with max value
    Board::detail::adcReturnValue = ADC_MAX_VALUE;
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(pbValue[i], MIDI_14_BIT_VALUE_MAX);
}