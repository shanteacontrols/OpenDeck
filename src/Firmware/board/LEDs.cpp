#include "Board.h"

volatile bool       blinkEnabled,
                    blinkState;

volatile uint8_t    pwmSteps,
                    ledState[MAX_NUMBER_OF_LEDS],
                    activeLEDcolumn;

volatile uint16_t   ledBlinkTime;

volatile int8_t     transitionCounter[MAX_NUMBER_OF_LEDS];
volatile uint32_t   blinkTimerCounter;


uint8_t Board::getRGBaddress(uint8_t rgbID, rgbIndex_t index)
{
    //get RGB LED address for specified index
    uint8_t column = rgbID % NUMBER_OF_LED_COLUMNS;
    uint8_t row  = (rgbID/NUMBER_OF_BUTTON_COLUMNS)*3;

    uint8_t address = column + NUMBER_OF_LED_COLUMNS*row;

    switch(index)
    {
        case rgb_R:
        return address;

        case rgb_G:
        return address + NUMBER_OF_LED_COLUMNS*1;

        case rgb_B:
        return address + NUMBER_OF_LED_COLUMNS*2;
    }

    return 0;
}

uint8_t Board::getRGBID(uint8_t ledNumber)
{
    uint8_t row = ledNumber/NUMBER_OF_LED_COLUMNS;

    uint8_t mod = row%3;    //RGB LED = 3 normal LEDs
    row -= mod;

    uint8_t column = ledNumber % NUMBER_OF_BUTTON_COLUMNS;

    return (row*NUMBER_OF_LED_COLUMNS)/3 + column;
}