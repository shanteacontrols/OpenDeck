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
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"

using namespace IO;

void LEDs::init(bool startUp)
{
    if (startUp)
    {
        if (_database.read(Database::Section::leds_t::global, static_cast<uint16_t>(setting_t::useStartupAnimation)))
            startUpAnimation();

#ifdef LED_FADING
        setFadeSpeed(_database.read(Database::Section::leds_t::global, static_cast<uint16_t>(setting_t::fadeSpeed)));
#endif
    }

    setBlinkType(static_cast<blinkType_t>(_database.read(Database::Section::leds_t::global, static_cast<uint16_t>(setting_t::blinkWithMIDIclock))));

    for (size_t i = 0; i < TOTAL_BLINK_SPEEDS; i++)
        _blinkState[i] = true;

    for (size_t i = 0; i < MAX_LEDS; i++)
        _brightness[i] = brightness_t::bOff;
}

void LEDs::checkBlinking(bool forceChange)
{
    if (_blinkResetArrayPtr == nullptr)
        return;

    switch (_ledBlinkType)
    {
    case blinkType_t::timer:
        //update blink states every 100ms - minimum blink time
        if ((core::timing::currentRunTimeMs() - _lastLEDblinkUpdateTime) < 100)
            return;

        _lastLEDblinkUpdateTime = core::timing::currentRunTimeMs();
        break;

    case blinkType_t::midiClock:
        if (!forceChange)
            return;
        break;

    default:
        return;
    }

    //change the blink state for specific blink rate
    for (size_t i = 0; i < TOTAL_BLINK_SPEEDS; i++)
    {
        if (++_blinkCounter[i] < _blinkResetArrayPtr[i])
            continue;

        _blinkState[i]   = !_blinkState[i];
        _blinkCounter[i] = 0;

        //assign changed state to all leds which have this speed
        for (size_t j = 0; j < MAX_LEDS; j++)
        {
            if (!bit(j, ledBit_t::blinkOn))
                continue;

            if (_blinkTimer[j] != i)
                continue;

            updateBit(j, ledBit_t::state, _blinkState[i]);
            _hwa.setState(j, bit(j, ledBit_t::state) ? _brightness[j] : brightness_t::bOff);
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
        _hwa.setState(i, brightness_t::bOff);
        core::timing::waitMs(35);
    }

    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
    {
        _hwa.setState(MAX_NUMBER_OF_LEDS - 1 - i, brightness_t::b100);
        core::timing::waitMs(35);
    }

    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
    {
        _hwa.setState(i, brightness_t::bOff);
        core::timing::waitMs(35);
    }

    //turn all off again
    setAllOff();
#ifdef LED_FADING
    setFadeSpeed(_database.read(Database::Section::leds_t::global, static_cast<uint16_t>(setting_t::fadeSpeed)));
#endif
}

LEDs::color_t LEDs::valueToColor(uint8_t value)
{
    //there are 7 total colors (+ off)
    return static_cast<color_t>(value / 16);
}

LEDs::blinkSpeed_t LEDs::valueToBlinkSpeed(uint8_t value)
{
    if (value < 16)
        return blinkSpeed_t::noBlink;

    //there are 4 total blink speeds
    return static_cast<blinkSpeed_t>(value % 16 / TOTAL_BLINK_SPEEDS);
}

LEDs::brightness_t LEDs::valueToBrightness(uint8_t value)
{
    if (value < 16)
        return brightness_t::bOff;

    return static_cast<brightness_t>((value % 16 % TOTAL_BRIGHTNESS_VALUES) + 1);
}

void LEDs::midiToState(MIDI::messageType_t messageType, uint8_t data1, uint8_t data2, uint8_t channel, dataSource_t dataSource)
{
    for (size_t i = 0; i < MAX_LEDS; i++)
    {
        //no point in checking if channel doesn't match
        if (_database.read(Database::Section::leds_t::midiChannel, i) != channel)
            continue;

        bool setState = false;
        bool setBlink = false;

        auto controlType = static_cast<controlType_t>(_database.read(Database::Section::leds_t::controlType, i));

        //determine whether led state or blink state should be changed
        //received MIDI message must match with defined control type
        if (dataSource == dataSource_t::internal)
        {
            switch (controlType)
            {
            case controlType_t::localNoteSingleVal:
            {
                if ((messageType == MIDI::messageType_t::noteOn) || (messageType == MIDI::messageType_t::noteOff))
                    setState = true;
            }
            break;

            case controlType_t::localCCSingleVal:
            {
                if (messageType == MIDI::messageType_t::controlChange)
                    setState = true;
            }
            break;

            //set state for program change control type regardless of local/midi in setting
            case controlType_t::midiInPCSingleVal:
            case controlType_t::localPCSingleVal:
            {
                if (messageType == MIDI::messageType_t::programChange)
                    setState = true;
            }
            break;

            case controlType_t::localNoteMultiVal:
            {
                if ((messageType == MIDI::messageType_t::noteOn) || (messageType == MIDI::messageType_t::noteOff))
                {
                    setState = true;
                    setBlink = true;
                }
            }
            break;

            case controlType_t::localCCMultiVal:
            {
                if (messageType == MIDI::messageType_t::controlChange)
                {
                    setState = true;
                    setBlink = true;
                }
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
            case controlType_t::midiInNoteSingleVal:
            {
                if ((messageType == MIDI::messageType_t::noteOn) || (messageType == MIDI::messageType_t::noteOff))
                    setState = true;
            }
            break;

            case controlType_t::midiInCCSingleVal:
            {
                if (messageType == MIDI::messageType_t::controlChange)
                    setState = true;
            }
            break;

            //set state for program change control type regardless of local/midi in setting
            case controlType_t::midiInPCSingleVal:
            case controlType_t::localPCSingleVal:
            {
                if (messageType == MIDI::messageType_t::programChange)
                    setState = true;
            }
            break;

            case controlType_t::midiInNoteMultiVal:
            {
                if ((messageType == MIDI::messageType_t::noteOn) || (messageType == MIDI::messageType_t::noteOff))
                {
                    setState = true;
                    setBlink = true;
                }
            }
            break;

            case controlType_t::midiInCCMultiVal:
            {
                if (messageType == MIDI::messageType_t::controlChange)
                {
                    setState = true;
                    setBlink = true;
                }
            }
            break;

            default:
                break;
            }
        }

        auto color      = color_t::off;
        auto brightness = brightness_t::bOff;

        //in single value modes, brightness and blink speed cannot be controlled since we're dealing
        //with one value only

        if (setState)
        {
            //match activation ID with received ID
            if (_database.read(Database::Section::leds_t::activationID, i) == data1)
            {
                if (messageType == MIDI::messageType_t::programChange)
                {
                    //byte2 doesn't exist on program change message
                    color      = color_t::red;
                    brightness = brightness_t::b100;
                }
                else
                {
                    //use data2 value (note velocity / cc value) to set led color
                    //and possibly blink speed (depending on configuration)
                    //when note/cc are used to control both state and blinking ignore activation velocity
                    if (setState && setBlink)
                    {
                        color      = valueToColor(data2);
                        brightness = valueToBrightness(data2);
                    }
                    else
                    {
                        //this has side effect that it will always set RGB LED to red color since no color information is available
                        color      = (_database.read(Database::Section::leds_t::activationValue, i) == data2) ? color_t::red : color_t::off;
                        brightness = brightness_t::b100;
                    }
                }

                setColor(i, color, brightness);
            }
        }

        if (setBlink)
        {
            //match activation ID with received ID
            if (_database.read(Database::Section::leds_t::activationID, i) == data1)
            {
                //blink speed depends on data2 value
                setBlinkSpeed(i, valueToBlinkSpeed(data2));
            }
        }
    }
}

void LEDs::setBlinkSpeed(uint8_t ledID, blinkSpeed_t state)
{
    uint8_t ledArray[3] = {};
    uint8_t leds        = 0;
    uint8_t rgbIndex    = _hwa.rgbIndex(ledID);

    if (_database.read(Database::Section::leds_t::rgbEnable, rgbIndex))
    {
        ledArray[0] = _hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::r);
        ledArray[1] = _hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::g);
        ledArray[2] = _hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::b);

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

        _hwa.setState(ledArray[i], _brightness[ledArray[i]]);
        _blinkTimer[ledID] = static_cast<uint8_t>(state);
    }
}

