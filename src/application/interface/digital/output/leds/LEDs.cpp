/*

Copyright 2015-2020 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifdef LEDS_SUPPORTED

#include "LEDs.h"
#include "board/Board.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"

using namespace Interface::digital::output;

#define LED_ON_MASK   ((0x01 << static_cast<uint8_t>(ledBit_t::active)) | (0x01 << static_cast<uint8_t>(ledBit_t::state)))
#define LED_ON(state) ((state & LED_ON_MASK) == LED_ON_MASK)

void LEDs::init(bool startUp)
{
    if (startUp)
    {
        if (database.read(Database::Section::leds_t::global, static_cast<uint16_t>(setting_t::useStartupAnimation)))
            startUpAnimation();

#ifdef LED_FADING
        setFadeTime(database.read(Database::Section::leds_t::global, static_cast<uint16_t>(setting_t::fadeSpeed)));
#endif
    }

    setBlinkType(static_cast<blinkType_t>(database.read(Database::Section::leds_t::global, static_cast<uint16_t>(setting_t::blinkWithMIDIclock))));

    for (int i = 0; i < static_cast<uint8_t>(blinkSpeed_t::AMOUNT); i++)
        blinkState[i] = true;
}

void LEDs::checkBlinking(bool forceChange)
{
    if (blinkResetArrayPtr == nullptr)
        return;

    switch (ledBlinkType)
    {
    case blinkType_t::timer:
        //update blink states every 100ms - minimum blink time
        if ((core::timing::currentRunTimeMs() - lastLEDblinkUpdateTime) < 100)
            return;

        lastLEDblinkUpdateTime = core::timing::currentRunTimeMs();
        break;

    case blinkType_t::midiClock:
        if (!forceChange)
            return;
        break;

    default:
        return;
    }

    //change the blink state for specific blink rate
    for (int i = 0; i < static_cast<uint8_t>(blinkSpeed_t::AMOUNT); i++)
    {
        if (++blinkCounter[i] < blinkResetArrayPtr[i])
            continue;

        blinkState[i]   = !blinkState[i];
        blinkCounter[i] = 0;

        //assign changed state to all leds which have this speed
        for (int j = 0; j < MAX_NUMBER_OF_LEDS; j++)
        {
            if (!getState(j, ledBit_t::blinkOn))
                continue;

            if (blinkTimer[j] != i)
                continue;

            updateState(j, ledBit_t::state, blinkState[i]);
        }
    }
}

__attribute__((weak)) void LEDs::startUpAnimation()
{
#ifdef LED_FADING
    setFadeTime(1);
#endif
    setAllOn();
    core::timing::waitMs(2000);
    setAllOff();
    core::timing::waitMs(2000);
#ifdef LED_FADING
    setFadeTime(database.read(Database::Section::leds_t::global, static_cast<uint16_t>(setting_t::fadeSpeed)));
#endif
}

LEDs::color_t LEDs::valueToColor(uint8_t value)
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

    return (color_t)(value / 16);
}

LEDs::blinkSpeed_t LEDs::valueToBlinkSpeed(uint8_t value)
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

    uint8_t blinkValue = value / static_cast<uint8_t>(blinkSpeed_t::AMOUNT);

    //in case of overflow return highest allowed value
    if (blinkValue >= static_cast<uint8_t>(blinkSpeed_t::AMOUNT))
        return static_cast<blinkSpeed_t>(static_cast<uint8_t>(blinkSpeed_t::AMOUNT) - 1);
    else
        return static_cast<blinkSpeed_t>(blinkValue);
}

void LEDs::midiToState(MIDI::messageType_t messageType, uint8_t data1, uint8_t data2, uint8_t channel, bool local)
{
    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
    {
        //no point in checking if channel doesn't match
        if (database.read(Database::Section::leds_t::midiChannel, i) != channel)
            continue;

        bool setState = false;
        bool setBlink = false;

        auto controlType = static_cast<controlType_t>(database.read(Database::Section::leds_t::controlType, i));

        //determine whether led state or blink state should be changed
        //received MIDI message must match with defined control type
        if (local)
        {
            switch (controlType)
            {
            case controlType_t::localNoteForStateNoBlink:
                if ((messageType == MIDI::messageType_t::noteOn) || (messageType == MIDI::messageType_t::noteOff))
                    setState = true;
                break;

            case controlType_t::localCCforStateNoBlink:
                if (messageType == MIDI::messageType_t::controlChange)
                    setState = true;
                break;

            //set state for program change control type regardless of local/midi in setting
            case controlType_t::midiInPCforStateNoBlink:
            case controlType_t::localPCforStateNoBlink:
                if (messageType == MIDI::messageType_t::programChange)
                    setState = true;
                break;

            case controlType_t::midiInNoteForStateAndBlink:
                if ((messageType == MIDI::messageType_t::noteOn) || (messageType == MIDI::messageType_t::noteOff))
                {
                    setState = true;
                    setBlink = true;
                }
                break;

            case controlType_t::midiInCCforStateAndBlink:
                if (messageType == MIDI::messageType_t::controlChange)
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
            switch (controlType)
            {
            case controlType_t::midiInNoteForStateCCforBlink:
                if ((messageType == MIDI::messageType_t::noteOn) || (messageType == MIDI::messageType_t::noteOff))
                    setState = true;
                else if (messageType == MIDI::messageType_t::controlChange)
                    setBlink = true;
                break;

            case controlType_t::midiInCCforStateNoteForBlink:
                if ((messageType == MIDI::messageType_t::noteOn) || (messageType == MIDI::messageType_t::noteOff))
                    setBlink = true;
                else if (messageType == MIDI::messageType_t::controlChange)
                    setState = true;
                break;

            case controlType_t::midiInNoteForStateAndBlink:
                if ((messageType == MIDI::messageType_t::noteOn) || (messageType == MIDI::messageType_t::noteOff))
                {
                    setState = true;
                    setBlink = true;
                }
                break;

            case controlType_t::midiInCCforStateAndBlink:
                if (messageType == MIDI::messageType_t::controlChange)
                {
                    setState = true;
                    setBlink = true;
                }
                break;

            //set state for program change control type regardless of local/midi in setting
            case controlType_t::midiInPCforStateNoBlink:
            case controlType_t::localPCforStateNoBlink:
                if (messageType == MIDI::messageType_t::programChange)
                    setState = true;
                break;

            default:
                break;
            }
        }

        auto color      = color_t::off;
        bool rgbEnabled = database.read(Database::Section::leds_t::rgbEnable, Board::io::getRGBID(i));

        if (setState)
        {
            //match activation ID with received ID
            if (database.read(Database::Section::leds_t::activationID, i) == data1)
            {
                if (messageType == MIDI::messageType_t::programChange)
                {
                    //byte2 doesn't exist on program change message
                    //color depends on data1 if rgb led is enabled
                    //otherwise just turn the led on - no activation value check
                    if (rgbEnabled)
                        color = valueToColor(data1);
                    else
                        color = color_t::red;    //any color is fine on single-color led
                }
                else
                {
                    //use data2 value (note velocity / cc value) to set led color
                    //and possibly blink speed (depending on configuration)
                    //when note/cc are used to control both state and blinking ignore activation velocity
                    if (rgbEnabled || (setState && setBlink))
                        color = valueToColor(data2);
                    else
                        color = (database.read(Database::Section::leds_t::activationValue, i) == data2) ? color_t::red : color_t::off;
                }

                setColor(i, color);
            }
            else if (messageType == MIDI::messageType_t::programChange)
            {
                //when ID doesn't match and control type is program change, make sure to turn the led off
                color = color_t::off;
                setColor(i, color);
            }
        }

        if (setBlink)
        {
            //match activation ID with received ID
            if (database.read(Database::Section::leds_t::activationID, i) == data1)
            {
                if (setState)
                {
                    auto blinkSpeed = static_cast<uint8_t>(blinkSpeed_t::noBlink);

                    if (data2 && static_cast<bool>(color))
                    {
                        //single message is being used to set both state and blink value
                        //first reduce data2 to range 0-15
                        //append 1 so that first value is blinking one
                        //turn off blinking only on higher range
                        blinkSpeed = 1 + (data2 - ((static_cast<uint8_t>(color) * 16)));

                        //make sure data2 is in range
                        //when it's not turn off blinking
                        if (blinkSpeed >= static_cast<uint8_t>(blinkSpeed_t::AMOUNT))
                            blinkSpeed = static_cast<uint8_t>(blinkSpeed_t::noBlink);
                    }

                    setBlinkState(i, static_cast<blinkSpeed_t>(blinkSpeed));
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
    uint8_t rgbIndex = Board::io::getRGBID(ledID);

    if (database.read(Database::Section::leds_t::rgbEnable, rgbIndex))
    {
        ledArray[0] = Board::io::getRGBaddress(rgbIndex, rgbIndex_t::r);
        ledArray[1] = Board::io::getRGBaddress(rgbIndex, rgbIndex_t::g);
        ledArray[2] = Board::io::getRGBaddress(rgbIndex, rgbIndex_t::b);

        leds = 3;
    }
    else
    {
        ledArray[0] = ledID;
        ledArray[1] = ledID;
        ledArray[2] = ledID;

        leds = 1;
    }

    for (int i = 0; i < leds; i++)
    {
        if (static_cast<bool>(state))
        {
            updateState(ledArray[i], ledBit_t::blinkOn, true, false);
            //this will turn the led on immediately
            updateState(ledArray[i], ledBit_t::state, true);
        }
        else
        {
            updateState(ledArray[i], ledBit_t::blinkOn, false, false);
            updateState(ledArray[i], ledBit_t::state, getState(ledArray[i], ledBit_t::active));
        }

        blinkTimer[ledID] = static_cast<uint8_t>(state);
    }
}

void LEDs::setAllOn()
{
    //turn on all LEDs
    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
        setColor(i, color_t::red);
}

void LEDs::setAllOff()
{
    //turn off all LEDs
    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
        resetState(i);
}

void LEDs::setColor(uint8_t ledID, color_t color)
{
    uint8_t rgbIndex = Board::io::getRGBID(ledID);

    if (database.read(Database::Section::leds_t::rgbEnable, rgbIndex))
    {
        //rgb led is composed of three standard LEDs
        //get indexes of individual LEDs first
        uint8_t rLED = Board::io::getRGBaddress(rgbIndex, rgbIndex_t::r);
        uint8_t gLED = Board::io::getRGBaddress(rgbIndex, rgbIndex_t::g);
        uint8_t bLED = Board::io::getRGBaddress(rgbIndex, rgbIndex_t::b);

        handleLED(rLED, BIT_READ(static_cast<bool>(color), static_cast<uint8_t>(rgbIndex_t::r)), true, rgbIndex_t::r);
        handleLED(gLED, BIT_READ(static_cast<bool>(color), static_cast<uint8_t>(rgbIndex_t::g)), true, rgbIndex_t::g);
        handleLED(bLED, BIT_READ(static_cast<bool>(color), static_cast<uint8_t>(rgbIndex_t::b)), true, rgbIndex_t::b);
    }
    else
    {
        handleLED(ledID, (bool)color, false);
    }
}

LEDs::color_t LEDs::getColor(uint8_t ledID)
{
    if (!getState(ledID, ledBit_t::active))
    {
        return color_t::off;
    }
    else
    {
        if (!getState(ledID, ledBit_t::rgb))
        {
            //single color led
            return color_t::red;
        }
        else
        {
            //rgb led
            uint8_t rgbIndex = Board::io::getRGBID(ledID);

            uint8_t color = 0;
            color |= getState(Board::io::getRGBaddress(rgbIndex, rgbIndex_t::b), ledBit_t::rgb_b);
            color <<= 1;
            color |= getState(Board::io::getRGBaddress(rgbIndex, rgbIndex_t::g), ledBit_t::rgb_g);
            color <<= 1;
            color |= getState(Board::io::getRGBaddress(rgbIndex, rgbIndex_t::r), ledBit_t::rgb_r);

            return static_cast<color_t>(color);
        }
    }
}

bool LEDs::getBlinkState(uint8_t ledID)
{
    return getState(ledID, ledBit_t::blinkOn);
}

bool LEDs::setFadeTime(uint8_t transitionSpeed)
{
    return Board::io::setLEDfadeSpeed(transitionSpeed);
}

void LEDs::handleLED(uint8_t ledID, bool state, bool rgbLED, rgbIndex_t index)
{
    if (state)
    {
        //turn on the led
        //if led was already active, clear the on bits before setting new state
        // if (BIT_READ(ledState[ledID], static_cast<uint8_t>(ledBit_t::blinkOn)))
        //     resetState(ledID);

        updateState(ledID, ledBit_t::active, true, false);
        updateState(ledID, ledBit_t::state, true, false);

        if (rgbLED)
        {
            updateState(ledID, ledBit_t::rgb, true, false);

            switch (index)
            {
            case rgbIndex_t::r:
                updateState(ledID, ledBit_t::rgb_r, state, true);
                break;

            case rgbIndex_t::g:
                updateState(ledID, ledBit_t::rgb_g, state, true);
                break;

            case rgbIndex_t::b:
                updateState(ledID, ledBit_t::rgb_b, state, true);
                break;
            }
        }
        else
        {
            updateState(ledID, ledBit_t::rgb, false);
        }
    }
    else
    {
        //turn off the led
        resetState(ledID);
    }
}

void LEDs::setBlinkType(blinkType_t blinkType)
{
    switch (blinkType)
    {
    case blinkType_t::timer:
        blinkResetArrayPtr = blinkReset_timer;
        break;

    case blinkType_t::midiClock:
        blinkResetArrayPtr = blinkReset_midiClock;
        break;

    default:
        return;
    }

    ledBlinkType = blinkType;
}

LEDs::blinkType_t LEDs::getBlinkType()
{
    return ledBlinkType;
}

void LEDs::resetBlinking()
{
    //reset all counters in this case
    //also make sure all leds are in sync again
    for (int i = 0; i < static_cast<uint8_t>(blinkSpeed_t::AMOUNT); i++)
    {
        blinkCounter[i] = 0;
        blinkState[i]   = true;
    }
}

void LEDs::updateState(uint8_t index, ledBit_t bit, bool state, bool setOnBoard)
{
    BIT_WRITE(ledState[index], static_cast<uint8_t>(bit), state);

    if (setOnBoard)
        Board::io::writeLEDstate(index, LED_ON(ledState[index]));
}

bool LEDs::getState(uint8_t index, ledBit_t bit)
{
    return BIT_READ(ledState[index], static_cast<uint8_t>(bit));
}

void LEDs::resetState(uint8_t index)
{
    ledState[index] = 0;
    //we have just cleared all the bits - the state to write is off
    Board::io::writeLEDstate(index, false);
}

#endif