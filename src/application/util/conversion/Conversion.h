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

#pragma once

#include "database/Database.h"
#include "system/Config.h"
#include "sysex/src/SysExConf.h"

namespace Util
{
    // Various helper functions used to convert between system config sections to
    // database sections.
    class Conversion
    {
        public:
        Conversion() = delete;

        // alias split/merge classes from sysexconf to be globally available through this class
        using Split14bit = SysExConf::Split14bit;
        using Merge14bit = SysExConf::Merge14bit;

        static constexpr Database::Config::Section::global_t sys2DBsection(System::Config::Section::global_t section)
        {
            return SYS_EX2_DB_GLOBAL[static_cast<uint8_t>(section)];
        }

        static constexpr Database::Config::Section::button_t sys2DBsection(System::Config::Section::button_t section)
        {
            return SYS_EX2_DB_BUTTON[static_cast<uint8_t>(section)];
        }

        static constexpr Database::Config::Section::encoder_t sys2DBsection(System::Config::Section::encoder_t section)
        {
            return SYS_EX2_DB_ENCODER[static_cast<uint8_t>(section)];
        }

        static constexpr Database::Config::Section::analog_t sys2DBsection(System::Config::Section::analog_t section)
        {
            return SYS_EX2_DB_ANALOG[static_cast<uint8_t>(section)];
        }

        static constexpr Database::Config::Section::leds_t sys2DBsection(System::Config::Section::leds_t section)
        {
            return SYS_EX2_DB_LEDS[static_cast<uint8_t>(section)];
        }

        static constexpr Database::Config::Section::i2c_t sys2DBsection(System::Config::Section::i2c_t section)
        {
            return SYS_EX2_DB_I2C[static_cast<uint8_t>(section)];
        }

        static constexpr Database::Config::Section::touchscreen_t sys2DBsection(System::Config::Section::touchscreen_t section)
        {
            return SYS_EX2_DB_TOUCHSCREEN[static_cast<uint8_t>(section)];
        }

        private:
        static constexpr Database::Config::Section::global_t SYS_EX2_DB_GLOBAL[static_cast<uint8_t>(System::Config::Section::global_t::AMOUNT)] = {
            Database::Config::Section::global_t::MIDI_SETTINGS,
            Database::Config::Section::global_t::AMOUNT,    // blank/reserved
            Database::Config::Section::global_t::AMOUNT,    // unused
            Database::Config::Section::global_t::DMX_SETTINGS,
            Database::Config::Section::global_t::AMOUNT,    // unused
        };

        static constexpr Database::Config::Section::button_t SYS_EX2_DB_BUTTON[static_cast<uint8_t>(System::Config::Section::button_t::AMOUNT)] = {
            Database::Config::Section::button_t::TYPE,
            Database::Config::Section::button_t::MESSAGE_TYPE,
            Database::Config::Section::button_t::MIDI_ID,
            Database::Config::Section::button_t::VALUE,
            Database::Config::Section::button_t::CHANNEL
        };

        static constexpr Database::Config::Section::encoder_t SYS_EX2_DB_ENCODER[static_cast<uint8_t>(System::Config::Section::encoder_t::AMOUNT)] = {
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

        static constexpr Database::Config::Section::analog_t SYS_EX2_DB_ANALOG[static_cast<uint8_t>(System::Config::Section::analog_t::AMOUNT)] = {
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

        static constexpr Database::Config::Section::leds_t SYS_EX2_DB_LEDS[static_cast<uint8_t>(System::Config::Section::leds_t::AMOUNT)] = {
            Database::Config::Section::leds_t::AMOUNT,
            Database::Config::Section::leds_t::AMOUNT,
            Database::Config::Section::leds_t::GLOBAL,
            Database::Config::Section::leds_t::ACTIVATION_ID,
            Database::Config::Section::leds_t::RGB_ENABLE,
            Database::Config::Section::leds_t::CONTROL_TYPE,
            Database::Config::Section::leds_t::ACTIVATION_VALUE,
            Database::Config::Section::leds_t::CHANNEL,
        };

        static constexpr Database::Config::Section::i2c_t SYS_EX2_DB_I2C[static_cast<uint8_t>(System::Config::Section::i2c_t::AMOUNT)] = {
            Database::Config::Section::i2c_t::DISPLAY,
        };

        static constexpr Database::Config::Section::touchscreen_t SYS_EX2_DB_TOUCHSCREEN[static_cast<uint8_t>(System::Config::Section::touchscreen_t::AMOUNT)] = {
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
    };
}    // namespace Util