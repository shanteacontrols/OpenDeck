/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015, 2016 Igor Petrovic

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
#include "../../board/Board.h"

LEDs::LEDs()
{
    //def const
}

void LEDs::init()
{
    if (!database.read(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterBlinkTime))
    {
        //make sure to set default blink time
        database.update(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterBlinkTime, BLINK_TIME_MIN);
    }

    setBlinkTime(database.read(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterBlinkTime));
    setFadeTime(database.read(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterFadeTime));

    //run LED animation on start-up
    //startUpAnimation();
}

void LEDs::startUpAnimation()
{
    if (!database.read(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterTotalLEDnumber) || !database.read(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterStartUpSwitchTime))
        return;

    //turn off all LEDs before starting animation
    setAllOff();

    //switch (database.read(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterStartUpRoutine))
    //{
        //case 1:
        //oneByOne(true, true, true);
        //oneByOne(false, false, true);
        //oneByOne(true, false, false);
        //oneByOne(false, true, true);
        //oneByOne(true, false, true);
        //oneByOne(false, false, false);
        //break;
//
        //case 2:
        //oneByOne(true, false, true);
        //oneByOne(false, false, false);
        //break;
//
        //case 3:
        //oneByOne(true, true, true);
        //oneByOne(false, true, true);
        //break;
//
        //case 4:
        //oneByOne(true, false, true);
        //oneByOne(true, false, false);
        //break;
//
        //case 5:
        //oneByOne(true, false, true);
        //break;
//
        //default:
        //break;
    //}

    setAllOff();
    wait(1000);
}

rgbValue_t LEDs::velocityToColor(uint8_t receivedVelocity)
{
    /*
        0-15 off
        16-31 white
        32-47 cyan
        48-63 magenta
        64-79 red
        80-95 blue
        96-111 yellow
        112-127 green
    */

    ledColor_t color = (ledColor_t)(receivedVelocity/16);
    return colors[(uint8_t)color];
}

void LEDs::ccToBlink(uint8_t cc, uint8_t value)
{
    bool blink = (value == 127);

    //match LED activation note with received cc
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        if (database.read(CONF_BLOCK_LED, ledActivationNoteSection, i) == cc)
        {
            blinkLED(i, blink);
        }
    }

    checkBlinkLEDs();
}

void LEDs::noteToState(uint8_t receivedNote, uint8_t receivedVelocity, bool ledID, bool local)
{
    rgbValue_t color = velocityToColor(receivedVelocity);

    if (!ledID)
    {
        //match LED activation note with its index
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        {
            if (database.read(CONF_BLOCK_LED, ledActivationNoteSection, i) == receivedNote)
            {
                if (database.read(CONF_BLOCK_LED, ledRGBenabledSection, i))
                {
                    if (local)
                    {
                        //if local is set to true, check if local led control is enabled for this led before changing state
                        if (database.read(CONF_BLOCK_LED, ledLocalControlSection, i))
                            handleLED(i, color);
                    }
                    else
                    {
                        handleLED(i, color);
                    }
                }
                else
                {
                    bool state = color.r || color.g || color.b;
                    if (local)
                    {
                        if (database.read(CONF_BLOCK_LED, ledLocalControlSection, i))
                            handleLED(i, state);
                    }
                    else
                    {
                        handleLED(i, state);
                    }
                }
            }
        }
    }
    else
    {
        //treat received note as led ID
        //we can ignore local argument here
        if (database.read(CONF_BLOCK_LED, ledRGBenabledSection, receivedNote))
        {
            //rgb led
            handleLED(receivedNote, color);
        }
        else
        {
            bool state = color.r || color.g || color.b;
            handleLED(receivedNote, state);
        }
    }

    checkBlinkLEDs();
}

void LEDs::blinkLED(uint8_t ledID, bool state)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if (state)
        {
            bitSet(ledState[ledID], LED_BLINK_ON_BIT);
            //this will turn the led immediately no matter how little time it's
            //going to blink first time
            bitSet(ledState[ledID], LED_BLINK_STATE_BIT);
        }
        else
        {
            bitClear(ledState[ledID], LED_BLINK_ON_BIT);
            bitClear(ledState[ledID], LED_BLINK_STATE_BIT);
        }
    }
}

