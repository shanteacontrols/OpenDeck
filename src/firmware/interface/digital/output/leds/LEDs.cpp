/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "LEDs.h"

static bool         blinkState;

volatile uint8_t    pwmSteps;
uint8_t             ledState[MAX_NUMBER_OF_LEDS];

static uint32_t     ledBlinkTime,
                    lastLEDblinkUpdateTime;

volatile int8_t     transitionCounter[MAX_NUMBER_OF_LEDS];


LEDs::LEDs()
{
    //def const
}

void LEDs::init()
{
    if (!database.read(DB_BLOCK_LED, ledHardwareParameterSection, ledHwParameterBlinkTime))
    {
        //make sure to set default blink time
        database.update(DB_BLOCK_LED, ledHardwareParameterSection, ledHwParameterBlinkTime, BLINK_TIME_MIN);
    }

    setBlinkTime(database.read(DB_BLOCK_LED, ledHardwareParameterSection, ledHwParameterBlinkTime));
    #ifdef BOARD_OPEN_DECK
    setFadeTime(database.read(DB_BLOCK_LED, ledHardwareParameterSection, ledHwParameterFadeTime));
    #endif

    //run LED animation on start-up
    startUpAnimation();
}

void LEDs::update()
{
    if ((rTimeMs() - lastLEDblinkUpdateTime) >= ledBlinkTime)
    {
        //time to update blinking leds
        //change blinkBit state and write it into ledState variable if LED is in blink state
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        {
            if (BIT_READ(ledState[i], LED_BLINK_ON_BIT))
            {
                if (blinkState)
                    BIT_SET(ledState[i], LED_STATE_BIT);
                else
                    BIT_CLEAR(ledState[i], LED_STATE_BIT);
            }
        }

        blinkState = !blinkState;
        lastLEDblinkUpdateTime = rTimeMs();
    }
}

void LEDs::startUpAnimation()
{
    if (database.read(DB_BLOCK_LED, ledHardwareParameterSection, ledHwParameterStartUpRoutine))
    {
        #ifdef BOARD_OPEN_DECK
        setFadeTime(1);
        #endif
        setAllOn();
        wait_ms(2000);
        setAllOff();
        wait_ms(2000);
        #ifdef BOARD_OPEN_DECK
        setFadeTime(database.read(DB_BLOCK_LED, ledHardwareParameterSection, ledHwParameterFadeTime));
        #endif
    }
}

ledColor_t LEDs::velocityToColor(uint8_t receivedVelocity)
{
    /*
        Velocity    Color       Color Index
        0-15        Off         0
        16-31       Red         1
        32-47       Green       2
        48-63       Yellow      3
        64-79       Blue        4
        80-95       Magenta     5
        96-111      Cyan        6
        112-127     White       7
    */

    return (ledColor_t)(receivedVelocity/16);
}

void LEDs::ccToBlink(uint8_t cc, uint8_t value, uint8_t channel)
{
    bool blink = (bool)value;

    //match LED activation note with received cc
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        if ((database.read(DB_BLOCK_LED, ledActivationNoteSection, i) == cc) && (database.read(DB_BLOCK_LED, ledMIDIchannelSection, i) == channel))
        {
            setBlinkState(i, blink);
        }
    }
}

void LEDs::noteToState(uint8_t receivedNote, uint8_t receivedVelocity, uint8_t receivedChannel, bool local)
{
    ledColor_t color = velocityToColor(receivedVelocity);

    //match LED activation note with its index
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        if (database.read(DB_BLOCK_LED, ledActivationNoteSection, i) == receivedNote)
        {
            //change color if rgb led is disabled
            //any color will do
            if (!database.read(DB_BLOCK_LED, ledRGBenabledSection, i))
                color = (database.read(DB_BLOCK_LED, ledSingleVelocityValueSection, i) == receivedVelocity) ? colorRed : colorOff;

            if (local)
            {
                //if local is set to true, check if local led control is enabled for this led before changing state
                if (database.read(DB_BLOCK_LED, ledLocalControlSection, i))
                    setColor(i, color);
            }
            else if (database.read(DB_BLOCK_LED, ledMIDIchannelSection, i) == receivedChannel)
            {
                //channels must match here
                setColor(i, color);
            }
        }
    }
}

