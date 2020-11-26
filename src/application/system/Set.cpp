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

#include "System.h"
#include "io/leds/LEDs.h"

SysExConf::DataHandler::result_t System::SysExDataHandler::set(uint8_t                     block,
                                                               uint8_t                     section,
                                                               size_t                      index,
                                                               SysExConf::sysExParameter_t newValue)
{
    auto sysExBlock = static_cast<block_t>(block);
    auto result     = System::result_t::notSupported;

    switch (sysExBlock)
    {
    case block_t::global:
    {
        result = system.onSetGlobal(static_cast<Section::global_t>(section), index, newValue);
    }
    break;

    case block_t::buttons:
    {
        result = system.onSetButtons(static_cast<Section::button_t>(section), index, newValue);
    }
    break;

    case block_t::encoders:
    {
        result = system.onSetEncoders(static_cast<Section::encoder_t>(section), index, newValue);
    }
    break;

    case block_t::analog:
    {
        result = system.onSetAnalog(static_cast<Section::analog_t>(section), index, newValue);
    }
    break;

    case block_t::leds:
    {
        result = system.onSetLEDs(static_cast<Section::leds_t>(section), index, newValue);
    }
    break;

    case block_t::display:
    {
        result = system.onSetDisplay(static_cast<Section::display_t>(section), index, newValue);
    }
    break;

    case block_t::touchscreen:
    {
        result = system.onSetTouchscreen(static_cast<Section::touchscreen_t>(section), index, newValue);
    }
    break;

    default:
        break;
    }

    system.display.displayMIDIevent(IO::Display::eventType_t::in,
                                    IO::Display::event_t::systemExclusive,
                                    0,
                                    0,
                                    0);

    return result;
}

