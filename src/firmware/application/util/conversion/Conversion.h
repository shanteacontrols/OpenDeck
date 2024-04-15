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

#include "application/database/Database.h"
#include "application/system/Config.h"
#include "SysExConf/SysExConf.h"

namespace util
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

        static constexpr database::Config::Section::global_t SYS_2_DB_SECTION(sys::Config::Section::global_t section)
        {
            return SYS_EX2_DB_GLOBAL[static_cast<uint8_t>(section)];
        }

        static constexpr database::Config::Section::button_t SYS_2_DB_SECTION(sys::Config::Section::button_t section)
        {
            return SYS_EX2_DB_BUTTON[static_cast<uint8_t>(section)];
        }

        static constexpr database::Config::Section::encoder_t SYS_2_DB_SECTION(sys::Config::Section::encoder_t section)
        {
            return SYS_EX2_DB_ENCODER[static_cast<uint8_t>(section)];
        }

        static constexpr database::Config::Section::analog_t SYS_2_DB_SECTION(sys::Config::Section::analog_t section)
        {
            return SYS_EX2_DB_ANALOG[static_cast<uint8_t>(section)];
        }

        static constexpr database::Config::Section::leds_t SYS_2_DB_SECTION(sys::Config::Section::leds_t section)
        {
            return SYS_EX2_DB_LEDS[static_cast<uint8_t>(section)];
        }

        static constexpr database::Config::Section::i2c_t SYS_2_DB_SECTION(sys::Config::Section::i2c_t section)
        {
            return SYS_EX2_DB_I2C[static_cast<uint8_t>(section)];
        }

        static constexpr database::Config::Section::touchscreen_t SYS_2_DB_SECTION(sys::Config::Section::touchscreen_t section)
        {
            return SYS_EX2_DB_TOUCHSCREEN[static_cast<uint8_t>(section)];
        }

        private:
        static constexpr database::Config::Section::global_t SYS_EX2_DB_GLOBAL[static_cast<uint8_t>(sys::Config::Section::global_t::AMOUNT)] = {
            database::Config::Section::global_t::MIDI_SETTINGS,
            database::Config::Section::global_t::AMOUNT,    // blank/reserved
            database::Config::Section::global_t::AMOUNT,    // unused
        };

        static constexpr database::Config::Section::button_t SYS_EX2_DB_BUTTON[static_cast<uint8_t>(sys::Config::Section::button_t::AMOUNT)] = {
            database::Config::Section::button_t::TYPE,
            database::Config::Section::button_t::MESSAGE_TYPE,
            database::Config::Section::button_t::MIDI_ID,
            database::Config::Section::button_t::VALUE,
            database::Config::Section::button_t::CHANNEL
        };

        static constexpr database::Config::Section::encoder_t SYS_EX2_DB_ENCODER[static_cast<uint8_t>(sys::Config::Section::encoder_t::AMOUNT)] = {
            database::Config::Section::encoder_t::ENABLE,
            database::Config::Section::encoder_t::INVERT,
            database::Config::Section::encoder_t::MODE,
            database::Config::Section::encoder_t::MIDI_ID,
            database::Config::Section::encoder_t::CHANNEL,
            database::Config::Section::encoder_t::PULSES_PER_STEP,
            database::Config::Section::encoder_t::ACCELERATION,
            database::Config::Section::encoder_t::MIDI_ID,
            database::Config::Section::encoder_t::REMOTE_SYNC,
            database::Config::Section::encoder_t::LOWER_LIMIT,
            database::Config::Section::encoder_t::UPPER_LIMIT,
            database::Config::Section::encoder_t::REPEATED_VALUE,
        };

        static constexpr database::Config::Section::analog_t SYS_EX2_DB_ANALOG[static_cast<uint8_t>(sys::Config::Section::analog_t::AMOUNT)] = {
            database::Config::Section::analog_t::ENABLE,
            database::Config::Section::analog_t::INVERT,
            database::Config::Section::analog_t::TYPE,
            database::Config::Section::analog_t::MIDI_ID,
            database::Config::Section::analog_t::MIDI_ID,
            database::Config::Section::analog_t::LOWER_LIMIT,
            database::Config::Section::analog_t::LOWER_LIMIT,
            database::Config::Section::analog_t::UPPER_LIMIT,
            database::Config::Section::analog_t::UPPER_LIMIT,
            database::Config::Section::analog_t::CHANNEL,
            database::Config::Section::analog_t::LOWER_OFFSET,
            database::Config::Section::analog_t::UPPER_OFFSET,
        };

        static constexpr database::Config::Section::leds_t SYS_EX2_DB_LEDS[static_cast<uint8_t>(sys::Config::Section::leds_t::AMOUNT)] = {
            database::Config::Section::leds_t::AMOUNT,
            database::Config::Section::leds_t::AMOUNT,
            database::Config::Section::leds_t::GLOBAL,
            database::Config::Section::leds_t::ACTIVATION_ID,
            database::Config::Section::leds_t::RGB_ENABLE,
            database::Config::Section::leds_t::CONTROL_TYPE,
            database::Config::Section::leds_t::ACTIVATION_VALUE,
            database::Config::Section::leds_t::CHANNEL,
        };

        static constexpr database::Config::Section::i2c_t SYS_EX2_DB_I2C[static_cast<uint8_t>(sys::Config::Section::i2c_t::AMOUNT)] = {
            database::Config::Section::i2c_t::DISPLAY,
        };

        static constexpr database::Config::Section::touchscreen_t SYS_EX2_DB_TOUCHSCREEN[static_cast<uint8_t>(sys::Config::Section::touchscreen_t::AMOUNT)] = {
            database::Config::Section::touchscreen_t::SETTING,
            database::Config::Section::touchscreen_t::X_POS,
            database::Config::Section::touchscreen_t::Y_POS,
            database::Config::Section::touchscreen_t::WIDTH,
            database::Config::Section::touchscreen_t::HEIGHT,
            database::Config::Section::touchscreen_t::ON_SCREEN,
            database::Config::Section::touchscreen_t::OFF_SCREEN,
            database::Config::Section::touchscreen_t::PAGE_SWITCH_ENABLED,
            database::Config::Section::touchscreen_t::PAGE_SWITCH_INDEX,
        };
    };
}    // namespace util