void LEDs::setBlinkState(uint8_t ledID, bool state)
{
    uint8_t ledArray[3], leds = 0;

    if (database.read(DB_BLOCK_LED, ledRGBenabledSection, board.getRGBID(ledID)))
    {
        ledArray[0] = board.getRGBaddress(ledID, rgb_R);
        ledArray[1] = board.getRGBaddress(ledID, rgb_G);
        ledArray[2] = board.getRGBaddress(ledID, rgb_B);

        leds = 3;
    }
    else
    {
        ledArray[0] = ledID;
        ledArray[1] = ledID;
        ledArray[2] = ledID;

        leds = 1;
    }

    for (int i=0; i<leds; i++)
    {
        if (state)
        {
            BIT_SET(ledState[ledArray[i]], LED_BLINK_ON_BIT);
            //this will turn the led immediately no matter how little time it's
            //going to blink first time
            BIT_SET(ledState[ledArray[i]], LED_STATE_BIT);
        }
        else
        {
            BIT_CLEAR(ledState[ledArray[i]], LED_BLINK_ON_BIT);
            BIT_WRITE(ledState[ledArray[i]], LED_STATE_BIT, BIT_READ(ledState[i], LED_ACTIVE_BIT));
        }
    }
}

void LEDs::setAllOn()
{
    //turn on all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        setColor(i, colorRed);
}

void LEDs::setAllOff()
{
    //turn off all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        setColor(i, colorOff);
}

void LEDs::setColor(uint8_t ledNumber, ledColor_t color)
{
    if (database.read(DB_BLOCK_LED, ledRGBenabledSection, board.getRGBID(ledNumber)))
    {
        uint8_t led1 = board.getRGBaddress(ledNumber, rgb_R);
        uint8_t led2 = board.getRGBaddress(ledNumber, rgb_G);
        uint8_t led3 = board.getRGBaddress(ledNumber, rgb_B);

        handleLED(led1, BIT_READ(color, 0));
        handleLED(led2, BIT_READ(color, 1));
        handleLED(led3, BIT_READ(color, 2));
    }
    else
    {
        handleLED(ledNumber, (bool)color);
    }
}

ledColor_t LEDs::getColor(uint8_t ledID)
{
    uint8_t state = getState(ledID);

    if (!BIT_READ(state, LED_ACTIVE_BIT))
    {
        return colorOff;
    }
    else
    {
        if (!BIT_READ(state, LED_RGB_BIT))
        {
            //single color led
            return colorRed;
        }
        else
        {
            //rgb led
            uint8_t led1 = getState(board.getRGBaddress(ledID, rgb_R));
            uint8_t led2 = getState(board.getRGBaddress(ledID, rgb_G));
            uint8_t led3 = getState(board.getRGBaddress(ledID, rgb_B));

            uint8_t color = 0;
            color |= BIT_READ(led1, LED_RGB_B_BIT);
            color <<= 1;
            color |= BIT_READ(led2, LED_RGB_G_BIT);
            color <<= 1;
            color |= BIT_READ(led3, LED_RGB_R_BIT);

            return (ledColor_t)color;
        }
    }
}

uint8_t LEDs::getState(uint8_t ledNumber)
{
    return ledState[ledNumber];
}

bool LEDs::getBlinkState(uint8_t ledID)
{
    return BIT_READ(ledState[ledID], LED_BLINK_ON_BIT);
}

void LEDs::setFadeTime(uint8_t transitionSpeed)
{
    //reset transition counter
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            transitionCounter[i] = 0;

        pwmSteps = transitionSpeed;
    }
}

void LEDs::setBlinkTime(uint16_t blinkTime)
{
    ledBlinkTime = blinkTime*100;
    lastLEDblinkUpdateTime = 0;
}

void LEDs::handleLED(uint8_t ledNumber, bool state, bool rgbLED, rgbIndex_t index)
{
    /*

    LED state is stored into one byte (ledState). The bits have following meaning (7 being the MSB bit):

    7: B index of RGB LED
    6: G index of RGB LED
    5: R index of RGB LED
    4: RGB enabled
    3: Blink bit (timer changes this bit)
    2: LED is active (either it blinks or it's constantly on), this bit is OR function between bit 0 and 1
    1: LED blinks
    0: LED is constantly turned on

    */

    uint8_t currentState = getState(ledNumber);

    if (state)
    {
        //turn on the led
        //if led was already active, clear the on bits before setting new state
        if (BIT_READ(currentState, LED_ACTIVE_BIT))
            currentState = 0;

        BIT_SET(currentState, LED_ACTIVE_BIT);
        BIT_SET(currentState, LED_STATE_BIT);
        BIT_WRITE(currentState, LED_RGB_BIT, rgbLED);

        if (rgbLED)
        {
            switch(index)
            {
                case rgb_R:
                BIT_WRITE(currentState, LED_RGB_R_BIT, state);
                break;

                case rgb_G:
                BIT_WRITE(currentState, LED_RGB_G_BIT, state);
                break;

                case rgb_B:
                BIT_WRITE(currentState, LED_RGB_B_BIT, state);
                break;
            }
        }
    }
    else
    {
        //turn off the led
        currentState = 0;
    }

    ledState[ledNumber] = currentState;
}

LEDs leds;
