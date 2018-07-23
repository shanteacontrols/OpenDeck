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
#include "board/Board.h"

static bool         blinkState;

volatile uint8_t    pwmSteps;
uint8_t             ledState[MAX_NUMBER_OF_LEDS];

static uint32_t     ledBlinkTime,
                    lastLEDblinkUpdateTime;

volatile int8_t     transitionCounter[MAX_NUMBER_OF_LEDS];

#define BLINK_TIME_MIN_INT  (BLINK_TIME_MIN*100)
#define BLINK_TIME_MAX_INT  (BLINK_TIME_MAX*100)


LEDs::LEDs()
{
    //def const
}

void LEDs::init()
{
    setBlinkTime(database.read(DB_BLOCK_LEDS, dbSection_leds_hw, ledHwParameterBlinkTime)*BLINK_TIME_SYSEX_MULTIPLIER);

    if (database.read(DB_BLOCK_LEDS, dbSection_leds_hw, ledHwParameterStartUpRoutine))
    {
        //set to slowest fading speed for effect
        #ifdef LED_FADING_SUPPORTED
        setFadeTime(1);
        #endif

        if (board.startUpAnimation != NULL)
            board.startUpAnimation();
        else
            startUpAnimation();
    }

    #ifdef LED_FADING_SUPPORTED
    setFadeTime(database.read(DB_BLOCK_LEDS, dbSection_leds_hw, ledHwParameterFadeTime));
    #endif
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
    #ifdef LED_FADING_SUPPORTED
    setFadeTime(1);
    #endif
    setAllOn();
    wait_ms(2000);
    setAllOff();
    wait_ms(2000);
    #ifdef LED_FADING_SUPPORTED
    setFadeTime(database.read(DB_BLOCK_LEDS, dbSection_leds_hw, ledHwParameterFadeTime));
    #endif
}

ledColor_t LEDs::valueToColor(uint8_t receivedVelocity)
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

void LEDs::midiToState(midiMessageType_t messageType, uint8_t data1, uint8_t data2, uint8_t channel, bool local)
{
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        bool setState = false;
        bool setBlink = false;

        //determine whether led state or blink state should be changed
        if (local)
        {
            switch(database.read(DB_BLOCK_LEDS, dbSection_leds_controlType, i))
            {
                case ledControlLocal_Note:
                if ((messageType == midiMessageNoteOn) || (messageType == midiMessageNoteOff))
                    setState = true;
                break;

                case ledControlLocal_CC:
                if (messageType == midiMessageControlChange)
                    setState = true;
                break;

                case ledControlLocal_PC:
                if (messageType == midiMessageProgramChange)
                {
                    setState = true;
                    //2 byte message - set third byte to 127 so that color is never colorOff
                    data2 = 127;
                }
                break;

                default:
                break;
            }
        }
        else
        {
            switch(database.read(DB_BLOCK_LEDS, dbSection_leds_controlType, i))
            {
                case ledControlMIDIin_noteCC:
                if ((messageType == midiMessageNoteOn) || (messageType == midiMessageNoteOff))
                    setState = true;
                else if (messageType == midiMessageControlChange)
                    setBlink = true;
                break;

                case ledControlMIDIin_CCnote:
                if ((messageType == midiMessageNoteOn) || (messageType == midiMessageNoteOff))
                    setBlink = true;
                else if (messageType == midiMessageControlChange)
                    setState = true;
                break;

                case ledControlMIDIin_PC:
                if (messageType == midiMessageProgramChange)
                {
                    setState = true;
                    data2 = 127;
                }
                break;

                default:
                break;
            }
        }

        if (setState)
        {
            //base color on last midi byte
            ledColor_t color = valueToColor(data2);

            //match LED activation ID with its index
            if (database.read(DB_BLOCK_LEDS, dbSection_leds_activationID, i) == data1)
            {
                //on program change messages, third byte (data2) isn't present
                //data2 is normally compared to activationValue
                //in this case, use value from data1
                if (messageType == midiMessageProgramChange)
                    data2 = data1;

                //change color if rgb led is disabled
                //any color will do
                if (!database.read(DB_BLOCK_LEDS, dbSection_leds_rgbEnable, i))
                    color = (database.read(DB_BLOCK_LEDS, dbSection_leds_activationValue, i) == data2) ? colorRed : colorOff;

                //finally, match channel
                if (database.read(DB_BLOCK_LEDS, dbSection_leds_midiChannel, i) == channel)
                    setColor(i, color);
            }
        }

        if (setBlink)
        {
            //match LED activation note with received cc
            if ((database.read(DB_BLOCK_LEDS, dbSection_leds_activationID, i) == data1) && (database.read(DB_BLOCK_LEDS, dbSection_leds_midiChannel, i) == channel))
                setBlinkState(i, (bool)data2);
        }
    }
}

void LEDs::setBlinkState(uint8_t ledID, bool state)
{
    uint8_t ledArray[3], leds = 0;

    if (database.read(DB_BLOCK_LEDS, dbSection_leds_rgbEnable, board.getRGBID(ledID)))
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
    if (database.read(DB_BLOCK_LEDS, dbSection_leds_rgbEnable, board.getRGBID(ledNumber)))
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

bool LEDs::setFadeTime(uint8_t transitionSpeed)
{
    if (transitionSpeed > FADE_TIME_MAX)
    {
        return false;
    }

    //reset transition counter
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            transitionCounter[i] = 0;

        pwmSteps = transitionSpeed;
    }

    return true;
}

bool LEDs::setBlinkTime(uint16_t blinkTime)
{
    if ((blinkTime < BLINK_TIME_MIN_INT) || (blinkTime > BLINK_TIME_MAX_INT))
    {
        return false;
    }

    ledBlinkTime = blinkTime;
    lastLEDblinkUpdateTime = 0;

    return true;
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
