#include "SysConfig.h"

bool SysConfig::onSet(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
    auto sysExBlock = static_cast<block_t>(block);
    bool success    = false;

    switch (sysExBlock)
    {
    case block_t::global:
        success = onSetGlobal(static_cast<Section::global_t>(section), index, newValue);
        break;

    case block_t::buttons:
        success = onSetButtons(static_cast<Section::button_t>(section), index, newValue);
        break;

    case block_t::encoders:
        success = onSetEncoders(static_cast<Section::encoder_t>(section), index, newValue);
        break;

    case block_t::analog:
        success = onSetAnalog(static_cast<Section::analog_t>(section), index, newValue);
        break;

    case block_t::leds:
        success = onSetLEDs(static_cast<Section::leds_t>(section), index, newValue);
        break;

    case block_t::display:
        success = onSetDisplay(static_cast<Section::display_t>(section), index, newValue);
        break;

    default:
        break;
    }

#ifdef DISPLAY_SUPPORTED
    display.displayMIDIevent(Interface::Display::eventType_t::in, Interface::Display::event_t::systemExclusive, 0, 0, 0);
#endif

    return success;
}

bool SysConfig::onSetGlobal(Section::global_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
    bool success   = false;
    bool writeToDb = true;

    switch (section)
    {
    case Section::global_t::midiFeature:
    {
        auto feature = static_cast<SysConfig::midiFeature_t>(index);

        switch (feature)
        {
        case SysConfig::midiFeature_t::runningStatus:
#ifndef DIN_MIDI_SUPPORTED
            setError(SysExConf::status_t::errorNotSupported);
#else
            midi.setRunningStatusState(newValue);
            success = true;
#endif
            break;

        case SysConfig::midiFeature_t::standardNoteOff:
            newValue ? midi.setNoteOffMode(MIDI::noteOffType_t::standardNoteOff) : midi.setNoteOffMode(MIDI::noteOffType_t::noteOnZeroVel);
            success = true;
            break;

        case SysConfig::midiFeature_t::dinEnabled:
#ifndef DIN_MIDI_SUPPORTED
            setError(SysExConf::status_t::errorNotSupported);
#else
            newValue ? setupMIDIoverUART(UART_BAUDRATE_MIDI_STD, true, true) : Board::UART::deInit(UART_MIDI_CHANNEL);
            success = true;
#endif
            break;

        case SysConfig::midiFeature_t::mergeEnabled:
#ifndef DIN_MIDI_SUPPORTED
            setError(SysExConf::status_t::errorNotSupported);
#else
            if (isMIDIfeatureEnabled(midiFeature_t::dinEnabled) || !newValue)
            {
                success = true;
                //use recursive parsing when merging is active
                midi.useRecursiveParsing(newValue);

                //make sure everything is in correct state
                if (!newValue)
                {
                    setupMIDIoverUART(UART_BAUDRATE_MIDI_STD, true, true);
                }
                else
                {
                    //restore active settings
                    auto mergeType = midiMergeType();

                    if (mergeType == midiMergeType_t::odSlave)
                        mergeType = midiMergeType_t::odSlaveInitial;

                    configureMIDImerge(mergeType);
                }
            }
            else
            {
                //invalid configuration - trying to enable merge functionality while din midi is disabled
                setError(SysExConf::status_t::errorWrite);
            }
#endif
            break;

        default:
            break;
        }
    }
    break;

    case Section::global_t::midiMerge:
#ifndef DIN_MIDI_SUPPORTED
        setError(SysExConf::status_t::errorNotSupported);
#else
    {
        auto mergeParam = static_cast<midiMerge_t>(index);

        switch (mergeParam)
        {
        case midiMerge_t::mergeType:
            if (isMIDIfeatureEnabled(midiFeature_t::dinEnabled) && isMIDIfeatureEnabled(midiFeature_t::mergeEnabled))
            {
                if ((newValue >= 0) && (newValue < static_cast<size_t>(midiMergeType_t::AMOUNT)))
                {
                    success = true;

                    if (static_cast<midiMergeType_t>(newValue) == midiMergeType_t::odSlave)
                        newValue = static_cast<SysExConf::sysExParameter_t>(midiMergeType_t::odSlaveInitial);

                    configureMIDImerge(static_cast<midiMergeType_t>(newValue));
                }
                else
                {
                    setError(SysExConf::status_t::errorNewValue);
                }
            }
            else
            {
                //invalid configuration
                setError(SysExConf::status_t::errorWrite);
            }
            break;

        case midiMerge_t::mergeUSBchannel:
        case midiMerge_t::mergeDINchannel:
            //unused for now
            writeToDb = false;
            success = true;
            break;

        default:
            break;
        }
    }
#endif
        break;

    case Section::global_t::presets:
    {
        auto setting = static_cast<presetSetting_t>(index);

        switch (setting)
        {
        case presetSetting_t::activePreset:
            if (newValue < database.getSupportedPresets())
            {
                database.setPreset(newValue);
                success   = true;
                writeToDb = false;
            }
            else
            {
                setError(SysExConf::status_t::errorNotSupported);
            }
            break;

        case presetSetting_t::presetPreserve:
        {
            if ((newValue <= 1) && (newValue >= 0))
            {
                database.setPresetPreserveState(newValue);
                success   = true;
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

    if (success && writeToDb)
        success = database.update(dbSection(section), index, newValue);

    return success;
}

bool SysConfig::onSetButtons(Section::button_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
    bool success = false;

    //channels start from 0 in db, start from 1 in sysex
    if (section == Section::button_t::midiChannel)
        newValue--;

    success = database.update(dbSection(section), index, newValue);

    if (success)
    {
        if (
            (section == Section::button_t::type) ||
            (section == Section::button_t::midiMessage))
            buttons.reset(index);
    }

    return success;
}

bool SysConfig::onSetEncoders(Section::encoder_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
    bool success = false;

    //channels start from 0 in db, start from 1 in sysex
    if (section == Section::encoder_t::midiChannel)
        newValue--;

    success = database.update(dbSection(section), index, newValue);
    encoders.resetValue(index);

    return success;
}

bool SysConfig::onSetAnalog(Section::analog_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
    bool                 success = false;
    MIDI::encDec_14bit_t encDec_14bit;

    switch (section)
    {
    case Section::analog_t::midiID:
    case Section::analog_t::midiID_MSB:
    case Section::analog_t::lowerLimit:
    case Section::analog_t::lowerLimit_MSB:
    case Section::analog_t::upperLimit:
    case Section::analog_t::upperLimit_MSB:
        encDec_14bit.value = database.read(dbSection(section), index);
        encDec_14bit.split14bit();

        switch (section)
        {
        case Section::analog_t::midiID:
        case Section::analog_t::lowerLimit:
        case Section::analog_t::upperLimit:
            encDec_14bit.low = newValue;
            break;

        default:
            encDec_14bit.high = newValue;
            break;
        }

        encDec_14bit.mergeTo14bit();
        success = database.update(dbSection(section), index, encDec_14bit.value);
        break;

    case Section::analog_t::type:
        analog.debounceReset(index);
        success = database.update(dbSection(section), index, newValue);
        break;

    default:
        //channels start from 0 in db, start from 1 in sysex
        if (section == Section::analog_t::midiChannel)
            newValue--;

        success = database.update(dbSection(section), index, newValue);
        break;
    }

    return success;
}

bool SysConfig::onSetLEDs(Section::leds_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
#ifdef LEDS_SUPPORTED
    bool success   = false;
    bool writeToDb = true;

    switch (section)
    {
    case Section::leds_t::testColor:
        //no writing to database
        leds.setColor(index, static_cast<Interface::digital::output::LEDs::color_t>(newValue));
        success   = true;
        writeToDb = false;
        break;

    case Section::leds_t::testBlink:
        //no writing to database
        leds.setBlinkState(index, newValue ? Interface::digital::output::LEDs::blinkSpeed_t::s500ms : Interface::digital::output::LEDs::blinkSpeed_t::noBlink);
        success   = true;
        writeToDb = false;
        break;

    case Section::leds_t::global:
        switch (static_cast<Interface::digital::output::LEDs::setting_t>(index))
        {
        case Interface::digital::output::LEDs::setting_t::blinkWithMIDIclock:
            if ((newValue <= 1) && (newValue >= 0))
            {
                success = true;
                leds.setBlinkType(static_cast<Interface::digital::output::LEDs::blinkType_t>(newValue));
            }
            break;

        case Interface::digital::output::LEDs::setting_t::useStartupAnimation:
            if ((newValue <= 1) && (newValue >= 0))
                success = true;
            break;

        case Interface::digital::output::LEDs::setting_t::fadeSpeed:
#ifdef LED_FADING
            if ((newValue >= FADE_TIME_MIN) && (newValue <= FADE_TIME_MAX))
            {
                leds.setFadeTime(newValue);
                success = true;
            }
#else
            setError(SysExConf::status_t::errorNotSupported);
#endif
            break;

        default:
            break;
        }

        //write to db if success is true and writing should take place
        if (success && writeToDb)
            success = database.update(dbSection(section), index, newValue);
        break;

    case Section::leds_t::rgbEnable:
        //make sure to turn all three leds off before setting new state
        leds.setColor(Board::io::getRGBaddress(Board::io::getRGBID(index), Interface::digital::output::LEDs::rgbIndex_t::r), Interface::digital::output::LEDs::color_t::off);
        leds.setColor(Board::io::getRGBaddress(Board::io::getRGBID(index), Interface::digital::output::LEDs::rgbIndex_t::g), Interface::digital::output::LEDs::color_t::off);
        leds.setColor(Board::io::getRGBaddress(Board::io::getRGBID(index), Interface::digital::output::LEDs::rgbIndex_t::b), Interface::digital::output::LEDs::color_t::off);

        //write rgb enabled bit to led
        success = database.update(dbSection(section), Board::io::getRGBID(index), newValue);

        if (newValue && success)
        {
            //copy over note activation local control and midi channel settings to all three leds from the current led index

            for (int i = 0; i < 3; i++)
            {
                success = database.update(dbSection(Section::leds_t::activationID), Board::io::getRGBaddress(Board::io::getRGBID(index), static_cast<Interface::digital::output::LEDs::rgbIndex_t>(i)), database.read(dbSection(Section::leds_t::activationID), index));

                if (!success)
                    break;

                success = database.update(dbSection(Section::leds_t::controlType), Board::io::getRGBaddress(Board::io::getRGBID(index), static_cast<Interface::digital::output::LEDs::rgbIndex_t>(i)), database.read(dbSection(Section::leds_t::controlType), index));

                if (!success)
                    break;

                success = database.update(dbSection(Section::leds_t::midiChannel), Board::io::getRGBaddress(Board::io::getRGBID(index), static_cast<Interface::digital::output::LEDs::rgbIndex_t>(i)), database.read(dbSection(Section::leds_t::midiChannel), index));

                if (!success)
                    break;
            }
        }
        break;

    case Section::leds_t::activationID:
    case Section::leds_t::controlType:
    case Section::leds_t::midiChannel:
        //channels start from 0 in db, start from 1 in sysex
        if (section == Section::leds_t::midiChannel)
            newValue--;

        //first, find out if RGB led is enabled for this led index
        if (database.read(dbSection(Section::leds_t::rgbEnable), Board::io::getRGBID(index)))
        {
            //rgb led enabled - copy these settings to all three leds
            for (int i = 0; i < 3; i++)
            {
                success = database.update(dbSection(section), Board::io::getRGBaddress(Board::io::getRGBID(index), static_cast<Interface::digital::output::LEDs::rgbIndex_t>(i)), newValue);

                if (!success)
                    break;
            }
        }
        else
        {
            //apply to single led only
            success = database.update(dbSection(section), index, newValue);
        }
        break;

    default:
        success = database.update(dbSection(section), index, newValue);
        break;
    }

    return success;
#else
    setError(SysExConf::status_t::errorNotSupported);
    return false;
#endif
}

bool SysConfig::onSetDisplay(Section::display_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
#ifdef DISPLAY_SUPPORTED
    bool init = false;

    switch (section)
    {
    case Section::display_t::features:
    {
        auto feature = static_cast<Interface::Display::feature_t>(index);

        switch (feature)
        {
        case Interface::Display::feature_t::enable:
            init = true;
            break;

        case Interface::Display::feature_t::MIDIeventRetention:
            display.setRetentionState(newValue);
            break;

        case Interface::Display::feature_t::MIDInotesAlternate:
            display.setAlternateNoteDisplay(newValue);
            break;

        default:
            break;
        }
    }
    break;

    case Section::display_t::setting:
    {
        auto setting = static_cast<Interface::Display::setting_t>(index);

        switch (setting)
        {
        case Interface::Display::setting_t::controller:
            if ((newValue <= static_cast<uint8_t>(U8X8::displayController_t::AMOUNT)) && (newValue >= 0))
            {
                init = true;
            }
            break;

        case Interface::Display::setting_t::resolution:
            if ((newValue <= static_cast<uint8_t>(U8X8::displayResolution_t::AMOUNT)) && (newValue >= 0))
            {
                init = true;
            }
            break;

        case Interface::Display::setting_t::MIDIeventTime:
            if ((newValue >= MIN_MESSAGE_RETENTION_TIME) && (newValue <= MAX_MESSAGE_RETENTION_TIME))
            {
                display.setRetentionTime(newValue * 1000);
            }
            break;

        case Interface::Display::setting_t::octaveNormalization:
            display.setOctaveNormalization(newValue);
            break;

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    bool success = database.update(dbSection(section), index, newValue);

    if (init)
        display.init(false);

    return success;
#else
    setError(SysExConf::status_t::errorNotSupported);
    return false;
#endif
}