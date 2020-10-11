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

System::result_t System::SysExDataHandler::get(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t& value)
{
    auto sysExBlock = static_cast<System::block_t>(block);
    auto result     = System::result_t::notSupported;

    switch (sysExBlock)
    {
    case System::block_t::global:
    {
        result = system.onGetGlobal(static_cast<System::Section::global_t>(section), index, value);
    }
    break;

    case System::block_t::buttons:
    {
        result = system.onGetButtons(static_cast<System::Section::button_t>(section), index, value);
    }
    break;

    case System::block_t::encoders:
    {
        result = system.onGetEncoders(static_cast<System::Section::encoder_t>(section), index, value);
    }
    break;

    case System::block_t::analog:
    {
        result = system.onGetAnalog(static_cast<System::Section::analog_t>(section), index, value);
    }
    break;

    case System::block_t::leds:
    {
        result = system.onGetLEDs(static_cast<System::Section::leds_t>(section), index, value);
    }
    break;

    case System::block_t::display:
    {
        result = system.onGetDisplay(static_cast<System::Section::display_t>(section), index, value);
    }
    break;

    case System::block_t::touchscreen:
    {
        result = system.onGetTouchscreen(static_cast<System::Section::touchscreen_t>(section), index, value);
    }
    break;

    default:
        break;
    }

    system.display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::systemExclusive, 0, 0, 0);

    return result;
}

