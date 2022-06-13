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
#include "global/MIDIProgram.h"

#ifdef LEDS_SUPPORTED

#include "core/src/Timing.h"
#include "core/src/util/Util.h"
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
        _brightness[i] = brightness_t::OFF;
    }

    MIDIDispatcher.listen(Messaging::eventType_t::MIDI_IN,
                          [this](const Messaging::event_t& event)
                          {
                              switch (event.message)
                              {
                              case MIDI::messageType_t::NOTE_ON:
                              case MIDI::messageType_t::NOTE_OFF:
                              case MIDI::messageType_t::CONTROL_CHANGE:
                              case MIDI::messageType_t::PROGRAM_CHANGE:
                              {
                                  midiToState(event, Messaging::eventType_t::MIDI_IN);
                              }
                              break;

                              case MIDI::messageType_t::SYS_REAL_TIME_CLOCK:
                              {
                                  updateAll(true);
                              }
                              break;

                              case MIDI::messageType_t::SYS_REAL_TIME_START:
                              {
                                  resetBlinking();
                                  updateAll(true);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::BUTTON,
                          [this](const Messaging::event_t& event)
                          {
                              switch (event.message)
                              {
                              case MIDI::messageType_t::NOTE_ON:
                              case MIDI::messageType_t::NOTE_OFF:
                              case MIDI::messageType_t::CONTROL_CHANGE:
                              case MIDI::messageType_t::PROGRAM_CHANGE:
                              {
                                  midiToState(event, Messaging::eventType_t::BUTTON);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::ANALOG,
                          [this](const Messaging::event_t& event)
                          {
                              switch (event.message)
                              {
                              case MIDI::messageType_t::NOTE_ON:
                              case MIDI::messageType_t::NOTE_OFF:
                              case MIDI::messageType_t::CONTROL_CHANGE:
                              case MIDI::messageType_t::PROGRAM_CHANGE:
                              {
                                  midiToState(event, Messaging::eventType_t::ANALOG);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::SYSTEM,
                          [this](const Messaging::event_t& event)
                          {
                              switch (event.systemMessage)
                              {
                              case Messaging::systemMessage_t::PRESET_CHANGED:
                              {
                                  setAllOff();
                                  midiToState(event, Messaging::eventType_t::SYSTEM);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::TOUCHSCREEN_SCREEN,
                          [this](const Messaging::event_t& event)
                          {
                              refresh();
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::PROGRAM,
                          [this](const Messaging::event_t& event)
                          {
                              midiToState(event, Messaging::eventType_t::PROGRAM);
                          });

    ConfigHandler.registerConfig(
        System::Config::block_t::LEDS,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sysConfigGet(static_cast<System::Config::Section::leds_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sysConfigSet(static_cast<System::Config::Section::leds_t>(section), index, value);
        });
}

bool LEDs::init()
{
    setAllOff();

    if (_database.read(Database::Config::Section::leds_t::GLOBAL, setting_t::USE_STARTUP_ANIMATION))
    {
        startUpAnimation();
    }

    setBlinkType(static_cast<blinkType_t>(_database.read(Database::Config::Section::leds_t::GLOBAL, setting_t::BLINK_WITH_MIDI_CLOCK)));

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
    case blinkType_t::TIMER:
    {
        if ((core::timing::ms() - _lastLEDblinkUpdateTime) < LED_BLINK_TIMER_TYPE_CHECK_TIME)
        {
            return;
        }

        _lastLEDblinkUpdateTime = core::timing::ms();
    }
    break;

    case blinkType_t::MIDI_CLOCK:
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
            if (!bit(j, ledBit_t::BLINK_ON))
            {
                continue;
            }

            if (_blinkTimer[j] != i)
            {
                continue;
            }

            updateBit(j, ledBit_t::STATE, _blinkState[i]);
            setState(j, bit(j, ledBit_t::STATE) ? _brightness[j] : brightness_t::OFF);
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
        setState(i, brightness_t::OFF);
        core::timing::waitMs(35);
    }

    for (size_t i = 0; i < Collection::size(GROUP_DIGITAL_OUTPUTS); i++)
    {
        setState(Collection::size(GROUP_DIGITAL_OUTPUTS) - 1 - i, brightness_t::B100);
        core::timing::waitMs(35);
    }

    for (size_t i = 0; i < Collection::size(GROUP_DIGITAL_OUTPUTS); i++)
    {
        setState(i, brightness_t::OFF);
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
        return blinkSpeed_t::NO_BLINK;
    }

    // there are 4 total blink speeds
    return static_cast<blinkSpeed_t>(value % 16 / TOTAL_BLINK_SPEEDS);
}

LEDs::brightness_t LEDs::valueToBrightness(uint8_t value)
{
    if (value < 16)
    {
        return brightness_t::OFF;
    }

    return static_cast<brightness_t>((value % 16 % TOTAL_BRIGHTNESS_VALUES) + 1);
}

void LEDs::midiToState(const Messaging::event_t& event, Messaging::eventType_t source)
{
    const uint8_t GLOBAL_CHANNEL = _database.read(Database::Config::Section::global_t::MIDI_SETTINGS, MIDI::setting_t::GLOBAL_CHANNEL);
    const uint8_t CHANNEL        = _database.read(Database::Config::Section::global_t::MIDI_SETTINGS,
                                           MIDI::setting_t::USE_GLOBAL_CHANNEL)
                                       ? GLOBAL_CHANNEL
                                       : event.channel;

    const bool USE_OMNI = CHANNEL == MIDI::MIDI_CHANNEL_OMNI ? true : false;

    auto eventToMessage = [](const Messaging::event_t& event)
    {
        auto message = event.message;

        // ignore the distinction between note on and off
        if (message == MIDI::messageType_t::NOTE_OFF)
        {
            message = MIDI::messageType_t::NOTE_ON;
        }

        if (event.systemMessage == Messaging::systemMessage_t::PRESET_CHANGED)
        {
            message = MIDI::messageType_t::PROGRAM_CHANGE;
        }

        return message;
    };

    auto isControlTypeMatched = [](MIDI::messageType_t message, controlType_t controlType)
    {
        return CONTROL_TYPE_TO_MIDI_MESSAGE[static_cast<uint8_t>(controlType)] == message;
    };

    auto message = eventToMessage(event);

    for (size_t i = 0; i < Collection::size(); i++)
    {
        auto controlType = static_cast<controlType_t>(_database.read(Database::Config::Section::leds_t::CONTROL_TYPE, i));

        // match received midi message with the assigned LED control type
        if (!isControlTypeMatched(message, controlType))
        {
            continue;
        }

        bool setState     = false;
        bool setBlink     = false;
        bool checkChannel = true;

        // determine whether led state or blink state should be changed
        // received MIDI message must match with defined control type
        if (source != Messaging::eventType_t::MIDI_IN)
        {
            switch (controlType)
            {
            case controlType_t::LOCAL_NOTE_SINGLE_VAL:
            {
                if (message == MIDI::messageType_t::NOTE_ON)
                {
                    setState = true;
                }
            }
            break;

            case controlType_t::LOCAL_CC_SINGLE_VAL:
            {
                if (message == MIDI::messageType_t::CONTROL_CHANGE)
                {
                    setState = true;
                }
            }
            break;

            case controlType_t::PC_SINGLE_VAL:
            {
                // source must be verified here, otherwise no difference between program and preset source is detected
                if ((message == MIDI::messageType_t::PROGRAM_CHANGE) && (source != Messaging::eventType_t::SYSTEM))
                {
                    setState = true;
                }
            }
            break;

            case controlType_t::PRESET:
            {
                // message is set to PROGRAM_CHANGE when changing preset so that internal control type matching is possible
                if ((message == MIDI::messageType_t::PROGRAM_CHANGE) && (source == Messaging::eventType_t::SYSTEM))
                {
                    setState = true;
                }

                checkChannel = false;
            }
            break;

            case controlType_t::LOCAL_NOTE_MULTI_VAL:
            {
                if (message == MIDI::messageType_t::NOTE_ON)
                {
                    setState = true;
                    setBlink = true;
                }
            }
            break;

            case controlType_t::LOCAL_CC_MULTI_VAL:
            {
                if (message == MIDI::messageType_t::CONTROL_CHANGE)
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
            case controlType_t::MIDI_IN_NOTE_SINGLE_VAL:
            {
                if (message == MIDI::messageType_t::NOTE_ON)
                {
                    setState = true;
                }
            }
            break;

            case controlType_t::MIDI_IN_CC_SINGLE_VAL:
            {
                if (message == MIDI::messageType_t::CONTROL_CHANGE)
                {
                    setState = true;
                }
            }
            break;

            case controlType_t::PC_SINGLE_VAL:
            {
                if (message == MIDI::messageType_t::PROGRAM_CHANGE)
                {
                    setState = true;
                }
            }
            break;

            case controlType_t::MIDI_IN_NOTE_MULTI_VAL:
            {
                if (message == MIDI::messageType_t::NOTE_ON)
                {
                    setState = true;
                    setBlink = true;
                }
            }
            break;

            case controlType_t::MIDI_IN_CC_MULTI_VAL:
            {
                if (message == MIDI::messageType_t::CONTROL_CHANGE)
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

        if (checkChannel && !USE_OMNI)
        {
            // no point in further checking if channel doesn't match
            if (_database.read(Database::Config::Section::leds_t::CHANNEL, i) != CHANNEL)
            {
                continue;
            }
        }

        if (setState || setBlink)
        {
            auto color      = color_t::OFF;
            auto brightness = brightness_t::OFF;

            // in single value modes, brightness and blink speed cannot be controlled since we're dealing
            // with one value only

            uint8_t activationID = _database.read(Database::Config::Section::leds_t::ACTIVATION_ID, i);

            if (message == MIDI::messageType_t::PROGRAM_CHANGE)
            {
                if (_database.read(Database::Config::Section::leds_t::GLOBAL, setting_t::USE_MIDI_PROGRAM_OFFSET))
                {
                    activationID += MIDIProgram.offset();
                    activationID &= 0x7F;
                }
            }

            if (setState)
            {
                // match activation ID with received ID
                if (activationID == event.index)
                {
                    if (message == MIDI::messageType_t::PROGRAM_CHANGE)
                    {
                        // byte2 doesn't exist on program change message
                        color      = color_t::RED;
                        brightness = brightness_t::B100;
                    }
                    else
                    {
                        // when note/cc are used to control both state and blinking ignore activation velocity
                        if (setState && setBlink)
                        {
                            color      = valueToColor(event.value);
                            brightness = valueToBrightness(event.value);
                        }
                        else
                        {
                            // this has side effect that it will always set RGB LED to red color since no color information is available
                            color      = (_database.read(Database::Config::Section::leds_t::ACTIVATION_VALUE, i) == event.value) ? color_t::RED : color_t::OFF;
                            brightness = brightness_t::B100;
                        }
                    }

                    setColor(i, color, brightness);
                }
                else
                {
                    if (message == MIDI::messageType_t::PROGRAM_CHANGE)
                    {
                        setColor(i, color_t::OFF, brightness_t::OFF);
                    }
                }
            }

            if (setBlink)
            {
                // match activation ID with received ID
                if (activationID == event.index)
                {
                    // if both state and blink speed should be set, then don't update the state again in setBlinkSpeed
                    setBlinkSpeed(i, valueToBlinkSpeed(event.value), !(setState && setBlink));
                }
            }
        }
    }
}

void LEDs::setBlinkSpeed(uint8_t ledID, blinkSpeed_t state, bool updateState)
{
    uint8_t ledArray[3]   = {};
    uint8_t leds          = 0;
    uint8_t rgbFromOutput = _hwa.rgbFromOutput(ledID);

    if (_database.read(Database::Config::Section::leds_t::RGB_ENABLE, rgbFromOutput))
    {
        ledArray[0] = _hwa.rgbComponentFromRGB(rgbFromOutput, rgbComponent_t::R);
        ledArray[1] = _hwa.rgbComponentFromRGB(rgbFromOutput, rgbComponent_t::G);
        ledArray[2] = _hwa.rgbComponentFromRGB(rgbFromOutput, rgbComponent_t::B);

        leds = 3;
    }
    else
    {
        ledArray[0] = ledID;

        leds = 1;
    }

    for (int i = 0; i < leds; i++)
    {
        if (state != blinkSpeed_t::NO_BLINK)
        {
            updateBit(ledArray[i], ledBit_t::BLINK_ON, true);
            updateBit(ledArray[i], ledBit_t::STATE, true);
        }
        else
        {
            updateBit(ledArray[i], ledBit_t::BLINK_ON, false);
            updateBit(ledArray[i], ledBit_t::STATE, bit(ledArray[i], ledBit_t::ACTIVE));
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
        setColor(i, color_t::RED, brightness_t::B100);
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
    uint8_t rgbFromOutput = _hwa.rgbFromOutput(ledID);

    auto handleLED = [&](uint8_t index, rgbComponent_t rgbFromOutput, bool state, bool isRGB)
    {
        if (state)
        {
            updateBit(index, ledBit_t::ACTIVE, true);
            updateBit(index, ledBit_t::STATE, true);

            if (isRGB)
            {
                updateBit(index, ledBit_t::RGB, true);

                switch (rgbFromOutput)
                {
                case rgbComponent_t::R:
                {
                    updateBit(index, ledBit_t::RGB_R, state);
                }
                break;

                case rgbComponent_t::G:
                {
                    updateBit(index, ledBit_t::RGB_G, state);
                }
                break;

                case rgbComponent_t::B:
                {
                    updateBit(index, ledBit_t::RGB_B, state);
                }
                break;
                }
            }
            else
            {
                updateBit(index, ledBit_t::RGB, false);
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

    if (_database.read(Database::Config::Section::leds_t::RGB_ENABLE, rgbFromOutput))
    {
        // rgb led is composed of three standard LEDs
        // get indexes of individual LEDs first
        uint8_t rLED = _hwa.rgbComponentFromRGB(rgbFromOutput, rgbComponent_t::R);
        uint8_t gLED = _hwa.rgbComponentFromRGB(rgbFromOutput, rgbComponent_t::G);
        uint8_t bLED = _hwa.rgbComponentFromRGB(rgbFromOutput, rgbComponent_t::B);

        handleLED(rLED, rgbComponent_t::R, core::util::BIT_READ(static_cast<uint8_t>(color), static_cast<size_t>(rgbComponent_t::R)), true);
        handleLED(gLED, rgbComponent_t::G, core::util::BIT_READ(static_cast<uint8_t>(color), static_cast<size_t>(rgbComponent_t::G)), true);
        handleLED(bLED, rgbComponent_t::B, core::util::BIT_READ(static_cast<uint8_t>(color), static_cast<size_t>(rgbComponent_t::B)), true);
    }
    else
    {
        handleLED(ledID, rgbComponent_t::R, static_cast<bool>(color), false);
    }
}

IO::LEDs::blinkSpeed_t LEDs::blinkSpeed(uint8_t ledID)
{
    return static_cast<blinkSpeed_t>(_blinkTimer[ledID]);
}

void LEDs::setBlinkType(blinkType_t blinkType)
{
    switch (blinkType)
    {
    case blinkType_t::TIMER:
    {
        _blinkResetArrayPtr = BLINK_RESET_TIMER;
    }
    break;

    case blinkType_t::MIDI_CLOCK:
    {
        _blinkResetArrayPtr = BLINK_RESET_MIDI_CLOCK;
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
    core::util::BIT_WRITE(_ledState[index], static_cast<uint8_t>(bit), state);
}

bool LEDs::bit(uint8_t index, ledBit_t bit)
{
    return core::util::BIT_READ(_ledState[index], static_cast<size_t>(bit));
}

LEDs::color_t LEDs::color(uint8_t ledID)
{
    if (!bit(ledID, ledBit_t::ACTIVE))
    {
        return color_t::OFF;
    }

    if (!bit(ledID, ledBit_t::RGB))
    {
        // single color led
        return color_t::RED;
    }

    // rgb led
    uint8_t rgbFromOutput = _hwa.rgbFromOutput(ledID);

    uint8_t color = 0;
    color |= bit(_hwa.rgbComponentFromRGB(rgbFromOutput, rgbComponent_t::B), ledBit_t::RGB_B);
    color <<= 1;
    color |= bit(_hwa.rgbComponentFromRGB(rgbFromOutput, rgbComponent_t::G), ledBit_t::RGB_G);
    color <<= 1;
    color |= bit(_hwa.rgbComponentFromRGB(rgbFromOutput, rgbComponent_t::R), ledBit_t::RGB_R);

    return static_cast<color_t>(color);
}

void LEDs::resetState(uint8_t index)
{
    _ledState[index]   = 0;
    _brightness[index] = brightness_t::OFF;
    setState(index, brightness_t::OFF);
}

void LEDs::setState(size_t index, brightness_t brightness)
{
    if (index >= Collection::size(GROUP_DIGITAL_OUTPUTS))
    {
        // specified hwa interface only writes to physical leds
        // for touchscreen and other destinations, notify via dispatcher

        Messaging::event_t event;
        event.componentIndex = index - Collection::startIndex(GROUP_TOUCHSCREEN_COMPONENTS);
        event.value          = static_cast<uint16_t>(brightness);

        MIDIDispatcher.notify(Messaging::eventType_t::TOUCHSCREEN_LED, event);
    }
    else
    {
        _hwa.setState(index, brightness);
    }
}

std::optional<uint8_t> LEDs::sysConfigGet(System::Config::Section::leds_t section, size_t index, uint16_t& value)
{
    int32_t readValue;
    auto    result = System::Config::status_t::ACK;

    switch (section)
    {
    case System::Config::Section::leds_t::TEST_COLOR:
    {
        readValue = static_cast<int32_t>(color(index));
    }
    break;

    case System::Config::Section::leds_t::CHANNEL:
    {
        result = _database.read(Util::Conversion::sys2DBsection(section),
                                index,
                                readValue)
                     ? System::Config::status_t::ACK
                     : System::Config::status_t::ERROR_READ;
    }
    break;

    case System::Config::Section::leds_t::RGB_ENABLE:
    {
        result = _database.read(Util::Conversion::sys2DBsection(section),
                                _hwa.rgbFromOutput(index),
                                readValue)
                     ? System::Config::status_t::ACK
                     : System::Config::status_t::ERROR_READ;
    }
    break;

    default:
    {
        result = _database.read(Util::Conversion::sys2DBsection(section),
                                index,
                                readValue)
                     ? System::Config::status_t::ACK
                     : System::Config::status_t::ERROR_READ;
    }
    break;
    }

    value = readValue;

    return result;
}

std::optional<uint8_t> LEDs::sysConfigSet(System::Config::Section::leds_t section, size_t index, uint16_t value)
{
    uint8_t result = System::Config::status_t::ERROR_WRITE;

    switch (section)
    {
    case System::Config::Section::leds_t::TEST_COLOR:
    {
        // no writing to database
        setColor(index, static_cast<color_t>(value), brightness_t::B100);
        result = System::Config::status_t::ACK;
    }
    break;

    case System::Config::Section::leds_t::GLOBAL:
    {
        auto ledSetting = static_cast<setting_t>(index);

        switch (ledSetting)
        {
        case setting_t::BLINK_WITH_MIDI_CLOCK:
        {
            if ((value <= 1) && (value >= 0))
            {
                result = System::Config::status_t::ACK;
                setBlinkType(static_cast<blinkType_t>(value));
            }
        }
        break;

        case setting_t::USE_STARTUP_ANIMATION:
        {
            if ((value <= 1) && (value >= 0))
            {
                result = System::Config::status_t::ACK;
            }
        }
        break;

        case setting_t::UNUSED:
        {
            result = System::Config::status_t::ACK;
        }
        break;

        case setting_t::USE_MIDI_PROGRAM_OFFSET:
        {
            result = System::Config::status_t::ACK;
        }
        break;

        default:
            break;
        }

        // write to db if success is true and writing should take place
        if (result == System::Config::status_t::ACK)
        {
            result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ACK : System::Config::status_t::ERROR_WRITE;
        }
    }
    break;

    case System::Config::Section::leds_t::RGB_ENABLE:
    {
        // make sure to turn all three leds off before setting new state
        setColor(_hwa.rgbComponentFromRGB(_hwa.rgbFromOutput(index), rgbComponent_t::R), color_t::OFF, brightness_t::OFF);
        setColor(_hwa.rgbComponentFromRGB(_hwa.rgbFromOutput(index), rgbComponent_t::G), color_t::OFF, brightness_t::OFF);
        setColor(_hwa.rgbComponentFromRGB(_hwa.rgbFromOutput(index), rgbComponent_t::B), color_t::OFF, brightness_t::OFF);

        // write rgb enabled bit to led
        result = _database.update(Util::Conversion::sys2DBsection(section),
                                  _hwa.rgbFromOutput(index),
                                  value)
                     ? System::Config::status_t::ACK
                     : System::Config::status_t::ERROR_WRITE;

        if (value && (result == System::Config::status_t::ACK))
        {
            // copy over note activation local control and midi channel settings to all three leds from the current led index

            for (int i = 0; i < 3; i++)
            {
                result = _database.update(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::ACTIVATION_ID),
                                          _hwa.rgbComponentFromRGB(_hwa.rgbFromOutput(index), static_cast<rgbComponent_t>(i)),
                                          _database.read(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::ACTIVATION_ID), index))
                             ? System::Config::status_t::ACK
                             : System::Config::status_t::ERROR_WRITE;

                if (result != System::Config::status_t::ACK)
                {
                    break;
                }

                result = _database.update(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::CONTROL_TYPE),
                                          _hwa.rgbComponentFromRGB(_hwa.rgbFromOutput(index), static_cast<rgbComponent_t>(i)),
                                          _database.read(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::CONTROL_TYPE), index))
                             ? System::Config::status_t::ACK
                             : System::Config::status_t::ERROR_WRITE;

                if (result != System::Config::status_t::ACK)
                {
                    break;
                }

                result = _database.update(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::CHANNEL),
                                          _hwa.rgbComponentFromRGB(_hwa.rgbFromOutput(index), static_cast<rgbComponent_t>(i)),
                                          _database.read(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::CHANNEL), index))
                             ? System::Config::status_t::ACK
                             : System::Config::status_t::ERROR_WRITE;

                if (result != System::Config::status_t::ACK)
                {
                    break;
                }
            }
        }
    }
    break;

    case System::Config::Section::leds_t::ACTIVATION_ID:
    case System::Config::Section::leds_t::CONTROL_TYPE:
    case System::Config::Section::leds_t::CHANNEL:
    {
        // first, find out if RGB led is enabled for this led index
        if (_database.read(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::RGB_ENABLE), _hwa.rgbFromOutput(index)))
        {
            // rgb led enabled - copy these settings to all three leds
            for (int i = 0; i < 3; i++)
            {
                result = _database.update(Util::Conversion::sys2DBsection(section),
                                          _hwa.rgbComponentFromRGB(_hwa.rgbFromOutput(index), static_cast<rgbComponent_t>(i)),
                                          value)
                             ? System::Config::status_t::ACK
                             : System::Config::status_t::ERROR_WRITE;

                if (result != System::Config::status_t::ACK)
                {
                    break;
                }
            }
        }
        else
        {
            // apply to single led only
            result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ACK : System::Config::status_t::ERROR_WRITE;
        }
    }
    break;

    default:
    {
        result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ACK : System::Config::status_t::ERROR_WRITE;
    }
    break;
    }

    return result;
}

#endif