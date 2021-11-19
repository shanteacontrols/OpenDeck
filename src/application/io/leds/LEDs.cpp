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
#include "util/messaging/Messaging.h"

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
        _blinkState[i] = true;

    for (size_t i = 0; i < Collection::size(); i++)
        _brightness[i] = brightness_t::bOff;

    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::midiIn,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          switch (dispatchMessage.message)
                          {
                          case MIDI::messageType_t::noteOn:
                          case MIDI::messageType_t::noteOff:
                          case MIDI::messageType_t::controlChange:
                          case MIDI::messageType_t::programChange:
                          {
                              midiToState(dispatchMessage, Util::MessageDispatcher::messageSource_t::midiIn);
                          }
                          break;

                          case MIDI::messageType_t::sysRealTimeClock:
                          {
                              update(true);
                          }
                          break;

                          case MIDI::messageType_t::sysRealTimeStart:
                          {
                              resetBlinking();
                              update(true);
                          }
                          break;

                          default:
                              break;
                          }
                      });

    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::buttons,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          switch (dispatchMessage.message)
                          {
                          case MIDI::messageType_t::noteOn:
                          case MIDI::messageType_t::noteOff:
                          case MIDI::messageType_t::controlChange:
                          case MIDI::messageType_t::programChange:
                          {
                              midiToState(dispatchMessage, Util::MessageDispatcher::messageSource_t::buttons);
                          }
                          break;

                          default:
                              break;
                          }
                      });

    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::analog,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          switch (dispatchMessage.message)
                          {
                          case MIDI::messageType_t::noteOn:
                          case MIDI::messageType_t::noteOff:
                          case MIDI::messageType_t::controlChange:
                          case MIDI::messageType_t::programChange:
                          {
                              midiToState(dispatchMessage, Util::MessageDispatcher::messageSource_t::analog);
                          }
                          break;

                          default:
                              break;
                          }
                      });

    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::preset,
                      Util::MessageDispatcher::listenType_t::all,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          setAllOff();
                          midiToState(dispatchMessage, Util::MessageDispatcher::messageSource_t::preset);
                      });

    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::touchscreenScreen,
                      Util::MessageDispatcher::listenType_t::all,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          refresh();
                      });

    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::system,
                      Util::MessageDispatcher::listenType_t::all,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          if (dispatchMessage.componentIndex == static_cast<uint8_t>(Util::MessageDispatcher::systemMessages_t::midiProgramIndication))
                          {
                              // pretend this is midi in message - source isn't important here as
                              // both midi in and local control for program change are synced
                              midiToState(dispatchMessage, Util::MessageDispatcher::messageSource_t::midiIn);
                          }
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
        startUpAnimation();

    setBlinkType(static_cast<blinkType_t>(_database.read(Database::Section::leds_t::global, setting_t::blinkWithMIDIclock)));

    return true;
}

void LEDs::update(bool forceRefresh)
{
    if (_blinkResetArrayPtr == nullptr)
        return;

    switch (_ledBlinkType)
    {
    case blinkType_t::timer:
    {
        if ((core::timing::currentRunTimeMs() - _lastLEDblinkUpdateTime) < LED_BLINK_TIMER_TYPE_CHECK_TIME)
            return;

        _lastLEDblinkUpdateTime = core::timing::currentRunTimeMs();
    }
    break;

    case blinkType_t::midiClock:
    {
        if (!forceRefresh)
            return;
    }
    break;

    default:
        return;
    }

    // change the blink state for specific blink rate
    for (size_t i = 0; i < TOTAL_BLINK_SPEEDS; i++)
    {
        if (++_blinkCounter[i] < _blinkResetArrayPtr[i])
            continue;

        _blinkState[i]   = !_blinkState[i];
        _blinkCounter[i] = 0;

        // assign changed state to all leds which have this speed
        for (size_t j = 0; j < Collection::size(); j++)
        {
            if (!bit(j, ledBit_t::blinkOn))
                continue;

            if (_blinkTimer[j] != i)
                continue;

            updateBit(j, ledBit_t::state, _blinkState[i]);
            setState(j, bit(j, ledBit_t::state) ? _brightness[j] : brightness_t::bOff);
        }
    }
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
        return blinkSpeed_t::noBlink;

    // there are 4 total blink speeds
    return static_cast<blinkSpeed_t>(value % 16 / TOTAL_BLINK_SPEEDS);
}

LEDs::brightness_t LEDs::valueToBrightness(uint8_t value)
{
    if (value < 16)
        return brightness_t::bOff;

    return static_cast<brightness_t>((value % 16 % TOTAL_BRIGHTNESS_VALUES) + 1);
}

