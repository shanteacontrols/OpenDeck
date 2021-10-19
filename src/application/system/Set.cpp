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

uint8_t System::SysExDataHandler::set(uint8_t  block,
                                      uint8_t  section,
                                      uint16_t index,
                                      uint16_t newValue)
{
    auto sysExBlock = static_cast<block_t>(block);
    auto result     = static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

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

    return result;
}

uint8_t System::onSetGlobal(Section::global_t section, size_t index, uint16_t newValue)
{
    uint8_t result            = SysExConf::DataHandler::STATUS_ERROR_RW;
    bool    writeToDb         = true;
    auto    dinMIDIinitAction = initAction_t::asIs;
    auto    dmxInitAction     = initAction_t::asIs;

    switch (section)
    {
    case Section::global_t::midiFeatures:
    {
        auto feature = static_cast<System::midiFeature_t>(index);

        switch (feature)
        {
        case System::midiFeature_t::runningStatus:
        {
            if (_hwa.protocol().midi().dinSupported())
            {
                if (!isMIDIfeatureEnabled(midiFeature_t::dinEnabled) && _hwa.serialPeripheralAllocated(serialPeripheral_t::dinMIDI) && _backupRestoreState != backupRestoreState_t::none)
                {
                    result = SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    _midi.setRunningStatusState(newValue);
                    result = SysExConf::DataHandler::STATUS_OK;
                }
            }
            else
            {
                result = static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);
            }
        }
        break;

        case System::midiFeature_t::standardNoteOff:
        {
            if (newValue)
                _midi.setNoteOffMode(MIDI::noteOffType_t::standardNoteOff);
            else
                _midi.setNoteOffMode(MIDI::noteOffType_t::noteOnZeroVel);

            result = SysExConf::DataHandler::STATUS_OK;
        }
        break;

        case System::midiFeature_t::dinEnabled:
        {
            if (_hwa.protocol().midi().dinSupported())
            {
                if (!isMIDIfeatureEnabled(midiFeature_t::dinEnabled) && _hwa.serialPeripheralAllocated(serialPeripheral_t::dinMIDI) && _backupRestoreState != backupRestoreState_t::none)
                {
                    result = SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    if (newValue)
                        dinMIDIinitAction = initAction_t::init;
                    else
                        dinMIDIinitAction = initAction_t::deInit;

                    result = SysExConf::DataHandler::STATUS_OK;
                }
            }
            else
            {
                result = static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);
            }
        }
        break;

        case System::midiFeature_t::mergeEnabled:
        {
            if (_hwa.protocol().midi().dinSupported())
            {
                if (!isMIDIfeatureEnabled(midiFeature_t::dinEnabled) && _hwa.serialPeripheralAllocated(serialPeripheral_t::dinMIDI) && _backupRestoreState != backupRestoreState_t::none)
                {
                    result = SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    result = SysExConf::DataHandler::STATUS_OK;

                    if (isMIDIfeatureEnabled(midiFeature_t::dinEnabled))
                        dinMIDIinitAction = initAction_t::init;
                }
            }
            else
            {
                result = static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);
            }
        }
        break;

        default:
        {
            result = SysExConf::DataHandler::STATUS_OK;
        }
        break;
        }
    }
    break;

    case Section::global_t::midiMerge:
    {
        if (_hwa.protocol().midi().dinSupported())
        {
            if (!isMIDIfeatureEnabled(midiFeature_t::dinEnabled) && _hwa.serialPeripheralAllocated(serialPeripheral_t::dinMIDI) && _backupRestoreState != backupRestoreState_t::none)
            {
                result = SERIAL_PERIPHERAL_ALLOCATED_ERROR;
            }
            else
            {
                auto mergeParam = static_cast<midiMerge_t>(index);

                switch (mergeParam)
                {
                case midiMerge_t::mergeType:
                {
                    if ((newValue >= 0) && (newValue < static_cast<size_t>(midiMergeType_t::AMOUNT)))
                    {
                        result = SysExConf::DataHandler::STATUS_OK;

                        if (isMIDIfeatureEnabled(midiFeature_t::dinEnabled))
                            dinMIDIinitAction = initAction_t::init;
                    }
                    else
                    {
                        result = static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);
                    }
                }
                break;

                case midiMerge_t::mergeUSBchannel:
                case midiMerge_t::mergeDINchannel:
                {
                    // unused for now
                    writeToDb = false;
                    result    = SysExConf::DataHandler::STATUS_OK;
                }
                break;

                default:
                    break;
                }
            }
        }
        else
        {
            result = static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);
        }
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
                result    = SysExConf::DataHandler::STATUS_OK;
                writeToDb = false;
            }
            else
            {
                result = static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);
            }
        }
        break;

        case presetSetting_t::presetPreserve:
        {
            if ((newValue <= 1) && (newValue >= 0))
            {
                _database.setPresetPreserveState(newValue);
                result    = SysExConf::DataHandler::STATUS_OK;
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

    case Section::global_t::dmx:
    {
        if (!_hwa.protocol().dmx().supported())
            return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

        bool dmxEnabled = _database.read(dbSection(section), dmxSetting_t::enabled);

        if (!dmxEnabled && _hwa.serialPeripheralAllocated(serialPeripheral_t::dmx) && _backupRestoreState != backupRestoreState_t::none)
        {
            return SERIAL_PERIPHERAL_ALLOCATED_ERROR;
        }
        else
        {
            if (_touchscreen.isInitialized(IO::Touchscreen::mode_t::cdcPassthrough))
            {
                if (_backupRestoreState != backupRestoreState_t::none)
                {
                    return CDC_ALLOCATED_ERROR;
                }
            }

            dmxInitAction = newValue ? initAction_t::init : initAction_t::deInit;
            result        = SysExConf::DataHandler::STATUS_OK;
        }
    }
    break;

    default:
        break;
    }

    if ((result == SysExConf::DataHandler::STATUS_OK) && writeToDb)
    {
        result = _database.update(dbSection(section), index, newValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;

        switch (dinMIDIinitAction)
        {
        case initAction_t::init:
        {
            _midi.init(MIDI::interface_t::din);
        }
        break;

        case initAction_t::deInit:
        {
            _midi.deInit(MIDI::interface_t::din);
        }
        break;

        default:
            break;
        }

        switch (dmxInitAction)
        {
        case initAction_t::init:
        {
            _dmx.init();
        }
        break;

        case initAction_t::deInit:
        {
            _dmx.deInit();
        }
        break;

        default:
            break;
        }
    }

    return result;
}

uint8_t System::onSetButtons(Section::button_t section, size_t index, uint16_t newValue)
{
    if (!_hwa.io().buttons().supported())
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    uint8_t result = SysExConf::DataHandler::STATUS_ERROR_RW;

    // channels start from 0 in db, start from 1 in sysex
    if (section == Section::button_t::midiChannel)
        newValue--;

    result = _database.update(dbSection(section), index, newValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;

    if (result == SysExConf::DataHandler::STATUS_OK)
    {
        if (
            (section == Section::button_t::type) ||
            (section == Section::button_t::midiMessage))
            _buttons.reset(index);
    }

    return result;
}

uint8_t System::onSetEncoders(Section::encoder_t section, size_t index, uint16_t newValue)
{
    if (!_hwa.io().encoders().supported())
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    switch (section)
    {
    case Section::encoder_t::midiID_MSB:
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    default:
    {
        // channels start from 0 in db, start from 1 in sysex
        if (section == Section::encoder_t::midiChannel)
            newValue--;
    }
    break;
    }

    auto result = _database.update(dbSection(section), index, newValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;

    if (result == SysExConf::DataHandler::STATUS_OK)
        _encoders.resetValue(index);

    return result;
}

uint8_t System::onSetAnalog(Section::analog_t section, size_t index, uint16_t newValue)
{
    if (!_hwa.io().analog().supported())
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    switch (section)
    {
    case Section::analog_t::midiID_MSB:
    case Section::analog_t::lowerLimit_MSB:
    case Section::analog_t::upperLimit_MSB:
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    case Section::analog_t::type:
    {
        _analog.debounceReset(index);
    }
    break;

    default:
    {
        // channels start from 0 in db, start from 1 in sysex
        if (section == Section::analog_t::midiChannel)
            newValue--;
    }
    break;
    }

    return _database.update(dbSection(section), index, newValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;
}

uint8_t System::onSetLEDs(Section::leds_t section, size_t index, uint16_t newValue)
{
    if (!_hwa.io().leds().supported())
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    uint8_t result = SysExConf::DataHandler::STATUS_ERROR_RW;

    bool writeToDb = true;

    switch (section)
    {
    case Section::leds_t::testColor:
    {
        // no writing to database
        _leds.setColor(index, static_cast<IO::LEDs::color_t>(newValue), IO::LEDs::brightness_t::b100);
        result    = SysExConf::DataHandler::STATUS_OK;
        writeToDb = false;
    }
    break;

    case Section::leds_t::testBlink:
    {
        // no writing to database
        _leds.setBlinkSpeed(index, newValue ? IO::LEDs::blinkSpeed_t::s500ms : IO::LEDs::blinkSpeed_t::noBlink);
        result    = SysExConf::DataHandler::STATUS_OK;
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
                result = SysExConf::DataHandler::STATUS_OK;
                _leds.setBlinkType(static_cast<IO::LEDs::blinkType_t>(newValue));
            }
        }
        break;

        case IO::LEDs::setting_t::useStartupAnimation:
        {
            if ((newValue <= 1) && (newValue >= 0))
                result = SysExConf::DataHandler::STATUS_OK;
        }
        break;

        case IO::LEDs::setting_t::unused:
        {
            result = SysExConf::DataHandler::STATUS_OK;
        }
        break;

        default:
            break;
        }

        // write to db if success is true and writing should take place
        if ((result == SysExConf::DataHandler::STATUS_OK) && writeToDb)
            result = _database.update(dbSection(section), index, newValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;
    }
    break;

    case Section::leds_t::rgbEnable:
    {
        // make sure to turn all three leds off before setting new state
        _leds.setColor(_leds.rgbSignalIndex(_leds.rgbIndex(index), IO::LEDs::rgbIndex_t::r), IO::LEDs::color_t::off, IO::LEDs::brightness_t::bOff);
        _leds.setColor(_leds.rgbSignalIndex(_leds.rgbIndex(index), IO::LEDs::rgbIndex_t::g), IO::LEDs::color_t::off, IO::LEDs::brightness_t::bOff);
        _leds.setColor(_leds.rgbSignalIndex(_leds.rgbIndex(index), IO::LEDs::rgbIndex_t::b), IO::LEDs::color_t::off, IO::LEDs::brightness_t::bOff);

        // write rgb enabled bit to led
        result = _database.update(dbSection(section), _leds.rgbIndex(index), newValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;

        if (newValue && (result == SysExConf::DataHandler::STATUS_OK))
        {
            // copy over note activation local control and midi channel settings to all three leds from the current led index

            for (int i = 0; i < 3; i++)
            {
                result = _database.update(dbSection(Section::leds_t::activationID),
                                          _leds.rgbSignalIndex(_leds.rgbIndex(index), static_cast<IO::LEDs::rgbIndex_t>(i)),
                                          _database.read(dbSection(Section::leds_t::activationID), index))
                             ? SysExConf::DataHandler::STATUS_OK
                             : SysExConf::DataHandler::STATUS_ERROR_RW;

                if (result != SysExConf::DataHandler::STATUS_OK)
                    break;

                result = _database.update(dbSection(Section::leds_t::controlType),
                                          _leds.rgbSignalIndex(_leds.rgbIndex(index), static_cast<IO::LEDs::rgbIndex_t>(i)),
                                          _database.read(dbSection(Section::leds_t::controlType), index))
                             ? SysExConf::DataHandler::STATUS_OK
                             : SysExConf::DataHandler::STATUS_ERROR_RW;

                if (result != SysExConf::DataHandler::STATUS_OK)
                    break;

                result = _database.update(dbSection(Section::leds_t::midiChannel),
                                          _leds.rgbSignalIndex(_leds.rgbIndex(index), static_cast<IO::LEDs::rgbIndex_t>(i)),
                                          _database.read(dbSection(Section::leds_t::midiChannel), index))
                             ? SysExConf::DataHandler::STATUS_OK
                             : SysExConf::DataHandler::STATUS_ERROR_RW;

                if (result != SysExConf::DataHandler::STATUS_OK)
                    break;
            }
        }
    }
    break;

    case Section::leds_t::activationID:
    case Section::leds_t::controlType:
    case Section::leds_t::midiChannel:
    {
        // channels start from 0 in db, start from 1 in sysex
        if (section == Section::leds_t::midiChannel)
            newValue--;

        // first, find out if RGB led is enabled for this led index
        if (_database.read(dbSection(Section::leds_t::rgbEnable), _leds.rgbIndex(index)))
        {
            // rgb led enabled - copy these settings to all three leds
            for (int i = 0; i < 3; i++)
            {
                result = _database.update(dbSection(section),
                                          _leds.rgbSignalIndex(_leds.rgbIndex(index), static_cast<IO::LEDs::rgbIndex_t>(i)),
                                          newValue)
                             ? SysExConf::DataHandler::STATUS_OK
                             : SysExConf::DataHandler::STATUS_ERROR_RW;

                if (result != SysExConf::DataHandler::STATUS_OK)
                    break;
            }
        }
        else
        {
            // apply to single led only
            result = _database.update(dbSection(section), index, newValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;
        }
    }
    break;

    default:
    {
        result = _database.update(dbSection(section), index, newValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;
    }
    break;
    }

    return result;
}

uint8_t System::onSetDisplay(Section::display_t section, size_t index, uint16_t newValue)
{
    if (!_hwa.io().display().supported())
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

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

    auto result = _database.update(dbSection(section), index, newValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;

    if (initAction == initAction_t::init)
        _display.init(false);
    else if (initAction == initAction_t::deInit)
        _display.deInit();

    return result;
}

uint8_t System::onSetTouchscreen(Section::touchscreen_t section, size_t index, uint16_t newValue)
{
    if (!_hwa.io().touchscreen().supported())
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    if (!_touchscreen.isInitialized() && _hwa.serialPeripheralAllocated(serialPeripheral_t::touchscreen) && _backupRestoreState != backupRestoreState_t::none)
    {
        return SERIAL_PERIPHERAL_ALLOCATED_ERROR;
    }
    else
    {
        auto initAction = initAction_t::asIs;
        auto mode       = IO::Touchscreen::mode_t::normal;
        bool writeToDb  = true;

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
                        return SysExConf::DataHandler::STATUS_ERROR_RW;
                }
            }
            break;

            case static_cast<size_t>(IO::Touchscreen::setting_t::cdcPassthrough):
            {
                if (_database.read(Database::Section::global_t::dmx, dmxSetting_t::enabled))
                {
                    if (_backupRestoreState != backupRestoreState_t::none)
                    {
                        return CDC_ALLOCATED_ERROR;
                    }
                }

                mode       = IO::Touchscreen::mode_t::cdcPassthrough;
                initAction = newValue ? initAction_t::init : initAction_t::deInit;
                writeToDb  = false;
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

        bool result = true;

        if (writeToDb)
            result = _database.update(dbSection(section), index, newValue);

        if (result)
        {
            if (initAction == initAction_t::init)
                _touchscreen.init(mode);
            else if (initAction == initAction_t::deInit)
                _touchscreen.deInit(mode);

            return SysExConf::DataHandler::STATUS_OK;
        }
        else
        {
            return SysExConf::DataHandler::STATUS_ERROR_RW;
        }
    }
}