void LEDs::setAllOn()
{
    //turn on all LEDs
    for (size_t i = 0; i < MAX_LEDS; i++)
        setColor(i, color_t::red, brightness_t::b100);
}

void LEDs::setAllOff()
{
    //turn off all LEDs
    for (size_t i = 0; i < MAX_LEDS; i++)
        resetState(i);
}

void LEDs::refresh()
{
    for (size_t i = 0; i < MAX_LEDS; i++)
        _hwa.setState(i, _brightness[i]);
}

void LEDs::setColor(uint8_t ledID, color_t color, brightness_t brightness)
{
    uint8_t rgbIndex = _hwa.rgbIndex(ledID);

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

            _brightness[index] = brightness;
            _hwa.setState(index, brightness);
        }
        else
        {
            //turn off the led
            resetState(index);
        }
    };

    if (_database.read(Database::Section::leds_t::rgbEnable, rgbIndex))
    {
        //rgb led is composed of three standard LEDs
        //get indexes of individual LEDs first
        uint8_t rLED = _hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::r);
        uint8_t gLED = _hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::g);
        uint8_t bLED = _hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::b);

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
    return static_cast<blinkSpeed_t>(_blinkTimer[ledID]);
}

size_t LEDs::rgbSingleComponentIndex(size_t rgbIndex, LEDs::rgbIndex_t rgbComponent)
{
    return _hwa.rgbSingleComponentIndex(rgbIndex, rgbComponent);
}

