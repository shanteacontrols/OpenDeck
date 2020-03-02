#include "SysConfig.h"

bool SysConfig::onGet(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t& value)
{
    auto sysExBlock = static_cast<SysConfig::block_t>(block);
    bool success    = false;

    switch (sysExBlock)
    {
    case SysConfig::block_t::global:
        success = onGetGlobal(static_cast<SysConfig::Section::global_t>(section), index, value);
        break;

    case SysConfig::block_t::buttons:
        success = onGetButtons(static_cast<SysConfig::Section::button_t>(section), index, value);
        break;

    case SysConfig::block_t::encoders:
        success = onGetEncoders(static_cast<SysConfig::Section::encoder_t>(section), index, value);
        break;

    case SysConfig::block_t::analog:
        success = onGetAnalog(static_cast<SysConfig::Section::analog_t>(section), index, value);
        break;

    case SysConfig::block_t::leds:
        success = onGetLEDs(static_cast<SysConfig::Section::leds_t>(section), index, value);
        break;

    case SysConfig::block_t::display:
        success = onGetDisplay(static_cast<SysConfig::Section::display_t>(section), index, value);
        break;

    default:
        break;
    }

#ifdef DISPLAY_SUPPORTED
    display.displayMIDIevent(Interface::Display::eventType_t::in, Interface::Display::event_t::systemExclusive, 0, 0, 0);
#endif

    return success;
}

bool SysConfig::onGetGlobal(Section::global_t section, size_t index, SysExConf::sysExParameter_t& value)
{
    int32_t readValue = 0;
    bool    success   = false;

    switch (section)
    {
    case Section::global_t::midiFeature:
    case Section::global_t::midiMerge:
        success = database.read(dbSection(section), index, readValue);
        break;

    case Section::global_t::presets:
    {
        auto setting = static_cast<presetSetting_t>(index);

        switch (setting)
        {
        case presetSetting_t::activePreset:
            readValue = database.getPreset();
            success   = true;
            break;

        case presetSetting_t::presetPreserve:
            readValue = database.getPresetPreserveState();
            success   = true;
            break;

        default:
            success = false;
            break;
        }
    }
    break;

    default:
        success = false;
        break;
    }

    value = readValue;
    return success;
}

bool SysConfig::onGetButtons(Section::button_t section, size_t index, SysExConf::sysExParameter_t& value)
{
    int32_t readValue;
    bool    success = database.read(dbSection(section), index, readValue);

    //channels start from 0 in db, start from 1 in sysex
    if ((section == Section::button_t::midiChannel) && success)
        readValue++;

    value = readValue;
    return success;
}

bool SysConfig::onGetEncoders(Section::encoder_t section, size_t index, SysExConf::sysExParameter_t& value)
{
    int32_t              readValue;
    bool                 success = database.read(dbSection(section), index, readValue);
    MIDI::encDec_14bit_t encDec_14bit;

    if (success)
    {
        if ((section == Section::encoder_t::midiID) || (section == Section::encoder_t::midiID_msb))
        {
            encDec_14bit.value = readValue;
            encDec_14bit.split14bit();

            if (section == Section::encoder_t::midiID)
                readValue = encDec_14bit.low;
            else
                readValue = encDec_14bit.high;
        }
        else if (section == Section::encoder_t::midiChannel)
        {
            //channels start from 0 in db, start from 1 in sysex
            readValue++;
        }
    }

    value = readValue;
    return success;
}

bool SysConfig::onGetAnalog(Section::analog_t section, size_t index, SysExConf::sysExParameter_t& value)
{
    int32_t              readValue;
    bool                 success = database.read(dbSection(section), index, readValue);
    MIDI::encDec_14bit_t encDec_14bit;

    switch (section)
    {
    case Section::analog_t::midiID:
    case Section::analog_t::midiID_MSB:
    case Section::analog_t::lowerLimit:
    case Section::analog_t::lowerLimit_MSB:
    case Section::analog_t::upperLimit:
    case Section::analog_t::upperLimit_MSB:
        if (success)
        {
            encDec_14bit.value = readValue;
            encDec_14bit.split14bit();

            switch (section)
            {
            case Section::analog_t::midiID:
            case Section::analog_t::lowerLimit:
            case Section::analog_t::upperLimit:
                readValue = encDec_14bit.low;
                break;

            default:
                readValue = encDec_14bit.high;
                break;
            }
        }
        break;

    case Section::analog_t::midiChannel:
        //channels start from 0 in db, start from 1 in sysex
        if (success)
            readValue++;
        break;

    default:
        break;
    }

    value = readValue;
    return success;
}

bool SysConfig::onGetLEDs(Section::leds_t section, size_t index, SysExConf::sysExParameter_t& value)
{
#ifdef LEDS_SUPPORTED
    int32_t readValue;
    bool    success = true;

    switch (section)
    {
    case Section::leds_t::testColor:
        readValue = static_cast<int32_t>(leds.getColor(index));
        break;

    case Section::leds_t::testBlink:
        readValue = leds.getBlinkState(index);
        break;

    case Section::leds_t::midiChannel:
        success = database.read(dbSection(section), index, readValue);

        //channels start from 0 in db, start from 1 in sysex
        if (success)
            readValue++;
        break;

    case Section::leds_t::rgbEnable:
        success = database.read(dbSection(section), Board::io::getRGBID(index), readValue);
        break;

    default:
        success = database.read(dbSection(section), index, readValue);
        break;
    }

    value = readValue;
    return success;
#else
    setError(SysExConf::status_t::errorNotSupported);
    return false;
#endif
}

bool SysConfig::onGetDisplay(Section::display_t section, size_t index, SysExConf::sysExParameter_t& value)
{
#ifdef DISPLAY_SUPPORTED
    int32_t readValue;
    bool    success = database.read(dbSection(section), index, readValue);

    value = readValue;
    return success;
#else
    setError(SysExConf::status_t::errorNotSupported);
    return false;
#endif
}