System::result_t System::onSetGlobal(Section::global_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
    auto result    = System::result_t::error;
    bool writeToDb = true;

    switch (section)
    {
    case Section::global_t::midiFeature:
    {
        auto feature = static_cast<System::midiFeature_t>(index);

        switch (feature)
        {
        case System::midiFeature_t::runningStatus:
        {
#ifndef DIN_MIDI_SUPPORTED
            result = System::result_t::notSupported;
#else
            midi.setRunningStatusState(newValue);
            result = System::result_t::ok;
#endif
        }
        break;

        case System::midiFeature_t::standardNoteOff:
        {
            if (newValue)
                midi.setNoteOffMode(MIDI::noteOffType_t::standardNoteOff);
            else
                midi.setNoteOffMode(MIDI::noteOffType_t::noteOnZeroVel);

            result = System::result_t::ok;
        }
        break;

        case System::midiFeature_t::dinEnabled:
        {
#ifndef DIN_MIDI_SUPPORTED
            result = System::result_t::notSupported;
#else
            if (newValue)
                hwa.enableDINMIDI(false);
            else
                hwa.disableDINMIDI();

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
                    hwa.enableDINMIDI(false);
                    midi.useRecursiveParsing(false);
                }
            }
            else
            {
                //restore active settings if din midi is enabled
                if (isMIDIfeatureEnabled(midiFeature_t::dinEnabled))
                {
                    //use recursive parsing when merging is active
                    midi.useRecursiveParsing(true);
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
            if (newValue < database.getSupportedPresets())
            {
                database.setPreset(newValue);
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
                database.setPresetPreserveState(newValue);
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
        result = database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;

    return result;
}

System::result_t System::onSetButtons(Section::button_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
#ifdef BUTTONS_SUPPORTED
    auto result = System::result_t::error;

    //channels start from 0 in db, start from 1 in sysex
    if (section == Section::button_t::midiChannel)
        newValue--;

    result = database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;

    if (result == System::result_t::ok)
    {
        if (
            (section == Section::button_t::type) ||
            (section == Section::button_t::midiMessage))
            buttons.reset(index);
    }

    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onSetEncoders(Section::encoder_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
#ifdef ENCODERS_SUPPORTED
    switch (section)
    {
    case Section::encoder_t::midiID_MSB:

        if (sysExConf.paramSize() == SysExConf::paramSize_t::_14bit)
        {
            //no need for MSB parameters in 2-byte mode since the entire value
            //can be set via single value
            return System::result_t::notSupported;
        }

        //intentional fall-through

    case Section::encoder_t::midiID:
    {
        if (sysExConf.paramSize() == SysExConf::paramSize_t::_7bit)
        {
            MIDI::encDec_14bit_t encDec_14bit;

            encDec_14bit.value = database.read(dbSection(section), index);
            encDec_14bit.split14bit();

            switch (section)
            {
            case Section::encoder_t::midiID:
            {
                encDec_14bit.low = newValue;
            }
            break;

            default:
            {
                encDec_14bit.high = newValue;
            }
            break;
            }

            encDec_14bit.mergeTo14bit();
            newValue = encDec_14bit.value;
        }
    }
    break;

    default:
    {
        //channels start from 0 in db, start from 1 in sysex
        if (section == Section::encoder_t::midiChannel)
            newValue--;
    }
    break;
    }

    auto result = database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;

    if (result == System::result_t::ok)
        encoders.resetValue(index);

    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onSetAnalog(Section::analog_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
#ifdef ANALOG_SUPPORTED
    switch (section)
    {
    case Section::analog_t::midiID_MSB:
    case Section::analog_t::lowerLimit_MSB:
    case Section::analog_t::upperLimit_MSB:

        if (sysExConf.paramSize() == SysExConf::paramSize_t::_14bit)
        {
            //no need for MSB parameters in 2-byte mode since the entire value
            //can be set via single value
            return System::result_t::notSupported;
        }

        //intentional fall-through

    case Section::analog_t::midiID:
    case Section::analog_t::lowerLimit:
    case Section::analog_t::upperLimit:
    {
        if (sysExConf.paramSize() == SysExConf::paramSize_t::_7bit)
        {
            MIDI::encDec_14bit_t encDec_14bit;

            encDec_14bit.value = database.read(dbSection(section), index);
            encDec_14bit.split14bit();

            switch (section)
            {
            case Section::analog_t::midiID:
            case Section::analog_t::lowerLimit:
            case Section::analog_t::upperLimit:
            {
                encDec_14bit.low = newValue;
            }
            break;

            default:
            {
                encDec_14bit.high = newValue;
            }
            break;
            }

            encDec_14bit.mergeTo14bit();
            newValue = encDec_14bit.value;
        }
    }
    break;

    case Section::analog_t::type:
    {
        analog.debounceReset(index);
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

    return database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onSetLEDs(Section::leds_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
#ifdef LEDS_SUPPORTED
    auto result = System::result_t::error;

    bool writeToDb = true;

    switch (section)
    {
    case Section::leds_t::testColor:
    {
        //no writing to database
        leds.setColor(index, static_cast<IO::LEDs::color_t>(newValue), IO::LEDs::brightness_t::b100);
        result    = System::result_t::ok;
        writeToDb = false;
    }
    break;

    case Section::leds_t::testBlink:
    {
        //no writing to database
        leds.setBlinkState(index, newValue ? IO::LEDs::blinkSpeed_t::s500ms : IO::LEDs::blinkSpeed_t::noBlink);
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
                leds.setBlinkType(static_cast<IO::LEDs::blinkType_t>(newValue));
            }
        }
        break;

        case IO::LEDs::setting_t::useStartupAnimation:
        {
            if ((newValue <= 1) && (newValue >= 0))
                result = System::result_t::ok;
        }
        break;

        case IO::LEDs::setting_t::fadeSpeed:
        {
            if (leds.setFadeSpeed(newValue))
                result = System::result_t::ok;
        }
        break;

        default:
            break;
        }

        //write to db if success is true and writing should take place
        if ((result == System::result_t::ok) && writeToDb)
            result = database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;
    }
    break;

    case Section::leds_t::rgbEnable:
    {
        //make sure to turn all three leds off before setting new state
        leds.setColor(leds.rgbSingleComponentIndex(leds.rgbIndex(index), IO::LEDs::rgbIndex_t::r), IO::LEDs::color_t::off, IO::LEDs::brightness_t::bOff);
        leds.setColor(leds.rgbSingleComponentIndex(leds.rgbIndex(index), IO::LEDs::rgbIndex_t::g), IO::LEDs::color_t::off, IO::LEDs::brightness_t::bOff);
        leds.setColor(leds.rgbSingleComponentIndex(leds.rgbIndex(index), IO::LEDs::rgbIndex_t::b), IO::LEDs::color_t::off, IO::LEDs::brightness_t::bOff);

        //write rgb enabled bit to led
        result = database.update(dbSection(section), leds.rgbIndex(index), newValue) ? System::result_t::ok : System::result_t::error;

        if (newValue && (result == System::result_t::ok))
        {
            //copy over note activation local control and midi channel settings to all three leds from the current led index

            for (int i = 0; i < 3; i++)
            {
                result = database.update(dbSection(Section::leds_t::activationID),
                                         leds.rgbSingleComponentIndex(leds.rgbIndex(index), static_cast<IO::LEDs::rgbIndex_t>(i)),
                                         database.read(dbSection(Section::leds_t::activationID), index))
                             ? System::result_t::ok
                             : System::result_t::error;

                if (result != System::result_t::ok)
                    break;

                result = database.update(dbSection(Section::leds_t::controlType),
                                         leds.rgbSingleComponentIndex(leds.rgbIndex(index), static_cast<IO::LEDs::rgbIndex_t>(i)),
                                         database.read(dbSection(Section::leds_t::controlType), index))
                             ? System::result_t::ok
                             : System::result_t::error;

                if (result != System::result_t::ok)
                    break;

                result = database.update(dbSection(Section::leds_t::midiChannel),
                                         leds.rgbSingleComponentIndex(leds.rgbIndex(index), static_cast<IO::LEDs::rgbIndex_t>(i)),
                                         database.read(dbSection(Section::leds_t::midiChannel), index))
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
        if (database.read(dbSection(Section::leds_t::rgbEnable), leds.rgbIndex(index)))
        {
            //rgb led enabled - copy these settings to all three leds
            for (int i = 0; i < 3; i++)
            {
                result = database.update(dbSection(section),
                                         leds.rgbSingleComponentIndex(leds.rgbIndex(index), static_cast<IO::LEDs::rgbIndex_t>(i)),
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
            result = database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;
        }
    }
    break;

    default:
    {
        result = database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;
    }
    break;
    }

    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onSetDisplay(Section::display_t section, size_t index, SysExConf::sysExParameter_t newValue)
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
            display.setAlternateNoteDisplay(newValue);
        }

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
            display.setRetentionTime(newValue * 1000);
        }
        break;

        case IO::Display::setting_t::octaveNormalization:
        {
            display.setOctaveNormalization(newValue);
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

    auto result = database.update(dbSection(section), index, newValue) ? System::result_t::ok : System::result_t::error;

    if (initAction == initAction_t::init)
        display.init(false);
    else if (initAction == initAction_t::deInit)
        display.deInit();

    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onSetTouchscreen(Section::touchscreen_t section, size_t index, SysExConf::sysExParameter_t newValue)
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

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    bool result = database.update(dbSection(section), index, newValue);

    if (result)
    {
        if (initAction == initAction_t::init)
            touchscreen.init();
        else if (initAction == initAction_t::deInit)
            touchscreen.deInit();

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