void LEDs::midiToState(Util::MessageDispatcher::message_t message, Util::MessageDispatcher::messageSource_t source)
{
    for (size_t i = 0; i < Collection::size(); i++)
    {
        auto controlType = static_cast<controlType_t>(_database.read(Database::Section::leds_t::controlType, i));

        // match received midi message with the assigned LED control type
        if (!isControlTypeMatched(message.message, controlType))
            continue;

        // no point in checking if channel doesn't match
        if (_database.read(Database::Section::leds_t::midiChannel, i) != message.midiChannel)
            continue;

        bool setState = false;
        bool setBlink = false;

        // determine whether led state or blink state should be changed
        // received MIDI message must match with defined control type
        if (source != Util::MessageDispatcher::messageSource_t::midiIn)
        {
            switch (controlType)
            {
            case controlType_t::localNoteSingleVal:
            {
                if ((message.message == MIDI::messageType_t::noteOn) || (message.message == MIDI::messageType_t::noteOff))
                    setState = true;
            }
            break;

            case controlType_t::localCCSingleVal:
            {
                if (message.message == MIDI::messageType_t::controlChange)
                    setState = true;
            }
            break;

            case controlType_t::pcSingleVal:
            {
                if (message.message == MIDI::messageType_t::programChange)
                    setState = true;
            }
            break;

            case controlType_t::preset:
            {
                // it is expected for MIDI message to be set to program change when changing preset
                if (message.message == MIDI::messageType_t::programChange)
                    setState = true;
            }
            break;

            case controlType_t::localNoteMultiVal:
            {
                if ((message.message == MIDI::messageType_t::noteOn) || (message.message == MIDI::messageType_t::noteOff))
                {
                    setState = true;
                    setBlink = true;
                }
            }
            break;

            case controlType_t::localCCMultiVal:
            {
                if (message.message == MIDI::messageType_t::controlChange)
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
                if ((message.message == MIDI::messageType_t::noteOn) || (message.message == MIDI::messageType_t::noteOff))
                    setState = true;
            }
            break;

            case controlType_t::midiInCCSingleVal:
            {
                if (message.message == MIDI::messageType_t::controlChange)
                    setState = true;
            }
            break;

            case controlType_t::pcSingleVal:
            {
                if (message.message == MIDI::messageType_t::programChange)
                    setState = true;
            }
            break;

            case controlType_t::midiInNoteMultiVal:
            {
                if ((message.message == MIDI::messageType_t::noteOn) || (message.message == MIDI::messageType_t::noteOff))
                {
                    setState = true;
                    setBlink = true;
                }
            }
            break;

            case controlType_t::midiInCCMultiVal:
            {
                if (message.message == MIDI::messageType_t::controlChange)
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
                if (activationID == message.midiIndex)
                {
                    if (message.message == MIDI::messageType_t::programChange)
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
                            color      = valueToColor(message.midiValue);
                            brightness = valueToBrightness(message.midiValue);
                        }
                        else
                        {
                            // this has side effect that it will always set RGB LED to red color since no color information is available
                            color      = (_database.read(Database::Section::leds_t::activationValue, i) == message.midiValue) ? color_t::red : color_t::off;
                            brightness = brightness_t::b100;
                        }
                    }

                    setColor(i, color, brightness);
                }
                else
                {
                    if (message.message == MIDI::messageType_t::programChange)
                    {
                        setColor(i, color_t::off, brightness_t::bOff);
                    }
                }
            }

            if (setBlink)
            {
                // match activation ID with received ID
                if (activationID == message.midiIndex)
                    setBlinkSpeed(i, valueToBlinkSpeed(message.midiValue));
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
        ledArray[0] = _hwa.rgbSignalIndex(rgbIndex, rgbIndex_t::r);
        ledArray[1] = _hwa.rgbSignalIndex(rgbIndex, rgbIndex_t::g);
        ledArray[2] = _hwa.rgbSignalIndex(rgbIndex, rgbIndex_t::b);

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

        setState(ledArray[i], _brightness[ledArray[i]]);
        _blinkTimer[ledID] = static_cast<uint8_t>(state);
    }
}

void LEDs::setAllOn()
{
    // turn on all LEDs
    for (size_t i = 0; i < Collection::size(); i++)
        setColor(i, color_t::red, brightness_t::b100);
}

void LEDs::setAllOff()
{
    // turn off all LEDs
    for (size_t i = 0; i < Collection::size(); i++)
        resetState(i);
}

void LEDs::refresh()
{
    for (size_t i = 0; i < Collection::size(); i++)
        setState(i, _brightness[i]);
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
    else
    {
        if (!bit(ledID, ledBit_t::rgb))
        {
            // single color led
            return color_t::red;
        }
        else
        {
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
    }
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
        midiMessage = MIDI::messageType_t::noteOn;

    return controlTypeToMIDImessage[static_cast<uint8_t>(controlType)] == midiMessage;
}

void LEDs::setState(size_t index, brightness_t brightness)
{
    if (index >= Collection::size(GROUP_DIGITAL_OUTPUTS))
    {
        // specified hwa interface only writes to physical leds
        // for touchscreen and other destinations, notify via dispatcher

        Util::MessageDispatcher::message_t message;
        message.componentIndex = index - Collection::startIndex(GROUP_TOUCHSCREEN_COMPONENTS);
        message.midiValue      = static_cast<uint16_t>(brightness);

        Dispatcher.notify(Util::MessageDispatcher::messageSource_t::leds,
                          message,
                          Util::MessageDispatcher::listenType_t::fwd);
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

        // channels start from 0 in db, start from 1 in sysex
        if (result == System::Config::status_t::ack)
            readValue++;
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
                result = System::Config::status_t::ack;
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
            result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ack : System::Config::status_t::errorWrite;
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
                    break;

                result = _database.update(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::controlType),
                                          rgbSignalIndex(rgbIndex(index), static_cast<rgbIndex_t>(i)),
                                          _database.read(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::controlType), index))
                             ? System::Config::status_t::ack
                             : System::Config::status_t::errorWrite;

                if (result != System::Config::status_t::ack)
                    break;

                result = _database.update(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::midiChannel),
                                          rgbSignalIndex(rgbIndex(index), static_cast<rgbIndex_t>(i)),
                                          _database.read(Util::Conversion::sys2DBsection(System::Config::Section::leds_t::midiChannel), index))
                             ? System::Config::status_t::ack
                             : System::Config::status_t::errorWrite;

                if (result != System::Config::status_t::ack)
                    break;
            }
        }
    }
    break;

    case System::Config::Section::leds_t::activationID:
    case System::Config::Section::leds_t::controlType:
    case System::Config::Section::leds_t::midiChannel:
    {
        // channels start from 0 in db, start from 1 in sysex
        if (section == System::Config::Section::leds_t::midiChannel)
            value--;

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
                    break;
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