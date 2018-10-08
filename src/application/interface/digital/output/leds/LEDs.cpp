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
#include "board/common/digital/output/Variables.h"
#include "core/src/general/Timing.h"
#include "core/src/general/BitManipulation.h"

void LEDs::init(bool startUp)
{
    if (startUp)
    {
        if (database.read(DB_BLOCK_LEDS, dbSection_leds_global, ledGlobalParam_startUpRoutineState))
        {
            //set to slowest fading speed for effect
            #ifdef LED_FADING_SUPPORTED
            setFadeTime(1);
            #endif

            if (board.startUpAnimation != nullptr)
                board.startUpAnimation();
            else
                startUpAnimation();
        }

        #ifdef LED_FADING_SUPPORTED
        setFadeTime(database.read(DB_BLOCK_LEDS, dbSection_leds_global, ledGlobalParam_fadeSpeed));
        #endif
    }

    setBlinkType((blinkType_t)database.read(DB_BLOCK_LEDS, dbSection_leds_global, ledGlobalParam_blinkMIDIclock));

    for (int i=0; i<BLINK_SPEEDS; i++)
        blinkState[i] = true;
}

void LEDs::checkBlinking(bool forceChange)
{
    //holds last time in miliseconds when LED blinking has been updated
    static uint32_t lastLEDblinkUpdateTime = 0;

    switch(ledBlinkType)
    {
        case blinkType_timer:
        //update blink states every 100ms - minimum blink time
        if ((rTimeMs() - lastLEDblinkUpdateTime) < 100)
            return;

        lastLEDblinkUpdateTime = rTimeMs();
        break;

        case blinkType_midiClock:
        if (!forceChange)
            return;
        break;

        default:
        return;
    }

    //change the blink state for specific blink rate
    for (int i=0; i<BLINK_SPEEDS; i++)
    {
        if (++blinkCounter[i] < blinkResetArrayPtr[i])
            continue;

        blinkState[i] = !blinkState[i];
        blinkCounter[i] = 0;

        //assign changed state to all leds which have this speed
        for (int j=0; j<MAX_NUMBER_OF_LEDS; j++)
        {
            if (!BIT_READ(ledState[j], LED_BLINK_ON_BIT))
                continue;

            if (blinkTimer[j] != i)
                continue;

            BIT_WRITE(ledState[j], LED_STATE_BIT, blinkState[i]);
        }
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
    setFadeTime(database.read(DB_BLOCK_LEDS, dbSection_leds_global, ledGlobalParam_fadeSpeed));
    #endif
}

ledColor_t LEDs::valueToColor(uint8_t value)
{
    /*
        MIDI value  Color       Color index
        0-15        Off         0
        16-31       Red         1
        32-47       Green       2
        48-63       Yellow      3
        64-79       Blue        4
        80-95       Magenta     5
        96-111      Cyan        6
        112-127     White       7
    */

    return (ledColor_t)(value/16);
}

blinkSpeed_t LEDs::valueToBlinkSpeed(uint8_t value)
{
    /*
        MIDI value  Blink speed  Blink speed index
        0-9        0/disabled    0
        10-19      100ms         1
        20-29      200ms         2
        30-39      300ms         3
        40-49      400ms         4
        50-59      500ms         5
        60-69      600ms         6
        70-79      700ms         7
        80-89      800ms         8
        90-99      900ms         9
        100-109    1000ms        10
        110-127    1000ms        11
    */

    if (value >= 120)
        value = 119;

    return (blinkSpeed_t)(value/10);
}

void LEDs::midiToState(midiMessageType_t messageType, uint8_t data1, uint8_t data2, uint8_t channel, bool local)
{
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        //no point in checking if channel doesn't match
        if (database.read(DB_BLOCK_LEDS, dbSection_leds_midiChannel, i) != channel)
            continue;

        bool setState = false;
        bool setBlink = false;

        ledControlType_t controlType = (ledControlType_t)database.read(DB_BLOCK_LEDS, dbSection_leds_controlType, i);

        //determine whether led state or blink state should be changed
        //received MIDI message must match with defined control type
        if (local)
        {
            switch(controlType)
            {
                case ledControlLocal_NoteStateOnly:
                if ((messageType == midiMessageNoteOn) || (messageType == midiMessageNoteOff))
                    setState = true;
                break;

                case ledControlLocal_CCStateOnly:
                if (messageType == midiMessageControlChange)
                    setState = true;
                break;

                case ledControlLocal_PCStateOnly:
                if (messageType == midiMessageProgramChange)
                    setState = true;
                break;

                case ledControlMIDIin_noteStateBlink:
                if ((messageType == midiMessageNoteOn) || (messageType == midiMessageNoteOff))
                {
                    setState = true;
                    setBlink = true;
                }
                break;

                case ledControlMIDIin_CCStateBlink:
                if (messageType == midiMessageControlChange)
                {
                    setState = true;
                    setBlink = true;
                }
                break;

                default:
                break;
            }
        }
        else
        {
            switch(controlType)
            {
                case ledControlMIDIin_noteStateCCblink:
                if ((messageType == midiMessageNoteOn) || (messageType == midiMessageNoteOff))
                    setState = true;
                else if (messageType == midiMessageControlChange)
                    setBlink = true;
                break;

                case ledControlMIDIin_CCstateNoteBlink:
                if ((messageType == midiMessageNoteOn) || (messageType == midiMessageNoteOff))
                    setBlink = true;
                else if (messageType == midiMessageControlChange)
                    setState = true;
                break;

                case ledControlMIDIin_noteStateBlink:
                if ((messageType == midiMessageNoteOn) || (messageType == midiMessageNoteOff))
                {
                    setState = true;
                    setBlink = true;
                }
                break;

                case ledControlMIDIin_CCStateBlink:
                if (messageType == midiMessageControlChange)
                {
                    setState = true;
                    setBlink = true;
                }
                break;

                case ledControlMIDIin_PCStateOnly:
                if (messageType == midiMessageProgramChange)
                    setState = true;
                break;

                default:
                break;
            }
        }

        ledColor_t color = colorOff;
        bool rgbEnabled = database.read(DB_BLOCK_LEDS, dbSection_leds_rgbEnable, board.getRGBID(i));

        if (setState)
        {
            //match LED activation ID with received ID
            if (database.read(DB_BLOCK_LEDS, dbSection_leds_activationID, i) == data1)
            {
                if (messageType == midiMessageProgramChange)
                {
                    //byte2 doesn't exist on program change message
                    //color depends on data1 if rgb led is enabled
                    //otherwise just turn the led on - no activation value check
                    if (rgbEnabled)
                        color = valueToColor(data1);
                    else
                        color = colorRed; //any color is fine on single-color led
                }
                else
                {
                    //use data2 value (note velocity / cc value) to set led color
                    //and possibly blink speed (depending on configuration)
                    //when note/cc are used to control both state and blinking ignore activation velocity
                    if (rgbEnabled || (setState && setBlink))
                    {
                        color = valueToColor(data2);
                    }
                    else
                    {
                        color = (database.read(DB_BLOCK_LEDS, dbSection_leds_activationValue, i) == data2) ? colorRed : colorOff;
                    }
                }

                setColor(i, color);
            }
            else if (messageType == midiMessageProgramChange)
            {
                //when ID doesn't match and control type is program change, make sure to turn the led off
                color = colorOff;
                setColor(i, color);
            }
        }

        if (setBlink)
        {
            //match activation ID with received ID
            if (database.read(DB_BLOCK_LEDS, dbSection_leds_activationID, i) == data1)
            {
                if (setState)
                {
                    uint8_t blinkSpeed = (blinkSpeed_t)blinkSpeed_noBlink;

                    if (data2 && (bool)color)
                    {
                        //single message is being used to set both state and blink value
                        //first reduce data2 to range 0-15
                        //append 1 so that first value is blinking one
                        //turn off blinking only on higher range
                        blinkSpeed = 1 + (data2 - ((uint8_t)color*16));

                        //make sure data2 is in range
                        //when it's not turn off blinking
                        if (blinkSpeed >= BLINK_SPEEDS)
                            blinkSpeed = (blinkSpeed_t)blinkSpeed_noBlink;
                    }

                    setBlinkState(i, (blinkSpeed_t)blinkSpeed);
                }
                else
                {
                    //blink speed depends on data2 value
                    setBlinkState(i, valueToBlinkSpeed(data2));
                }
            }
        }
    }
}