System::result_t System::onGetGlobal(Section::global_t section, size_t index, SysExConf::sysExParameter_t& value)
{
    int32_t readValue = 0;
    auto    result    = System::result_t::error;

    switch (section)
    {
    case Section::global_t::midiFeature:
    {
        if (index == static_cast<size_t>(System::midiFeature_t::standardNoteOff))
        {
            result = database.read(dbSection(section), index, readValue) ? System::result_t::ok : System::result_t::error;
        }
        else
        {
#ifndef DIN_MIDI_SUPPORTED
            return System::result_t::notSupported;
#else
            result = database.read(dbSection(section), index, readValue) ? System::result_t::ok : System::result_t::error;
#endif
        }
    }
    break;

    case Section::global_t::midiMerge:
    {
#ifndef DIN_MIDI_SUPPORTED
        return System::result_t::notSupported;
#else
        result = database.read(dbSection(section), index, readValue) ? System::result_t::ok : System::result_t::error;
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
            readValue = database.getPreset();
            result    = System::result_t::ok;
        }
        break;

        case presetSetting_t::presetPreserve:
        {
            readValue = database.getPresetPreserveState();
            result    = System::result_t::ok;
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

    value = readValue;
    return result;
}

System::result_t System::onGetButtons(Section::button_t section, size_t index, SysExConf::sysExParameter_t& value)
{
#ifdef BUTTONS_SUPPORTED
    int32_t readValue;
    auto    result = database.read(dbSection(section), index, readValue) ? System::result_t::ok : System::result_t::error;

    //channels start from 0 in db, start from 1 in sysex
    if ((section == Section::button_t::midiChannel) && (result == System::result_t::ok))
        readValue++;

    value = readValue;

    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onGetEncoders(Section::encoder_t section, size_t index, SysExConf::sysExParameter_t& value)
{
#ifdef ENCODERS_SUPPORTED
    int32_t readValue;
    auto    result = database.read(dbSection(section), index, readValue) ? System::result_t::ok : System::result_t::error;

    if (result == System::result_t::ok)
    {
        if (sysExConf.paramSize() == SysExConf::paramSize_t::_14bit)
        {
            if (section == Section::encoder_t::midiID_MSB)
                return System::result_t::notSupported;
        }
        else
        {
            if ((section == Section::encoder_t::midiID) || (section == Section::encoder_t::midiID_MSB))
            {
                MIDI::encDec_14bit_t encDec_14bit;

                encDec_14bit.value = readValue;
                encDec_14bit.split14bit();

                if (section == Section::encoder_t::midiID)
                    readValue = encDec_14bit.low;
                else
                    readValue = encDec_14bit.high;
            }
        }

        if (section == Section::encoder_t::midiChannel)
        {
            //channels start from 0 in db, start from 1 in sysex
            readValue++;
        }
    }

    value = readValue;

    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onGetAnalog(Section::analog_t section, size_t index, SysExConf::sysExParameter_t& value)
{
#ifdef ANALOG_SUPPORTED
    int32_t readValue;
    auto    result = database.read(dbSection(section), index, readValue) ? System::result_t::ok : System::result_t::error;

    switch (section)
    {
    case Section::analog_t::midiID_MSB:
    case Section::analog_t::lowerLimit_MSB:
    case Section::analog_t::upperLimit_MSB:

        if (sysExConf.paramSize() == SysExConf::paramSize_t::_14bit)
        {
            //no need for MSB parameters in 2-byte mode since the entire value
            //can be retrieved via single value
            return System::result_t::notSupported;
        }

        //intentional fall-through

    case Section::analog_t::midiID:
    case Section::analog_t::lowerLimit:
    case Section::analog_t::upperLimit:
    {
        if (sysExConf.paramSize() == SysExConf::paramSize_t::_7bit)
        {
            if (result == System::result_t::ok)
            {
                MIDI::encDec_14bit_t encDec_14bit;

                encDec_14bit.value = readValue;
                encDec_14bit.split14bit();

                switch (section)
                {
                case Section::analog_t::midiID:
                case Section::analog_t::lowerLimit:
                case Section::analog_t::upperLimit:
                {
                    readValue = encDec_14bit.low;
                }
                break;

                default:
                {
                    readValue = encDec_14bit.high;
                }
                break;
                }
            }
        }
        else
        {
            //nothing to do
        }
    }
    break;

    case Section::analog_t::midiChannel:
    {
        //channels start from 0 in db, start from 1 in sysex
        if (result == System::result_t::ok)
            readValue++;
    }
    break;

    default:
        break;
    }

    value = readValue;

    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onGetLEDs(Section::leds_t section, size_t index, SysExConf::sysExParameter_t& value)
{
#ifdef LEDS_SUPPORTED
    int32_t readValue;
    auto    result = System::result_t::ok;

    switch (section)
    {
    case Section::leds_t::testColor:
    {
        readValue = static_cast<int32_t>(leds.getColor(index));
    }
    break;

    case Section::leds_t::testBlink:
    {
        readValue = leds.getBlinkState(index);
    }
    break;

    case Section::leds_t::midiChannel:
    {
        result = database.read(dbSection(section), index, readValue) ? System::result_t::ok : System::result_t::error;

        //channels start from 0 in db, start from 1 in sysex
        if (result == System::result_t::ok)
            readValue++;
    }
    break;

    case Section::leds_t::rgbEnable:
    {
        result = database.read(dbSection(section), leds.rgbIndex(index), readValue) ? System::result_t::ok : System::result_t::error;
    }
    break;

    default:
    {
        result = database.read(dbSection(section), index, readValue) ? System::result_t::ok : System::result_t::error;
    }
    break;
    }

    value = readValue;

    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onGetDisplay(Section::display_t section, size_t index, SysExConf::sysExParameter_t& value)
{
#ifdef DISPLAY_SUPPORTED
    int32_t readValue;
    auto    result = database.read(dbSection(section), index, readValue) ? System::result_t::ok : System::result_t::error;

    value = readValue;
    return result;
#else
    return System::result_t::notSupported;
#endif
}

System::result_t System::onGetTouchscreen(Section::touchscreen_t section, size_t index, SysExConf::sysExParameter_t& value)
{
#ifdef TOUCHSCREEN_SUPPORTED
    int32_t readValue;
    auto    result = database.read(dbSection(section), index, readValue) ? System::result_t::ok : System::result_t::error;

    value = readValue;
    return result;
#else
    return System::result_t::notSupported;
#endif
}