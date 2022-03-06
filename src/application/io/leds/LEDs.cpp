/*

Copyright 2015-2022 Igor Petrovic

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
#include "messaging/Messaging.h"

#ifdef LEDS_SUPPORTED

#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include "util/conversion/Conversion.h"
#include "util/configurable/Configurable.h"

using namespace IO;

LEDs::LEDs(HWA&      hwa,
           Database& database)
    : _hwa(hwa)
    , _database(database)
{
    for (size_t i = 0; i < TOTAL_BLINK_SPEEDS; i++)
    {
        _blinkState[i] = true;
    }

    for (size_t i = 0; i < Collection::size(); i++)
    {
        _brightness[i] = brightness_t::bOff;
    }

    MIDIDispatcher.listen(Messaging::eventSource_t::midiIn,
                          Messaging::listenType_t::nonFwd,
                          [this](const Messaging::event_t& event) {
                              switch (event.message)
                              {
                              case MIDI::messageType_t::noteOn:
                              case MIDI::messageType_t::noteOff:
                              case MIDI::messageType_t::controlChange:
                              case MIDI::messageType_t::programChange:
                              {
                                  midiToState(event, Messaging::eventSource_t::midiIn);
                              }
                              break;

                              case MIDI::messageType_t::sysRealTimeClock:
                              {
                                  updateAll(true);
                              }
                              break;

                              case MIDI::messageType_t::sysRealTimeStart:
                              {
                                  resetBlinking();
                                  updateAll(true);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    MIDIDispatcher.listen(Messaging::eventSource_t::buttons,
                          Messaging::listenType_t::nonFwd,
                          [this](const Messaging::event_t& event) {
                              switch (event.message)
                              {
                              case MIDI::messageType_t::noteOn:
                              case MIDI::messageType_t::noteOff:
                              case MIDI::messageType_t::controlChange:
                              case MIDI::messageType_t::programChange:
                              {
                                  midiToState(event, Messaging::eventSource_t::buttons);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    MIDIDispatcher.listen(Messaging::eventSource_t::analog,
                          Messaging::listenType_t::nonFwd,
                          [this](const Messaging::event_t& event) {
                              switch (event.message)
                              {
                              case MIDI::messageType_t::noteOn:
                              case MIDI::messageType_t::noteOff:
                              case MIDI::messageType_t::controlChange:
                              case MIDI::messageType_t::programChange:
                              {
                                  midiToState(event, Messaging::eventSource_t::analog);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    MIDIDispatcher.listen(Messaging::eventSource_t::preset,
                          Messaging::listenType_t::all,
                          [this](const Messaging::event_t& event) {
                              setAllOff();
                              midiToState(event, Messaging::eventSource_t::preset);
                          });

    MIDIDispatcher.listen(Messaging::eventSource_t::touchscreenScreen,
                          Messaging::listenType_t::all,
                          [this](const Messaging::event_t& event) {
                              refresh();
                          });

    MIDIDispatcher.listen(Messaging::eventSource_t::program,
                          Messaging::listenType_t::all,
                          [this](const Messaging::event_t& event) {
                              midiToState(event, Messaging::eventSource_t::program);
                          });

    ConfigHandler.registerConfig(
        System::Config::block_t::leds,
        // read
        [this](uint8_t section, size_t index, uint16_t& value) {
            return sysConfigGet(static_cast<System::Config::Section::leds_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value) {
            return sysConfigSet(static_cast<System::Config::Section::leds_t>(section), index, value);
        });
}

bool LEDs::init()
{
    setAllOff();

    if (_database.read(Database::Section::leds_t::global, setting_t::useStartupAnimation))
    {
        startUpAnimation();
    }

    setBlinkType(static_cast<blinkType_t>(_database.read(Database::Section::leds_t::global, setting_t::blinkWithMIDIclock)));

    return true;
}

void LEDs::updateSingle(size_t index, bool forceRefresh)
{
    // ignore index here - not applicable
    updateAll(forceRefresh);
}

void LEDs::updateAll(bool forceRefresh)
{
    if (_blinkResetArrayPtr == nullptr)
    {
        return;
    }

    switch (_ledBlinkType)
    {
    case blinkType_t::timer:
    {
        if ((core::timing::currentRunTimeMs() - _lastLEDblinkUpdateTime) < LED_BLINK_TIMER_TYPE_CHECK_TIME)
        {
            return;
        }

        _lastLEDblinkUpdateTime = core::timing::currentRunTimeMs();
    }
    break;

    case blinkType_t::midiClock:
    {
        if (!forceRefresh)
        {
            return;
        }
    }
    break;

    default:
        return;
    }

    // change the blink state for specific blink rate
    for (size_t i = 0; i < TOTAL_BLINK_SPEEDS; i++)
    {
        if (++_blinkCounter[i] < _blinkResetArrayPtr[i])
        {
            continue;
        }

        _blinkState[i]   = !_blinkState[i];
        _blinkCounter[i] = 0;

        // assign changed state to all leds which have this speed
        for (size_t j = 0; j < Collection::size(); j++)
        {
            if (!bit(j, ledBit_t::blinkOn))
            {
                continue;
            }

            if (_blinkTimer[j] != i)
            {
                continue;
            }

            updateBit(j, ledBit_t::state, _blinkState[i]);
            setState(j, bit(j, ledBit_t::state) ? _brightness[j] : brightness_t::bOff);
        }
    }
}

size_t LEDs::maxComponentUpdateIndex()
{
    return 0;
}

__attribute__((weak)) void LEDs::startUpAnimation()
{
    // turn all leds on first
    setAllOn();

    core::timing::waitMs(1000);

    for (size_t i = 0; i < Collection::size(GROUP_DIGITAL_OUTPUTS); i++)
    {
        setState(i, brightness_t::bOff);
        core::timing::waitMs(35);
    }

    for (size_t i = 0; i < Collection::size(GROUP_DIGITAL_OUTPUTS); i++)
    {
        setState(Collection::size(GROUP_DIGITAL_OUTPUTS) - 1 - i, brightness_t::b100);
        core::timing::waitMs(35);
    }

    for (size_t i = 0; i < Collection::size(GROUP_DIGITAL_OUTPUTS); i++)
    {
        setState(i, brightness_t::bOff);
        core::timing::waitMs(35);
    }

    // turn all off again
    setAllOff();
}

LEDs::color_t LEDs::valueToColor(uint8_t value)
{
    // there are 7 total colors (+ off)
    return static_cast<color_t>(value / 16);
}

LEDs::blinkSpeed_t LEDs::valueToBlinkSpeed(uint8_t value)
{
    if (value < 16)
    {
        return blinkSpeed_t::noBlink;
    }

    // there are 4 total blink speeds
    return static_cast<blinkSpeed_t>(value % 16 / TOTAL_BLINK_SPEEDS);
}

LEDs::brightness_t LEDs::valueToBrightness(uint8_t value)
{
    if (value < 16)
    {
        return brightness_t::bOff;
    }

    return static_cast<brightness_t>((value % 16 % TOTAL_BRIGHTNESS_VALUES) + 1);
}

void LEDs::midiToState(Messaging::event_t event, Messaging::eventSource_t source)
{
    for (size_t i = 0; i < Collection::size(); i++)
    {
        auto controlType = static_cast<controlType_t>(_database.read(Database::Section::leds_t::controlType, i));

        // match received midi message with the assigned LED control type
        if (!isControlTypeMatched(event.message, controlType))
        {
            continue;
        }

        bool setState     = false;
        bool setBlink     = false;
        bool checkChannel = true;

        // determine whether led state or blink state should be changed
        // received MIDI message must match with defined control type
        if (source != Messaging::eventSource_t::midiIn)
        {
            switch (controlType)
            {
            case controlType_t::localNoteSingleVal:
            {
                if ((event.message == MIDI::messageType_t::noteOn) || (event.message == MIDI::messageType_t::noteOff))
                {
                    setState = true;
                }
            }
            break;

            case controlType_t::localCCSingleVal:
            {
                if (event.message == MIDI::messageType_t::controlChange)
                {
                    setState = true;
                }
            }
            break;

            case controlType_t::pcSingleVal:
            {
                // source must be verified here, otherwise no difference between program and preset source is detected
                if ((event.message == MIDI::messageType_t::programChange) && (source != Messaging::eventSource_t::preset))
                {
                    setState = true;
                }
            }
            break;

            case controlType_t::preset:
            {
                // it is expected for MIDI message to be set to program change when changing preset
                // source must be verified here, otherwise no difference between program and preset source is detected
                if ((event.message == MIDI::messageType_t::programChange) && (source != Messaging::eventSource_t::program))
                {
                    setState = true;
                }

                checkChannel = false;
            }
            break;

            case controlType_t::localNoteMultiVal:
            {
                if ((event.message == MIDI::messageType_t::noteOn) || (event.message == MIDI::messageType_t::noteOff))
                {
                    setState = true;
                    setBlink = true;
                }
            }
            break;

            case controlType_t::localCCMultiVal:
            {
                if (event.message == MIDI::messageType_t::controlChange)
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
                if ((event.message == MIDI::messageType_t::noteOn) || (event.message == MIDI::messageType_t::noteOff))
                {
                    setState = true;
                }
            }
            break;

            case controlType_t::midiInCCSingleVal:
            {
                if (event.message == MIDI::messageType_t::controlChange)
                {
                    setState = true;
                }
            }
            break;

            case controlType_t::pcSingleVal:
            {
                if (event.message == MIDI::messageType_t::programChange)
                {
                    setState = true;
                }
            }
            break;

            case controlType_t::midiInNoteMultiVal:
            {
                if ((event.message == MIDI::messageType_t::noteOn) || (event.message == MIDI::messageType_t::noteOff))
                {
                    setState = true;
                    setBlink = true;
                }
            }
            break;

            case controlType_t::midiInCCMultiVal:
            {
                if (event.message == MIDI::messageType_t::controlChange)
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

        if (checkChannel)
        {
            // no point in further checking if channel doesn't match
            if (_database.read(Database::Section::leds_t::midiChannel, i) != event.midiChannel)
            {
                continue;
            }
        }

        if (setState || setBlink)
        {
            auto color      = color_t::off;
            auto brightness = brightness_t::bOff;

            // in single value modes, brightness and blink speed cannot be controlled since we're dealing
            // with one value only

            uint8_t activationID = _database.read(Database::Section::leds_t::activationID, i);

            if (setState)
            {
                // match activation ID with received ID
                if (activationID == event.midiIndex)
                {
                    if (event.message == MIDI::messageType_t::programChange)
                    {
                        // byte2 doesn't exist on program change message
                        color      = color_t::red;
                        brightness = brightness_t::b100;
                    }
                    else
                    {
                        // when note/cc are used to control both state and blinking ignore activation velocity
                        if (setState && setBlink)
                        {
                            color      = valueToColor(event.midiValue);
                            brightness = valueToBrightness(event.midiValue);
                        }
                        else
                        {
                            // this has side effect that it will always set RGB LED to red color since no color information is available
                            color      = (_database.read(Database::Section::leds_t::activationValue, i) == event.midiValue) ? color_t::red : color_t::off;
                            brightness = brightness_t::b100;
                        }
                    }

                    setColor(i, color, brightness);
                }
                else
                {
                    if (event.message == MIDI::messageType_t::programChange)
                    {
                        setColor(i, color_t::off, brightness_t::bOff);
                    }
                }
            }

            if (setBlink)
            {
                // match activation ID with received ID
                if (activationID == event.midiIndex)
                {
                    // if both state and blink speed should be set, then don't update the state again in setBlinkSpeed
                    setBlinkSpeed(i, valueToBlinkSpeed(event.midiValue), !(setState && setBlink));
                }
            }
        }
    }
}

void LEDs::setBlinkSpeed(uint8_t ledID, blinkSpeed_t state, bool updateState)
{
    uint8_t ledArray[3] = {};
    uint8_t leds        = 0;
    uint8_t rgbIndex    = _hwa.rgbIndex(ledID);

    if (_database.read(Database::Section::leds_t::rgbEnable, rgbIndex))
    {
        ledArray[0] = _hwa.rgbSignalIndex(rgbIndex, rgbIndex_t::r);
        ledArray[1] = _hwa.rgbSignalIndex(rgbIndex, rgbIndex_t::g);
        ledArray[2] = _hwa.rgbSignalIndex(rgbIndex, rgbIndex_t::b);

        leds = 3;
    }
    else
    {
        ledArray[0] = ledID;

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

        _blinkTimer[ledID] = static_cast<uint8_t>(state);

        if (updateState)
        {
            setState(ledArray[i], _brightness[ledArray[i]]);
        }
    }
}

void LEDs::setAllOn()
{
    // turn on all LEDs
    for (size_t i = 0; i < Collection::size(); i++)
    {
        setColor(i, color_t::red, brightness_t::b100);
    }
}

void LEDs::setAllOff()
{
    // turn off all LEDs
    for (size_t i = 0; i < Collection::size(); i++)
    {
        resetState(i);
    }
}

void LEDs::refresh()
{
    for (size_t i = 0; i < Collection::size(); i++)
    {
        setState(i, _brightness[i]);
    }
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
                {
                    updateBit(index, ledBit_t::rgb_r, state);
                }
                break;

                case rgbIndex_t::g:
                {
                    updateBit(index, ledBit_t::rgb_g, state);
                }
                break;

                case rgbIndex_t::b:
                {
                    updateBit(index, ledBit_t::rgb_b, state);
                }
                break;
                }
            }
            else
            {
                updateBit(index, ledBit_t::rgb, false);
            }

            _brightness[index] = brightness;
            setState(index, brightness);
        }
        else
        {
            // turn off the led
            resetState(index);
        }
    };

    if (_database.read(Database::Section::leds_t::rgbEnable, rgbIndex))
    {
        // rgb led is composed of three standard LEDs
        // get indexes of individual LEDs first
        uint8_t rLED = _hwa.rgbSignalIndex(rgbIndex, rgbIndex_t::r);
        uint8_t gLED = _hwa.rgbSignalIndex(rgbIndex, rgbIndex_t::g);
        uint8_t bLED = _hwa.rgbSignalIndex(rgbIndex, rgbIndex_t::b);

        handleLED(rLED, rgbIndex_t::r, BIT_READ(static_cast<uint8_t>(color), static_cast<uint8_t>(rgbIndex_t::r)), true);
        handleLED(gLED, rgbIndex_t::g, BIT_READ(static_cast<uint8_t>(color), static_cast<uint8_t>(rgbIndex_t::g)), true);
        handleLED(bLED, rgbIndex_t::b, BIT_READ(static_cast<uint8_t>(color), static_cast<uint8_t>(rgbIndex_t::b)), true);
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

size_t LEDs::rgbSignalIndex(size_t rgbIndex, LEDs::rgbIndex_t rgbComponent)
{
    return _hwa.rgbSignalIndex(rgbIndex, rgbComponent);
}

size_t LEDs::rgbIndex(size_t singleLEDindex)
{
    return _hwa.rgbIndex(singleLEDindex);
}

void LEDs::setBlinkType(blinkType_t blinkType)
{
    switch (blinkType)
    {
    case blinkType_t::timer:
    {
        _blinkResetArrayPtr = _blinkReset_timer;
    }
    break;

    case blinkType_t::midiClock:
    {
        _blinkResetArrayPtr = _blinkReset_midiClock;
    }
    break;

    default:
        return;
    }

    _ledBlinkType = blinkType;
}

void LEDs::resetBlinking()
{
    // reset all counters in this case
    // also make sure all leds are in sync again
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

    if (!bit(ledID, ledBit_t::rgb))
    {
        // single color led
        return color_t::red;
    }

    // rgb led
    uint8_t rgbIndex = _hwa.rgbIndex(ledID);

    uint8_t color = 0;
    color |= bit(_hwa.rgbSignalIndex(rgbIndex, rgbIndex_t::b), ledBit_t::rgb_b);
    color <<= 1;
    color |= bit(_hwa.rgbSignalIndex(rgbIndex, rgbIndex_t::g), ledBit_t::rgb_g);
    color <<= 1;
    color |= bit(_hwa.rgbSignalIndex(rgbIndex, rgbIndex_t::r), ledBit_t::rgb_r);

    return static_cast<color_t>(color);
}

void LEDs::resetState(uint8_t index)
{
    _ledState[index]   = 0;
    _brightness[index] = brightness_t::bOff;
    setState(index, brightness_t::bOff);
}

bool LEDs::isControlTypeMatched(MIDI::messageType_t midiMessage, controlType_t controlType)
{
    // match note off as well - only noteOn is present in the array
    if (midiMessage == MIDI::messageType_t::noteOff)
    {
        midiMessage = MIDI::messageType_t::noteOn;
    }

    return controlTypeToMIDImessage[static_cast<uint8_t>(controlType)] == midiMessage;
}

void LEDs::setState(size_t index, brightness_t brightness)
{
    if (index >= Collection::size(GROUP_DIGITAL_OUTPUTS))
    {
        // specified hwa interface only writes to physical leds
        // for touchscreen and other destinations, notify via dispatcher

        Messaging::event_t event;
        event.componentIndex = index - Collection::startIndex(GROUP_TOUCHSCREEN_COMPONENTS);
        event.midiValue      = static_cast<uint16_t>(brightness);

        MIDIDispatcher.notify(Messaging::eventSource_t::leds,
                              event,
                              Messaging::listenType_t::fwd);
    }
    else
    {
        _hwa.setState(index, brightness);
    }
}

std::optional<uint8_t> LEDs::sysConfigGet(System::Config::Section::leds_t section, size_t index, uint16_t& value)
{
    int32_t readValue;
    auto    result = System::Config::status_t::ack;

    switch (section)
    {
    case System::Config::Section::leds_t::testColor:
    {
        readValue = static_cast<int32_t>(color(index));
    }
    break;

    case System::Config::Section::leds_t::midiChannel:
    {
        result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ack : System::Config::status_t::errorRead;
    }
    break;

    case System::Config::Section::leds_t::rgbEnable:
    {
        result = _database.read(Util::Conversion::sys2DBsection(section), rgbIndex(index), readValue) ? System::Config::status_t::ack : System::Config::status_t::errorRead;
    }
    break;

    default:
    {
        result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ack : System::Config::status_t::errorRead;
    }
    break;
    }

    value = readValue;

    return result;
}

std::optional<uint8_t> LEDs::sysConfigSet(System::Config::Section::leds_t section, size_t index, uint16_t value)
{
    uint8_t result = System::Config::status_t::errorWrite;

    bool writeToDb = true;

    switch (section)
    {
    case System::Config::Section::leds_t::testColor:
    {
        // no writing to database
        setColor(index, static_cast<color_t>(value), brightness_t::b100);
        result    = System::Config::status_t::ack;
        writeToDb = false;
    }
    break;

    case System::Config::Section::leds_t::global:
    {
        auto ledSetting = static_cast<setting_t>(index);

        switch (ledSetting)
        {
        case setting_t::blinkWithMIDIclock:
        {
            if ((value <= 1) && (value >= 0))
            {
                result = System::Config::status_t::ack;
                setBlinkType(static_cast<blinkType_t>(value));
            }
        }
        break;

        case setting_t::useStartupAnimation:
        {
            if ((value <= 1) && (value >= 0))
            {
                result = System::Config::status_t::ack;
            }
        }
        break;

        case setting_t::unused:
        {
            result = System::Config::status_t::ack;
        }
        break;

        default:
            break;
        }

        // write to db if success is true and writing should take place
        if ((result == System::Config::status_t::ack) && writeToDb)
        {
            result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ack : System::Config::status_t::errorWrite;
        }
    }
    break;

    case System::Config::Section::leds_t::rgbEnable:
    {
        // make sure to turn all three leds off before setting new state
        setColor(rgbSignalIndex(rgbIndex(index), rgbIndex_t::r), color_t::off, brightness_t::bOff);
        setColor(rgbSignalIndex(rgbIndex(index), rgbIndex_t::g), color_t::off, brightness_t::bOff);
        setColor(rgbSignalIndex(rgbIndex(index), rgbIndex_t::b), color_t::off, brightness_t::bOff);

        // write rgb enabled bit to led
        result = _database.update(Util::Conversion::sys2DBsection(section), rgbIndex(index), value) ? System::Config::status_t::ack : System::Config::status_t::errorWrite;

        if (value && (result == System::Config::status_t::ack))
        {
            // copy over note activation local control and midi channel settings to all three leds from the current led index

            for (int i = 0; i < 3; i++)
            {
                result = _database.update(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::activationID),
                                          rgbSignalIndex(rgbIndex(index), static_cast<rgbIndex_t>(i)),
                                          _database.read(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::activationID), index))
                             ? System::Config::status_t::ack
                             : System::Config::status_t::errorWrite;

                if (result != System::Config::status_t::ack)
                {
                    break;
                }

                result = _database.update(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::controlType),
                                          rgbSignalIndex(rgbIndex(index), static_cast<rgbIndex_t>(i)),
                                          _database.read(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::controlType), index))
                             ? System::Config::status_t::ack
                             : System::Config::status_t::errorWrite;

                if (result != System::Config::status_t::ack)
                {
                    break;
                }

                result = _database.update(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::midiChannel),
                                          rgbSignalIndex(rgbIndex(index), static_cast<rgbIndex_t>(i)),
                                          _database.read(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::midiChannel), index))
                             ? System::Config::status_t::ack
                             : System::Config::status_t::errorWrite;

                if (result != System::Config::status_t::ack)
                {
                    break;
                }
            }
        }
    }
    break;

    case System::Config::Section::leds_t::activationID:
    case System::Config::Section::leds_t::controlType:
    case System::Config::Section::leds_t::midiChannel:
    {
        // first, find out if RGB led is enabled for this led index
        if (_database.read(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::rgbEnable), rgbIndex(index)))
        {
            // rgb led enabled - copy these settings to all three leds
            for (int i = 0; i < 3; i++)
            {
                result = _database.update(Util::Conversion::sys2DBsection(section),
                                          rgbSignalIndex(rgbIndex(index), static_cast<rgbIndex_t>(i)),
                                          value)
                             ? System::Config::status_t::ack
                             : System::Config::status_t::errorWrite;

                if (result != System::Config::status_t::ack)
                {
                    break;
                }
            }
        }
        else
        {
            // apply to single led only
            result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ack : System::Config::status_t::errorWrite;
        }
    }
    break;

    default:
    {
        result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ack : System::Config::status_t::errorWrite;
    }
    break;
    }

    return result;
}

#endif