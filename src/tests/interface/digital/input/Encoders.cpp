#include <gtest/gtest.h>
#include "../../../../application/interface/digital/input/encoders/Encoders.h"
#include "../../../../application/interface/CInfo.h"
#include "../../../../modules/midi/src/MIDI.h"
#include "../../../../modules/core/src/general/Timing.h"
#include "../../../../application/database/Database.h"
#include "../../../stubs/database/DB_ReadWrite.h"

#define MAX_MIDI_MESSAGES   MAX_NUMBER_OF_ENCODERS

uint8_t controlValue[MAX_MIDI_MESSAGES];
uint8_t messageCounter;
encoderPosition_t encoderPosition;

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
        EXPECT_EQ(database.getDBsize() < LESSDB_SIZE, true);

        //confirm that signature is valid after database initialization
        EXPECT_EQ(database.signatureValid(), true);
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
        return encoderPosition;
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

    encoderPosition = encMoveLeft;
    messageCounter = 0;
    rTime_ms = 0;

    encoders.update();

    //verify that all received values are 127 (left movement = value 127)
    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
        EXPECT_EQ(controlValue[i], 127);

    EXPECT_EQ(messageCounter, MAX_NUMBER_OF_ENCODERS);

    //reset values
    messageCounter = 0;
    //set new direction
    //time hasn't changed - debouncer should override/ignore this value
    encoderPosition = encMoveRight;

    encoders.update();

    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
        EXPECT_EQ(controlValue[i], 127);

    EXPECT_EQ(messageCounter, MAX_NUMBER_OF_ENCODERS);

    //two consecutives values in row should reset the debouncer (change direction)
    messageCounter = 0;

    encoders.update();

    //verify that all received values are 1 (right movement = value 1)
    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
        EXPECT_EQ(controlValue[i], 1);

    EXPECT_EQ(messageCounter, MAX_NUMBER_OF_ENCODERS);
}