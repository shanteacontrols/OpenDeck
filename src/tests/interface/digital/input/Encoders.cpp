#include <gtest/gtest.h>
#include "../../../../application/interface/digital/input/encoders/Encoders.h"
#include "../../../../application/interface/CInfo.h"
#include "../../../../modules/midi/src/MIDI.h"
#include "../../../../modules/core/src/general/Timing.h"
#include "../../../../application/database/Database.h"
#include "../../../stubs/database/DB_ReadWrite.h"

#define MAX_MIDI_MESSAGES   32

uint8_t controlValue[MAX_MIDI_MESSAGES];
uint8_t messageCounter;
encoderPosition_t encoderPosition[MAX_MIDI_MESSAGES];
uint8_t encoderResultCounter;

void MIDI::sendControlChange(uint8_t inControlNumber, uint8_t inControlValue, uint8_t inChannel)
{
    controlValue[messageCounter] = inControlValue;
    messageCounter++;
}

void MIDI::sendProgramChange(uint8_t inProgramNumber, uint8_t inChannel)
{

}

class EncodersTest : public ::testing::Test
{
    protected:
    virtual void SetUp()
    {
        cinfoHandler = nullptr;

        //init checks - no point in running further tests if these conditions fail
        database.init();
        EXPECT_TRUE(database.getDBsize() < LESSDB_SIZE);
        EXPECT_TRUE(database.isSignatureValid());
    }

    virtual void TearDown()
    {
        
    }

    Database database = Database(DatabaseStub::memoryRead, DatabaseStub::memoryWrite);
    MIDI midi;
    Encoders encoders = Encoders(database, midi);
};

namespace Board
{
    encoderPosition_t getEncoderState(uint8_t encoderID, uint8_t pulsesPerStep)
    {
        return encoderPosition[encoderResultCounter];
    }
}