void LEDs::setAllOn()
{
    //turn on all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        handleLED(i, true);
}

void LEDs::setAllOff()
{
    //turn off all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        handleLED(i, false);
}

void LEDs::setState(uint8_t ledNumber, bool state)
{
    handleLED(ledNumber, state);
}

void LEDs::setColor(uint8_t ledNumber, rgbValue_t color)
{
    handleLED(ledNumber, color);
}

ledColor_t LEDs::getColor(uint8_t ledID)
{
    if (!database.read(CONF_BLOCK_LED, ledRGBenabledSection, ledID))
        return (ledColor_t)(127/16); //127 = velocity, 16 = color divider
    else
        return colorWhite;
}

uint8_t LEDs::getState(uint8_t ledNumber)
{
    uint8_t returnValue;
    returnValue = ledState[ledNumber];
    return returnValue;
}

bool LEDs::isBlinking(uint8_t ledID)
{
    uint8_t state = getState(ledID);
    return bitRead(state, LED_BLINK_ON_BIT);
}

bool LEDs::allLEDsOn()
{
    //return true if all LEDs are on
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        if (getState(i))
            return false;
    }

    return true;
}

bool LEDs::allLEDsOff()
{
    //return true if all LEDs are off
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        if (!getState(i))
            return false;
    }

    return true;
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
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        ledBlinkTime = blinkTime*100;
        blinkTimerCounter = 0;
    }
}

void LEDs::handleLED(uint8_t ledNumber, bool state)
{
    /*

    LED state is stored into one byte (ledState). The bits have following meaning (7 being the MSB bit):

    7: x
    6: x
    5: x
    4: x
    3: Blink bit (timer changes this bit)
    2: LED is active (either it blinks or it's constantly on), this bit is OR function between bit 0 and 1
    1: LED blinks
    0: LED is constantly turned on

    */

    uint8_t currentState = getState(ledNumber);

    switch(state) {

        case false:
        //turn off the led
        currentState = 0;
        break;

        case true:
        //turn on the led
        //if led was already active, clear the on bits before setting new state
        if (bitRead(currentState, LED_ACTIVE_BIT))
            currentState = 0;

        bitSet(currentState, LED_ACTIVE_BIT);
        bitSet(currentState, LED_CONSTANT_ON_BIT);
        break;
    }

    ledState[ledNumber] = currentState;
}

void LEDs::handleLED(uint8_t ledNumber, rgbValue_t color)
{
    uint8_t led1 = board.getRGBaddress(ledNumber, rgb_R);
    uint8_t led2 = board.getRGBaddress(ledNumber, rgb_G);
    uint8_t led3 = board.getRGBaddress(ledNumber, rgb_B);

    handleLED(led1, color.r);
    handleLED(led2, color.g);
    handleLED(led3, color.b);
}

void LEDs::checkBlinkLEDs()
{
    //this function will disable blinking
    //if none of the LEDs is in blinking state

    //else it will enable it

    bool _blinkEnabled = false;
    uint8_t ledState;

    //if any LED is blinking, set timerState to true and exit the loop
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        ledState = getState(i);
        if (bitRead(ledState, LED_BLINK_ON_BIT) && bitRead(ledState, LED_ACTIVE_BIT))
        {
            _blinkEnabled = true;
            break;
        }
    }

    if (_blinkEnabled && !blinkEnabled)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            blinkEnabled = true;
            blinkState = true;
            blinkTimerCounter = 0;
        }
    }
    else if (!_blinkEnabled && blinkEnabled)
    {
        //don't bother reseting variables if blinking is already disabled
        //reset blinkState to default value
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            blinkState = true;
            blinkTimerCounter = 0;
            blinkEnabled = false;
        }
    }
}

LEDs leds;
