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

uint8_t System::SysExDataHandler::get(uint8_t block, uint8_t section, uint16_t index, uint16_t& value)
{
    auto sysExBlock = static_cast<System::block_t>(block);
    auto result     = static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    switch (sysExBlock)
    {
    case System::block_t::global:
    {
        result = _system.onGetGlobal(static_cast<System::Section::global_t>(section), index, value);
    }
    break;

    case System::block_t::buttons:
    {
        result = _system.onGetButtons(static_cast<System::Section::button_t>(section), index, value);
    }
    break;

    case System::block_t::encoders:
    {
        result = _system.onGetEncoders(static_cast<System::Section::encoder_t>(section), index, value);
    }
    break;

    case System::block_t::analog:
    {
        result = _system.onGetAnalog(static_cast<System::Section::analog_t>(section), index, value);
    }
    break;

    case System::block_t::leds:
    {
        result = _system.onGetLEDs(static_cast<System::Section::leds_t>(section), index, value);
    }
    break;

    case System::block_t::display:
    {
        result = _system.onGetDisplay(static_cast<System::Section::display_t>(section), index, value);
    }
    break;

    case System::block_t::touchscreen:
    {
        result = _system.onGetTouchscreen(static_cast<System::Section::touchscreen_t>(section), index, value);
    }
    break;

    default:
        break;
    }

    return result;
}

uint8_t System::onGetGlobal(Section::global_t section, size_t index, uint16_t& value)
{
    int32_t readValue = 0;
    uint8_t result    = SysExConf::DataHandler::STATUS_ERROR_RW;

    switch (section)
    {
    case Section::global_t::midiFeatures:
    {
        if (index == static_cast<size_t>(System::midiFeature_t::standardNoteOff))
        {
            result = _database.read(dbSection(section), index, readValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;
        }
        else
        {
            if (_hwa.protocol().midi().dinSupported())
            {
                if (!isMIDIfeatureEnabled(midiFeature_t::dinEnabled) && _hwa.serialPeripheralAllocated(serialPeripheral_t::dinMIDI) && _backupRestoreState != backupRestoreState_t::none)
                {
                    return SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    result = _database.read(dbSection(section), index, readValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;
                }
            }
            else
            {
                result = static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);
            }
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
                result = _database.read(dbSection(section), index, readValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;
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
            readValue = _database.getPreset();
            result    = SysExConf::DataHandler::STATUS_OK;
        }
        break;

        case presetSetting_t::presetPreserve:
        {
            readValue = _database.getPresetPreserveState();
            result    = SysExConf::DataHandler::STATUS_OK;
        }
        break;

        default:
            break;
        }
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

            result = _database.read(dbSection(section), index, readValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;
        }
    }
    break;

    default:
        break;
    }

    value = readValue;
    return result;
}

uint8_t System::onGetButtons(Section::button_t section, size_t index, uint16_t& value)
{
    if (!_hwa.io().buttons().supported())
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    int32_t readValue;
    auto    result = _database.read(dbSection(section), index, readValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;

    // channels start from 0 in db, start from 1 in sysex
    if ((section == Section::button_t::midiChannel) && (result == SysExConf::DataHandler::STATUS_OK))
        readValue++;

    value = readValue;

    return result;
}

uint8_t System::onGetEncoders(Section::encoder_t section, size_t index, uint16_t& value)
{
    if (!_hwa.io().encoders().supported())
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    int32_t readValue;
    auto    result = _database.read(dbSection(section), index, readValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;

    if (result == SysExConf::DataHandler::STATUS_OK)
    {
        if (section == Section::encoder_t::midiID_MSB)
            return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

        if (section == Section::encoder_t::midiChannel)
        {
            // channels start from 0 in db, start from 1 in sysex
            readValue++;
        }
    }

    value = readValue;

    return result;
}

uint8_t System::onGetAnalog(Section::analog_t section, size_t index, uint16_t& value)
{
    if (!_hwa.io().analog().supported())
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    int32_t readValue;
    auto    result = _database.read(dbSection(section), index, readValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;

    switch (section)
    {
    case Section::analog_t::midiID_MSB:
    case Section::analog_t::lowerLimit_MSB:
    case Section::analog_t::upperLimit_MSB:
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    case Section::analog_t::midiChannel:
    {
        // channels start from 0 in db, start from 1 in sysex
        if (result == SysExConf::DataHandler::STATUS_OK)
            readValue++;
    }
    break;

    default:
        break;
    }

    value = readValue;

    return result;
}

uint8_t System::onGetLEDs(Section::leds_t section, size_t index, uint16_t& value)
{
    if (!_hwa.io().leds().supported())
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    int32_t readValue;
    auto    result = SysExConf::DataHandler::STATUS_OK;

    switch (section)
    {
    case Section::leds_t::testColor:
    {
        readValue = static_cast<int32_t>(_leds.color(index));
    }
    break;

    case Section::leds_t::testBlink:
    {
        readValue = _leds.blinkSpeed(index) != IO::LEDs::blinkSpeed_t::noBlink;
    }
    break;

    case Section::leds_t::midiChannel:
    {
        result = _database.read(dbSection(section), index, readValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;

        // channels start from 0 in db, start from 1 in sysex
        if (result == SysExConf::DataHandler::STATUS_OK)
            readValue++;
    }
    break;

    case Section::leds_t::rgbEnable:
    {
        result = _database.read(dbSection(section), _leds.rgbIndex(index), readValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;
    }
    break;

    default:
    {
        result = _database.read(dbSection(section), index, readValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;
    }
    break;
    }

    value = readValue;

    return result;
}

uint8_t System::onGetDisplay(Section::display_t section, size_t index, uint16_t& value)
{
    if (!_hwa.io().display().supported())
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    int32_t readValue;
    auto    result = _database.read(dbSection(section), index, readValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;

    value = readValue;

    return result;
}

uint8_t System::onGetTouchscreen(Section::touchscreen_t section, size_t index, uint16_t& value)
{
    if (!_hwa.io().touchscreen().supported())
        return static_cast<uint8_t>(SysExConf::status_t::errorNotSupported);

    if (!_touchscreen.isInitialized() && _hwa.serialPeripheralAllocated(serialPeripheral_t::touchscreen) && _backupRestoreState != backupRestoreState_t::none)
    {
        return SERIAL_PERIPHERAL_ALLOCATED_ERROR;
    }
    else
    {
        switch (section)
        {
        case Section::touchscreen_t::setting:
        {
            switch (index)
            {
            case static_cast<size_t>(IO::Touchscreen::setting_t::cdcPassthrough):
            {
                if (_database.read(Database::Section::global_t::dmx, dmxSetting_t::enabled))
                {
                    if (_backupRestoreState != backupRestoreState_t::none)
                    {
                        return CDC_ALLOCATED_ERROR;
                    }
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

        int32_t readValue;
        auto    result = _database.read(dbSection(section), index, readValue) ? SysExConf::DataHandler::STATUS_OK : SysExConf::DataHandler::STATUS_ERROR_RW;

        value = readValue;
        return result;
    }
}