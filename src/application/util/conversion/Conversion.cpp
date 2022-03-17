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

#include "Conversion.h"

namespace
{
    const Database::Config::Section::global_t _sysEx2DB_global[static_cast<uint8_t>(System::Config::Section::global_t::AMOUNT)] = {
        Database::Config::Section::global_t::MIDI_SETTINGS,
        Database::Config::Section::global_t::AMOUNT,    // blank/reserved
        Database::Config::Section::global_t::AMOUNT,    // unused
        Database::Config::Section::global_t::DMX_SETTINGS,
        Database::Config::Section::global_t::AMOUNT,    // unused
    };

    const Database::Config::Section::button_t _sysEx2DB_button[static_cast<uint8_t>(System::Config::Section::button_t::AMOUNT)] = {
        Database::Config::Section::button_t::TYPE,
        Database::Config::Section::button_t::MESSAGE_TYPE,
        Database::Config::Section::button_t::MIDI_ID,
        Database::Config::Section::button_t::VALUE,
        Database::Config::Section::button_t::CHANNEL
    };

    const Database::Config::Section::encoder_t _sysEx2DB_encoder[static_cast<uint8_t>(System::Config::Section::encoder_t::AMOUNT)] = {
        Database::Config::Section::encoder_t::ENABLE,
        Database::Config::Section::encoder_t::INVERT,
        Database::Config::Section::encoder_t::MODE,
        Database::Config::Section::encoder_t::MIDI_ID,
        Database::Config::Section::encoder_t::CHANNEL,
        Database::Config::Section::encoder_t::PULSES_PER_STEP,
        Database::Config::Section::encoder_t::ACCELERATION,
        Database::Config::Section::encoder_t::MIDI_ID,
        Database::Config::Section::encoder_t::REMOTE_SYNC
    };

    const Database::Config::Section::analog_t _sysEx2DB_analog[static_cast<uint8_t>(System::Config::Section::analog_t::AMOUNT)] = {
        Database::Config::Section::analog_t::ENABLE,
        Database::Config::Section::analog_t::INVERT,
        Database::Config::Section::analog_t::TYPE,
        Database::Config::Section::analog_t::MIDI_ID,
        Database::Config::Section::analog_t::MIDI_ID,
        Database::Config::Section::analog_t::LOWER_LIMIT,
        Database::Config::Section::analog_t::LOWER_LIMIT,
        Database::Config::Section::analog_t::UPPER_LIMIT,
        Database::Config::Section::analog_t::UPPER_LIMIT,
        Database::Config::Section::analog_t::CHANNEL,
        Database::Config::Section::analog_t::LOWER_OFFSET,
        Database::Config::Section::analog_t::UPPER_OFFSET,
    };

    const Database::Config::Section::leds_t _sysEx2DB_leds[static_cast<uint8_t>(System::Config::Section::leds_t::AMOUNT)] = {
        Database::Config::Section::leds_t::AMOUNT,
        Database::Config::Section::leds_t::AMOUNT,
        Database::Config::Section::leds_t::GLOBAL,
        Database::Config::Section::leds_t::ACTIVATION_ID,
        Database::Config::Section::leds_t::RGB_ENABLE,
        Database::Config::Section::leds_t::CONTROL_TYPE,
        Database::Config::Section::leds_t::ACTIVATION_VALUE,
        Database::Config::Section::leds_t::CHANNEL,
    };

    const Database::Config::Section::i2c_t _sysEx2DB_i2c[static_cast<uint8_t>(System::Config::Section::i2c_t::AMOUNT)] = {
        Database::Config::Section::i2c_t::DISPLAY,
    };

    const Database::Config::Section::touchscreen_t _sysEx2DB_touchscreen[static_cast<uint8_t>(System::Config::Section::touchscreen_t::AMOUNT)] = {
        Database::Config::Section::touchscreen_t::SETTING,
        Database::Config::Section::touchscreen_t::X_POS,
        Database::Config::Section::touchscreen_t::Y_POS,
        Database::Config::Section::touchscreen_t::WIDTH,
        Database::Config::Section::touchscreen_t::HEIGHT,
        Database::Config::Section::touchscreen_t::ON_SCREEN,
        Database::Config::Section::touchscreen_t::OFF_SCREEN,
        Database::Config::Section::touchscreen_t::PAGE_SWITCH_ENABLED,
        Database::Config::Section::touchscreen_t::PAGE_SWITCH_INDEX,
    };
}    // namespace

namespace Util::Conversion
{
    Database::Config::Section::global_t sys2DBsection(System::Config::Section::global_t section)
    {
        return _sysEx2DB_global[static_cast<uint8_t>(section)];
    }

    Database::Config::Section::button_t sys2DBsection(System::Config::Section::button_t section)
    {
        return _sysEx2DB_button[static_cast<uint8_t>(section)];
    }

    Database::Config::Section::encoder_t sys2DBsection(System::Config::Section::encoder_t section)
    {
        return _sysEx2DB_encoder[static_cast<uint8_t>(section)];
    }

    Database::Config::Section::analog_t sys2DBsection(System::Config::Section::analog_t section)
    {
        return _sysEx2DB_analog[static_cast<uint8_t>(section)];
    }

    Database::Config::Section::leds_t sys2DBsection(System::Config::Section::leds_t section)
    {
        return _sysEx2DB_leds[static_cast<uint8_t>(section)];
    }

    Database::Config::Section::i2c_t sys2DBsection(System::Config::Section::i2c_t section)
    {
        return _sysEx2DB_i2c[static_cast<uint8_t>(section)];
    }

    Database::Config::Section::touchscreen_t sys2DBsection(System::Config::Section::touchscreen_t section)
    {
        return _sysEx2DB_touchscreen[static_cast<uint8_t>(section)];
    }
}    // namespace Util::Conversion