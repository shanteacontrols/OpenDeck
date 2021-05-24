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

#include "System.h"
#include "io/leds/LEDs.h"

SysExConf::DataHandler::result_t System::SysExDataHandler::set(uint8_t  block,
                                                               uint8_t  section,
                                                               uint16_t index,
                                                               uint16_t newValue)
{
    auto sysExBlock = static_cast<block_t>(block);
    auto result     = System::result_t::notSupported;

    switch (sysExBlock)
    {
    case block_t::global:
    {
        result = _system.onSetGlobal(static_cast<Section::global_t>(section), index, newValue);
    }
    break;

    case block_t::buttons:
    {
        result = _system.onSetButtons(static_cast<Section::button_t>(section), index, newValue);
    }
    break;

    case block_t::encoders:
    {
        result = _system.onSetEncoders(static_cast<Section::encoder_t>(section), index, newValue);
    }
    break;

    case block_t::analog:
    {
        result = _system.onSetAnalog(static_cast<Section::analog_t>(section), index, newValue);
    }
    break;

    case block_t::leds:
    {
        result = _system.onSetLEDs(static_cast<Section::leds_t>(section), index, newValue);
    }
    break;

    case block_t::display:
    {
        result = _system.onSetDisplay(static_cast<Section::display_t>(section), index, newValue);
    }
    break;

    case block_t::touchscreen:
    {
        result = _system.onSetTouchscreen(static_cast<Section::touchscreen_t>(section), index, newValue);
    }
    break;

    default:
        break;
    }

    _system._display.displayMIDIevent(IO::Display::eventType_t::in,
                                      IO::Display::event_t::systemExclusive,
                                      0,
                                      0,
                                      0);

    return result;
}

System::result_t System::onSetGlobal(Section::global_t section, size_t index, uint16_t newValue)
{
    auto result    = System::result_t::error;
    bool writeToDb = true;

    switch (section)
    {
    case Section::global_t::midiFeatures:
    {
        auto feature = static_cast<System::midiFeature_t>(index);

        switch (feature)
        {
        case System::midiFeature_t::runningStatus:
        {
#ifndef DIN_MIDI_SUPPORTED
            result = System::result_t::notSupported;
#else
            _midi.setRunningStatusState(newValue);
            result = System::result_t::ok;
#endif
        }
        break;

        case System::midiFeature_t::standardNoteOff:
        {
            if (newValue)
                _midi.setNoteOffMode(MIDI::noteOffType_t::standardNoteOff);
            else
                _midi.setNoteOffMode(MIDI::noteOffType_t::noteOnZeroVel);

            result = System::result_t::ok;
        }
        break;

        case System::midiFeature_t::dinEnabled:
        {
#ifndef DIN_MIDI_SUPPORTED
            result = System::result_t::notSupported;
#else
            if (newValue)
                _hwa.enableDINMIDI(false);
            else
                _hwa.disableDINMIDI();

            result = System::result_t::ok;
#endif
        }
        break;

        case System::midiFeature_t::mergeEnabled:
        {
#ifndef DIN_MIDI_SUPPORTED
            result = System::result_t::notSupported;
#else
            result = System::result_t::ok;

            //make sure everything is in correct state
            if (!newValue)
            {
                if (isMIDIfeatureEnabled(midiFeature_t::dinEnabled))
                {
                    _hwa.enableDINMIDI(false);
                    _midi.useRecursiveParsing(false);
                }
            }
            else
            {
                //restore active settings if din midi is enabled
                if (isMIDIfeatureEnabled(midiFeature_t::dinEnabled))
                {
                    //use recursive parsing when merging is active
                    _midi.useRecursiveParsing(true);
                    configureMIDImerge(midiMergeType());
                }
            }
#endif
        }
        break;

        default:
            result = System::result_t::ok;
            break;
        }
    }
    break;

    case Section::global_t::midiMerge:
    {
#ifndef DIN_MIDI_SUPPORTED
        result = System::result_t::notSupported;
#else
        auto mergeParam = static_cast<midiMerge_t>(index);

        switch (mergeParam)
        {
        case midiMerge_t::mergeType:
        {
            if ((newValue >= 0) && (newValue < static_cast<size_t>(midiMergeType_t::AMOUNT)))
            {
                result = System::result_t::ok;

                //configure merging only if din midi and merging options are enabled
                if (isMIDIfeatureEnabled(midiFeature_t::dinEnabled) && isMIDIfeatureEnabled(midiFeature_t::mergeEnabled))
                    configureMIDImerge(static_cast<midiMergeType_t>(newValue));
            }
            else
            {
                result = System::result_t::notSupported;
            }
        }
        break;

        case midiMerge_t::mergeUSBchannel:
        case midiMerge_t::mergeDINchannel:
        {
            //unused for now
            writeToDb = false;
            result = System::result_t::ok;
        }
        break;

        default:
            break;
        }
#endif
    }
    break;

    case Section::global_t::presets:
    {
        auto setting = static_cast<presetSetting_t>(index);

        switch (setting)
        {
        case presetSetting_t::activePreset:
        {
            if (newValue < _database.getSupportedPresets())
            {
                _database.setPreset(newValue);
                result    = System::result_t::ok;
                writeToDb = false;
            }
            else
            {
                result = System::result_t::notSupported;
            }
        }
        break;

        case presetSetting_t::presetPreserve:
        {
            if ((newValue <= 1) && (newValue >= 0))
            {
                _database.setPresetPreserveState(newValue);
                result    = System::result_t::ok;
                writeToDb = false;
            }
        }
        break;

        default:
            break;
        }
        break;
    }
    break;

    default:
        break;
    }

    if ((result == System::result_t::ok) && writeToDb)
        result = _database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;

    return result;
}