void LEDs::setBlinkState(uint8_t ledID, blinkSpeed_t state)
{
    uint8_t ledArray[3], leds = 0;
    uint8_t rgbIndex = board.getRGBID(ledID);

    if (database.read(DB_BLOCK_LEDS, dbSection_leds_rgbEnable, ledID))
    {
        ledArray[0] = board.getRGBaddress(rgbIndex, rgb_R);
        ledArray[1] = board.getRGBaddress(rgbIndex, rgb_G);
        ledArray[2] = board.getRGBaddress(rgbIndex, rgb_B);

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
        if ((bool)state)
        {
            BIT_SET(ledState[ledArray[i]], LED_BLINK_ON_BIT);
            //this will turn the led on immediately
            BIT_SET(ledState[ledArray[i]], LED_STATE_BIT);
        }
        else
        {
            BIT_CLEAR(ledState[ledArray[i]], LED_BLINK_ON_BIT);
            BIT_WRITE(ledState[ledArray[i]], LED_STATE_BIT, BIT_READ(ledState[ledArray[i]], LED_ACTIVE_BIT));
        }

        blinkTimer[ledID] = (uint8_t)state;
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

void LEDs::setColor(uint8_t ledID, ledColor_t color)
{
    uint8_t rgbIndex = board.getRGBID(ledID);

    if (database.read(DB_BLOCK_LEDS, dbSection_leds_rgbEnable, ledID))
    {
        uint8_t led1 = board.getRGBaddress(rgbIndex, rgb_R);
        uint8_t led2 = board.getRGBaddress(rgbIndex, rgb_G);
        uint8_t led3 = board.getRGBaddress(rgbIndex, rgb_B);

        handleLED(led1, BIT_READ(color, 0));
        handleLED(led2, BIT_READ(color, 1));
        handleLED(led3, BIT_READ(color, 2));
    }
    else
    {
        handleLED(ledID, (bool)color);
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
            uint8_t rgbIndex = board.getRGBID(ledID);
            uint8_t led1 = getState(board.getRGBaddress(rgbIndex, rgb_R));
            uint8_t led2 = getState(board.getRGBaddress(rgbIndex, rgb_G));
            uint8_t led3 = getState(board.getRGBaddress(rgbIndex, rgb_B));

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

uint8_t LEDs::getState(uint8_t ledID)
{
    return ledState[ledID];
}

bool LEDs::getBlinkState(uint8_t ledID)
{
    return BIT_READ(ledState[ledID], LED_BLINK_ON_BIT);
}

bool LEDs::setFadeTime(uint8_t transitionSpeed)
{
    return board.setLEDfadeSpeed(transitionSpeed);
}

void LEDs::handleLED(uint8_t ledID, bool state, bool rgbLED, rgbIndex_t index)
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

    uint8_t currentState = getState(ledID);

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

    ledState[ledID] = currentState;
}

void LEDs::setBlinkType(blinkType_t blinkType)
{
    switch(blinkType)
    {
        case blinkType_timer:
        blinkResetArrayPtr = blinkReset_timer;
        break;

        case blinkType_midiClock:
        blinkResetArrayPtr = blinkReset_midiClock;
        break;

        default:
        return;
    }

    ledBlinkType = blinkType;
}

blinkType_t LEDs::getBlinkType()
{
    return ledBlinkType;
}

void LEDs::resetBlinking()
{
    //reset all counters in this case
    //also make sure all leds are in sync again
    for (int i=0; i<BLINK_SPEEDS; i++)
    {
        blinkCounter[i] = 0;
        blinkState[i] = true;
    }
}