TEST_F(EncodersTest, Debounce)
{
    //set known state
    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
    {
        //enable all encoders
        EXPECT_EQ(database.update(DB_BLOCK_ENCODERS, dbSection_encoders_enable, i, 1), true);

        //disable invert state
        EXPECT_EQ(database.update(DB_BLOCK_ENCODERS, dbSection_encoders_invert, i, 0), true);

        //set type of message to encType7Fh01h
        EXPECT_EQ(database.update(DB_BLOCK_ENCODERS, dbSection_encoders_mode, i, encType7Fh01h), true);
    }

    encoderResultCounter = 0;
    messageCounter = 0;
    rTime_ms = 0;

    encoderPosition[encoderResultCounter] = encMoveLeft;

    encoders.update();

    //verify that all received values are correct
    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
        EXPECT_EQ(controlValue[i], encValue[static_cast<uint8_t>(encType7Fh01h)][static_cast<uint8_t>(encMoveLeft)]);

    EXPECT_EQ(messageCounter, MAX_NUMBER_OF_ENCODERS);

    //reset values
    messageCounter = 0;
    //set new direction
    encoderPosition[encoderResultCounter] = encMoveRight;

    encoders.update();

    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
        EXPECT_EQ(controlValue[i], encValue[static_cast<uint8_t>(encType7Fh01h)][static_cast<uint8_t>(encMoveRight)]);

    EXPECT_EQ(messageCounter, MAX_NUMBER_OF_ENCODERS);

    //test the scenario where the values alternate between last position
    //(right) and different position (left)
    //after four consecutive values debouncer should ignore all futher
    //changes in direction until encoder slows down

    encoderPosition_t encoderPositionTest1[MAX_MIDI_MESSAGES] =
    {
        encMoveRight,
        encMoveRight,
        encMoveRight,
        encMoveRight,
        encMoveLeft,
        encMoveRight,
        encMoveLeft,
        encMoveLeft,
        encMoveLeft,
        encMoveRight,
        encMoveRight,
        encMoveRight,
        encMoveLeft,
        encMoveRight,
        encMoveRight,
        encMoveRight,
        encMoveLeft,
        encMoveLeft,
        encMoveLeft,
        encMoveRight,
        encMoveRight,
        encMoveLeft,
        encMoveRight,
        encMoveLeft,
        encMoveLeft,
        encMoveLeft,
        encMoveRight,
        encMoveRight,
        encMoveRight,
        encMoveRight,
        encMoveLeft,
        encMoveRight
    };

    for (int i=0; i<MAX_MIDI_MESSAGES; i++)
        encoderPosition[i] = encoderPositionTest1[i];

    rTime_ms += DEBOUNCE_RESET_TIME+1;

    //all values should be right position
    //call ::update n times (MAX_MIDI_MESSAGES)
    //each call retrieves encoder position from test array
    //after each call switch to next value from test array
    for (int i=0; i<MAX_MIDI_MESSAGES; i++)
    {
        messageCounter = 0;
        encoders.update();

        for (int j=0; j<MAX_NUMBER_OF_ENCODERS; j++)
            EXPECT_EQ(controlValue[j], encValue[(uint8_t)encType7Fh01h][(uint8_t)encMoveRight]);

        encoderResultCounter++;
    }

    //perform the same test again but with time difference
    //returned values should match the values in test array:
    //debouncer should not be initiated
    encoderResultCounter = 0;
    rTime_ms = DEBOUNCE_RESET_TIME+1;

    for (int i=0; i<MAX_MIDI_MESSAGES; i++)
    {
        messageCounter = 0;
        encoders.update();

        for (int j=0; j<MAX_NUMBER_OF_ENCODERS; j++)
            EXPECT_EQ(controlValue[j], encValue[(uint8_t)encType7Fh01h][encoderPosition[encoderResultCounter]]);

        encoderResultCounter++;
        rTime_ms += DEBOUNCE_RESET_TIME+1;
    }

    //test the following scenario:
    //1) debouncer becomes initated and starts ignoring the changes in direction
    //2) after n values (16) encoder starts spinning in opposite direction more than
    // DEBOUNCE_COUNT times
    //test whether the debouncer registered the valid change

    encoderPosition_t encoderPositionTest2[MAX_MIDI_MESSAGES] =
    {
        encMoveRight,
        encMoveRight,
        encMoveRight,
        encMoveRight,
        encMoveLeft,
        encMoveRight,
        encMoveLeft,
        encMoveLeft,
        encMoveLeft,
        encMoveRight,
        encMoveRight,
        encMoveRight,
        encMoveLeft,
        encMoveRight,
        encMoveRight,
        encMoveRight,
        encMoveLeft,
        encMoveLeft,
        encMoveLeft,
        encMoveLeft,
        encMoveRight,
        encMoveRight,
        encMoveLeft,
        encMoveRight,
        encMoveLeft,
        encMoveLeft,
        encMoveLeft,
        encMoveLeft,
        encMoveRight,
        encMoveRight,
        encMoveRight,
        encMoveLeft,
    };
 
    for (int i=0; i<MAX_MIDI_MESSAGES; i++)
        encoderPosition[i] = encoderPositionTest2[i];

    encoderResultCounter = 0;
    rTime_ms += DEBOUNCE_RESET_TIME+1;

    for (int i=0; i<16; i++)
    {
        messageCounter = 0;
        encoders.update();

        for (int j=0; j<MAX_NUMBER_OF_ENCODERS; j++)
            EXPECT_EQ(controlValue[j], encValue[(uint8_t)encType7Fh01h][(uint8_t)encMoveRight]);

        encoderResultCounter++;
    }

    for (int i=16; i<19; i++)
    {
        //first three values should still be right direction
        messageCounter = 0;
        encoders.update();

        for (int j=0; j<MAX_NUMBER_OF_ENCODERS; j++)
            EXPECT_EQ(controlValue[j], encValue[(uint8_t)encType7Fh01h][(uint8_t)encMoveRight]);

        encoderResultCounter++;
    }

    //from now on, debouncer should be initiated and all other values should be left
    for (int i=19; i<MAX_MIDI_MESSAGES; i++)
    {
        //first three values should still be right direction
        messageCounter = 0;
        encoders.update();

        for (int j=0; j<MAX_NUMBER_OF_ENCODERS; j++)
            EXPECT_EQ(controlValue[j], encValue[(uint8_t)encType7Fh01h][(uint8_t)encMoveLeft]);

        encoderResultCounter++;
    }
}