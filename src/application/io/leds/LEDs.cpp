/*

Copyright 2015-2021 Igor Petrovic

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

#include "LEDs.h"
#include "Constants.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"

using namespace IO;

void LEDs::init(bool startUp)
{
    if (startUp)
    {
        if (database.read(Database::Section::leds_t::global, static_cast<uint16_t>(setting_t::useStartupAnimation)))
            startUpAnimation();

#ifdef LED_FADING
        setFadeSpeed(database.read(Database::Section::leds_t::global, static_cast<uint16_t>(setting_t::fadeSpeed)));
#endif
    }

    setBlinkType(static_cast<blinkType_t>(database.read(Database::Section::leds_t::global, static_cast<uint16_t>(setting_t::blinkWithMIDIclock))));

    for (size_t i = 0; i < totalBlinkSpeeds; i++)
        blinkState[i] = true;

    for (size_t i = 0; i < maxLEDs; i++)
        brightness[i] = brightness_t::bOff;
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
    for (size_t i = 0; i < totalBlinkSpeeds; i++)
    {
        if (++blinkCounter[i] < blinkResetArrayPtr[i])
            continue;

        blinkState[i]   = !blinkState[i];
        blinkCounter[i] = 0;

        //assign changed state to all leds which have this speed
        for (size_t j = 0; j < maxLEDs; j++)
        {
            if (!bit(j, ledBit_t::blinkOn))
                continue;

            if (blinkTimer[j] != i)
                continue;

            updateBit(j, ledBit_t::state, blinkState[i]);
            hwa.setState(j, bit(j, ledBit_t::state) ? brightness[j] : brightness_t::bOff);
        }
    }
}

__attribute__((weak)) void LEDs::startUpAnimation()
{
#ifdef LED_FADING
    setFadeSpeed(1);
#endif
    //turn all leds on first
    setAllOn();

    core::timing::waitMs(1000);

    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
    {
        hwa.setState(i, brightness_t::bOff);
        core::timing::waitMs(35);
    }

    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
    {
        hwa.setState(MAX_NUMBER_OF_LEDS - 1 - i, brightness_t::b100);
        core::timing::waitMs(35);
    }

    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
    {
        hwa.setState(i, brightness_t::bOff);
        core::timing::waitMs(35);
    }

    //turn all off again
    setAllOff();
#ifdef LED_FADING
    setFadeSpeed(database.read(Database::Section::leds_t::global, static_cast<uint16_t>(setting_t::fadeSpeed)));
#endif
}

LEDs::color_t LEDs::valueToColor(uint8_t value)
{
    //there are 7 total colors (+ off)
    return static_cast<color_t>(value / 16);
}

LEDs::blinkSpeed_t LEDs::valueToBlinkSpeed(uint8_t value)
{
    //there are 4 total blink speeds
    return static_cast<blinkSpeed_t>(value % totalBlinkSpeeds);
}

LEDs::brightness_t LEDs::valueToBrightness(uint8_t value)
{
    //there are 4 total brightness options
    return static_cast<brightness_t>(value % 16 % totalBrightnessValues);
}

void LEDs::midiToState(MIDI::messageType_t messageType, uint8_t data1, uint8_t data2, uint8_t channel, bool local)
{
    for (size_t i = 0; i < maxLEDs; i++)
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

            case controlType_t::localNoteForStateAndBlink:
                if ((messageType == MIDI::messageType_t::noteOn) || (messageType == MIDI::messageType_t::noteOff))
                {
                    setState = true;
                    setBlink = true;
                }
                break;

            case controlType_t::localCCforStateAndBlink:
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
        auto brightness = brightness_t::bOff;
        bool rgbEnabled = database.read(Database::Section::leds_t::rgbEnable, hwa.rgbIndex(i));

        if (setState)
        {
            //match activation ID with received ID
            if (database.read(Database::Section::leds_t::activationID, i) == data1)
            {
                if (messageType == MIDI::messageType_t::programChange)
                {
                    //byte2 doesn't exist on program change message
                    color      = valueToColor(data1);
                    brightness = valueToBrightness(data1);
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

                    brightness = valueToBrightness(data2);
                }

                setColor(i, color, brightness);
            }
        }

        if (setBlink)
        {
            //match activation ID with received ID
            if (database.read(Database::Section::leds_t::activationID, i) == data1)
            {
                //blink speed depends on data2 value
                setBlinkSpeed(i, valueToBlinkSpeed(data2));
            }
        }
    }
}

void LEDs::setBlinkSpeed(uint8_t ledID, blinkSpeed_t state)
{
    uint8_t ledArray[3], leds = 0;
    uint8_t rgbIndex = hwa.rgbIndex(ledID);

    if (database.read(Database::Section::leds_t::rgbEnable, rgbIndex))
    {
        ledArray[0] = hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::r);
        ledArray[1] = hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::g);
        ledArray[2] = hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::b);

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
        if (state != blinkSpeed_t::noBlink)
        {
            updateBit(ledArray[i], ledBit_t::blinkOn, true);
            updateBit(ledArray[i], ledBit_t::state, true);
        }
        else
        {
            updateBit(ledArray[i], ledBit_t::blinkOn, false);
            updateBit(ledArray[i], ledBit_t::state, bit(ledArray[i], ledBit_t::active));
        }

        hwa.setState(ledArray[i], brightness[ledArray[i]]);
        blinkTimer[ledID] = static_cast<uint8_t>(state);
    }
}

void LEDs::setAllOn()
{
    //turn on all LEDs
    for (size_t i = 0; i < maxLEDs; i++)
        setColor(i, color_t::red, brightness_t::b100);
}

void LEDs::setAllOff()
{
    //turn off all LEDs
    for (size_t i = 0; i < maxLEDs; i++)
        resetState(i);
}

void LEDs::refresh()
{
    for (size_t i = 0; i < maxLEDs; i++)
        hwa.setState(i, brightness[i]);
}

void LEDs::setColor(uint8_t ledID, color_t color, brightness_t brightness)
{
    uint8_t rgbIndex = hwa.rgbIndex(ledID);

    auto handleLED = [&](uint8_t index, rgbIndex_t rgbIndex, bool state, bool isRGB) {
        if (state)
        {
            updateBit(index, ledBit_t::active, true);
            updateBit(index, ledBit_t::state, true);

            if (isRGB)
            {
                updateBit(index, ledBit_t::rgb, true);

                switch (rgbIndex)
                {
                case rgbIndex_t::r:
                    updateBit(index, ledBit_t::rgb_r, state);
                    break;

                case rgbIndex_t::g:
                    updateBit(index, ledBit_t::rgb_g, state);
                    break;

                case rgbIndex_t::b:
                    updateBit(index, ledBit_t::rgb_b, state);
                    break;
                }
            }
            else
            {
                updateBit(index, ledBit_t::rgb, false);
            }

            this->brightness[index] = brightness;
            hwa.setState(index, brightness);
        }
        else
        {
            //turn off the led
            resetState(index);
        }
    };

    if (database.read(Database::Section::leds_t::rgbEnable, rgbIndex))
    {
        //rgb led is composed of three standard LEDs
        //get indexes of individual LEDs first
        uint8_t rLED = hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::r);
        uint8_t gLED = hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::g);
        uint8_t bLED = hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::b);

        handleLED(rLED, rgbIndex_t::r, BIT_READ(static_cast<bool>(color), static_cast<uint8_t>(rgbIndex_t::r)), true);
        handleLED(gLED, rgbIndex_t::g, BIT_READ(static_cast<bool>(color), static_cast<uint8_t>(rgbIndex_t::g)), true);
        handleLED(bLED, rgbIndex_t::b, BIT_READ(static_cast<bool>(color), static_cast<uint8_t>(rgbIndex_t::b)), true);
    }
    else
    {
        handleLED(ledID, rgbIndex_t::r, static_cast<bool>(color), false);
    }
}

IO::LEDs::blinkSpeed_t LEDs::blinkSpeed(uint8_t ledID)
{
    return static_cast<blinkSpeed_t>(blinkTimer[ledID]);
}

size_t LEDs::rgbSingleComponentIndex(size_t rgbIndex, LEDs::rgbIndex_t rgbComponent)
{
    return hwa.rgbSingleComponentIndex(rgbIndex, rgbComponent);
}

size_t LEDs::rgbIndex(size_t singleLEDindex)
{
    return hwa.rgbIndex(singleLEDindex);
}

bool LEDs::setFadeSpeed(uint8_t transitionSpeed)
{
    if ((transitionSpeed >= FADE_TIME_MIN) && (transitionSpeed <= FADE_TIME_MAX))
    {
        hwa.setFadeSpeed(transitionSpeed);
        return true;
    }

    return false;
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

void LEDs::resetBlinking()
{
    //reset all counters in this case
    //also make sure all leds are in sync again
    for (size_t i = 0; i < totalBlinkSpeeds; i++)
    {
        blinkCounter[i] = 0;
        blinkState[i]   = true;
    }
}

void LEDs::updateBit(uint8_t index, ledBit_t bit, bool state)
{
    BIT_WRITE(ledState[index], static_cast<uint8_t>(bit), state);
}

bool LEDs::bit(uint8_t index, ledBit_t bit)
{
    return BIT_READ(ledState[index], static_cast<uint8_t>(bit));
}

LEDs::color_t LEDs::color(uint8_t ledID)
{
    if (!bit(ledID, ledBit_t::active))
    {
        return color_t::off;
    }
    else
    {
        if (!bit(ledID, ledBit_t::rgb))
        {
            //single color led
            return color_t::red;
        }
        else
        {
            //rgb led
            uint8_t rgbIndex = hwa.rgbIndex(ledID);

            uint8_t color = 0;
            color |= bit(hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::b), ledBit_t::rgb_b);
            color <<= 1;
            color |= bit(hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::g), ledBit_t::rgb_g);
            color <<= 1;
            color |= bit(hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::r), ledBit_t::rgb_r);

            return static_cast<color_t>(color);
        }
    }
}

void LEDs::resetState(uint8_t index)
{
    ledState[index]   = 0;
    brightness[index] = brightness_t::bOff;
    hwa.setState(index, brightness_t::bOff);
}