System::result_t System::onSetButtons(Section::button_t section, size_t index, uint16_t newValue)
{
#ifdef BUTTONS_SUPPORTED
    auto result = System::result_t::error;

    //channels start from 0 in db, start from 1 in sysex
    if (section == Section::button_t::midiChannel)
        newValue--;

    result = _database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;

    if (result == System::result_t::ok)
    {
        if (
            (section == Section::button_t::type) ||
            (section == Section::button_t::midiMessage))
            _buttons.reset(index);
    }

    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onSetEncoders(Section::encoder_t section, size_t index, uint16_t newValue)
{
#ifdef ENCODERS_SUPPORTED
    switch (section)
    {
    case Section::encoder_t::midiID_MSB:
        return System::result_t::notSupported;

    default:
    {
        //channels start from 0 in db, start from 1 in sysex
        if (section == Section::encoder_t::midiChannel)
            newValue--;
    }
    break;
    }

    auto result = _database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;

    if (result == System::result_t::ok)
        _encoders.resetValue(index);

    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onSetAnalog(Section::analog_t section, size_t index, uint16_t newValue)
{
#ifdef ANALOG_SUPPORTED
    switch (section)
    {
    case Section::analog_t::midiID_MSB:
    case Section::analog_t::lowerLimit_MSB:
    case Section::analog_t::upperLimit_MSB:
        return System::result_t::notSupported;

    case Section::analog_t::type:
    {
        _analog.debounceReset(index);
    }
    break;

    default:
    {
        //channels start from 0 in db, start from 1 in sysex
        if (section == Section::analog_t::midiChannel)
            newValue--;
    }
    break;
    }

    return _database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onSetLEDs(Section::leds_t section, size_t index, uint16_t newValue)
{
#ifdef LEDS_SUPPORTED
    auto result = System::result_t::error;

    bool writeToDb = true;

    switch (section)
    {
    case Section::leds_t::testColor:
    {
        //no writing to database
        _leds.setColor(index, static_cast<IO::LEDs::color_t>(newValue), IO::LEDs::brightness_t::b100);
        result    = System::result_t::ok;
        writeToDb = false;
    }
    break;

    case Section::leds_t::testBlink:
    {
        //no writing to database
        _leds.setBlinkSpeed(index, newValue ? IO::LEDs::blinkSpeed_t::s500ms : IO::LEDs::blinkSpeed_t::noBlink);
        result    = System::result_t::ok;
        writeToDb = false;
    }
    break;

    case Section::leds_t::global:
    {
        auto ledSetting = static_cast<IO::LEDs::setting_t>(index);

        switch (ledSetting)
        {
        case IO::LEDs::setting_t::blinkWithMIDIclock:
        {
            if ((newValue <= 1) && (newValue >= 0))
            {
                result = System::result_t::ok;
                _leds.setBlinkType(static_cast<IO::LEDs::blinkType_t>(newValue));
            }
        }
        break;

        case IO::LEDs::setting_t::useStartupAnimation:
        {
            if ((newValue <= 1) && (newValue >= 0))
                result = System::result_t::ok;
        }
        break;

        case IO::LEDs::setting_t::unused:
        {
            result = System::result_t::ok;
        }
        break;

        default:
            break;
        }

        //write to db if success is true and writing should take place
        if ((result == System::result_t::ok) && writeToDb)
            result = _database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;
    }
    break;

    case Section::leds_t::rgbEnable:
    {
        //make sure to turn all three leds off before setting new state
        _leds.setColor(_leds.rgbSignalIndex(_leds.rgbIndex(index), IO::LEDs::rgbIndex_t::r), IO::LEDs::color_t::off, IO::LEDs::brightness_t::bOff);
        _leds.setColor(_leds.rgbSignalIndex(_leds.rgbIndex(index), IO::LEDs::rgbIndex_t::g), IO::LEDs::color_t::off, IO::LEDs::brightness_t::bOff);
        _leds.setColor(_leds.rgbSignalIndex(_leds.rgbIndex(index), IO::LEDs::rgbIndex_t::b), IO::LEDs::color_t::off, IO::LEDs::brightness_t::bOff);

        //write rgb enabled bit to led
        result = _database.update(dbSection(section), _leds.rgbIndex(index), newValue) ? System::result_t::ok : System::result_t::error;

        if (newValue && (result == System::result_t::ok))
        {
            //copy over note activation local control and midi channel settings to all three leds from the current led index

            for (int i = 0; i < 3; i++)
            {
                result = _database.update(dbSection(Section::leds_t::activationID),
                                          _leds.rgbSignalIndex(_leds.rgbIndex(index), static_cast<IO::LEDs::rgbIndex_t>(i)),
                                          _database.read(dbSection(Section::leds_t::activationID), index))
                             ? System::result_t::ok
                             : System::result_t::error;

                if (result != System::result_t::ok)
                    break;

                result = _database.update(dbSection(Section::leds_t::controlType),
                                          _leds.rgbSignalIndex(_leds.rgbIndex(index), static_cast<IO::LEDs::rgbIndex_t>(i)),
                                          _database.read(dbSection(Section::leds_t::controlType), index))
                             ? System::result_t::ok
                             : System::result_t::error;

                if (result != System::result_t::ok)
                    break;

                result = _database.update(dbSection(Section::leds_t::midiChannel),
                                          _leds.rgbSignalIndex(_leds.rgbIndex(index), static_cast<IO::LEDs::rgbIndex_t>(i)),
                                          _database.read(dbSection(Section::leds_t::midiChannel), index))
                             ? System::result_t::ok
                             : System::result_t::error;

                if (result != System::result_t::ok)
                    break;
            }
        }
    }
    break;

    case Section::leds_t::activationID:
    case Section::leds_t::controlType:
    case Section::leds_t::midiChannel:
    {
        //channels start from 0 in db, start from 1 in sysex
        if (section == Section::leds_t::midiChannel)
            newValue--;

        //first, find out if RGB led is enabled for this led index
        if (_database.read(dbSection(Section::leds_t::rgbEnable), _leds.rgbIndex(index)))
        {
            //rgb led enabled - copy these settings to all three leds
            for (int i = 0; i < 3; i++)
            {
                result = _database.update(dbSection(section),
                                          _leds.rgbSignalIndex(_leds.rgbIndex(index), static_cast<IO::LEDs::rgbIndex_t>(i)),
                                          newValue)
                             ? System::result_t::ok
                             : System::result_t::error;

                if (result != System::result_t::ok)
                    break;
            }
        }
        else
        {
            //apply to single led only
            result = _database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;
        }
    }
    break;

    default:
    {
        result = _database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;
    }
    break;
    }

    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onSetDisplay(Section::display_t section, size_t index, uint16_t newValue)
{
#ifdef DISPLAY_SUPPORTED
    auto initAction = initAction_t::asIs;

    switch (section)
    {
    case Section::display_t::features:
    {
        auto feature = static_cast<IO::Display::feature_t>(index);

        switch (feature)
        {
        case IO::Display::feature_t::enable:
        {
            initAction = newValue ? initAction_t::init : initAction_t::deInit;
        }
        break;

        case IO::Display::feature_t::MIDInotesAlternate:
        {
            _display.setAlternateNoteDisplay(newValue);
        }
        break;

        default:
            break;
        }
    }
    break;

    case Section::display_t::setting:
    {
        auto setting = static_cast<IO::Display::setting_t>(index);

        switch (setting)
        {
        case IO::Display::setting_t::controller:
        {
            if ((newValue <= static_cast<uint8_t>(IO::U8X8::displayController_t::AMOUNT)) && (newValue >= 0))
            {
                initAction = initAction_t::init;
            }
        }
        break;

        case IO::Display::setting_t::resolution:
        {
            if ((newValue <= static_cast<uint8_t>(IO::U8X8::displayResolution_t::AMOUNT)) && (newValue >= 0))
            {
                initAction = initAction_t::init;
            }
        }
        break;

        case IO::Display::setting_t::MIDIeventTime:
        {
            _display.setRetentionTime(newValue * 1000);
        }
        break;

        case IO::Display::setting_t::octaveNormalization:
        {
            _display.setOctaveNormalization(newValue);
        }
        break;

        case IO::Display::setting_t::i2cAddress:
        {
            initAction = initAction_t::init;
        }
        break;

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    auto result = _database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;

    if (initAction == initAction_t::init)
        _display.init(false);
    else if (initAction == initAction_t::deInit)
        _display.deInit();

    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onSetTouchscreen(Section::touchscreen_t section, size_t index, uint16_t newValue)
{
#ifdef TOUCHSCREEN_SUPPORTED
    auto initAction = initAction_t::asIs;

    switch (section)
    {
    case Section::touchscreen_t::setting:
    {
        switch (index)
        {
        case static_cast<size_t>(IO::Touchscreen::setting_t::enable):
        {
            if (newValue)
                initAction = initAction_t::init;
            else
                initAction = initAction_t::deInit;
            break;
        }
        break;

        case static_cast<size_t>(IO::Touchscreen::setting_t::model):
        {
            initAction = initAction_t::init;
        }
        break;

        case static_cast<size_t>(IO::Touchscreen::setting_t::brightness):
        {
            if (_touchscreen.isInitialized())
            {
                if (!_touchscreen.setBrightness(static_cast<IO::Touchscreen::brightness_t>(newValue)))
                    return System::result_t::error;
            }
        }
        break;

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    bool result = _database.update(dbSection(section), index, newValue);

    if (result)
    {
        if (initAction == initAction_t::init)
            _touchscreen.init();
        else if (initAction == initAction_t::deInit)
            _touchscreen.deInit();

        return System::result_t::ok;
    }
    else
    {
        return System::result_t::error;
    }
#else
    return System::result_t::notSupported;
#endif
}