size_t LEDs::rgbIndex(size_t singleLEDindex)
{
    return _hwa.rgbIndex(singleLEDindex);
}

bool LEDs::setFadeSpeed(uint8_t transitionSpeed)
{
    if ((transitionSpeed >= FADE_TIME_MIN) && (transitionSpeed <= FADE_TIME_MAX))
    {
        _hwa.setFadeSpeed(transitionSpeed);
        return true;
    }

    return false;
}

void LEDs::setBlinkType(blinkType_t blinkType)
{
    switch (blinkType)
    {
    case blinkType_t::timer:
        _blinkResetArrayPtr = _blinkReset_timer;
        break;

    case blinkType_t::midiClock:
        _blinkResetArrayPtr = _blinkReset_midiClock;
        break;

    default:
        return;
    }

    _ledBlinkType = blinkType;
}

void LEDs::resetBlinking()
{
    //reset all counters in this case
    //also make sure all leds are in sync again
    for (size_t i = 0; i < TOTAL_BLINK_SPEEDS; i++)
    {
        _blinkCounter[i] = 0;
        _blinkState[i]   = true;
    }
}

void LEDs::updateBit(uint8_t index, ledBit_t bit, bool state)
{
    BIT_WRITE(_ledState[index], static_cast<uint8_t>(bit), state);
}

bool LEDs::bit(uint8_t index, ledBit_t bit)
{
    return BIT_READ(_ledState[index], static_cast<uint8_t>(bit));
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
            uint8_t rgbIndex = _hwa.rgbIndex(ledID);

            uint8_t color = 0;
            color |= bit(_hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::b), ledBit_t::rgb_b);
            color <<= 1;
            color |= bit(_hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::g), ledBit_t::rgb_g);
            color <<= 1;
            color |= bit(_hwa.rgbSingleComponentIndex(rgbIndex, rgbIndex_t::r), ledBit_t::rgb_r);

            return static_cast<color_t>(color);
        }
    }
}

void LEDs::resetState(uint8_t index)
{
    _ledState[index]   = 0;
    _brightness[index] = brightness_t::bOff;
    _hwa.setState(index, brightness_t::bOff);
}