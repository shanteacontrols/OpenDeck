#include <gtest/gtest.h>
#include "../../firmware/interface/analog/Analog.h"

MIDImessage_t           dinMessage,
                        usbMessage;

database_t database;
board_t board;
midi_t midi;
sysEx_t sysEx;
buttons_t buttons;
leds_t leds;

class InterfaceAnalogTest : public ::testing::Test
{
    protected:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {
        
    }
};

TEST_F(InterfaceAnalogTest, DebounceReset)
{
    analog.update();
}