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

#include <inttypes.h>
#include <stddef.h>

namespace Database
{
    class Config
    {
        public:
        Config() = delete;

        static constexpr size_t MAX_PRESETS = 10;

        enum class block_t : uint8_t
        {
            GLOBAL,
            BUTTONS,
            ENCODERS,
            ANALOG,
            LEDS,
            I2C,
            TOUCHSCREEN,
            AMOUNT
        };

        class Section
        {
            public:
            Section() = delete;

            // internal
            enum class system_t : uint8_t
            {
                UID,
                PRESETS,
                AMOUNT
            };

            enum class global_t : uint8_t
            {
                MIDI_SETTINGS,
                DMX_SETTINGS,
                AMOUNT
            };

            enum class button_t : uint8_t
            {
                TYPE,
                MESSAGE_TYPE,
                MIDI_ID,
                VALUE,
                CHANNEL,
                AMOUNT
            };

            enum class encoder_t : uint8_t
            {
                ENABLE,
                INVERT,
                MODE,
                MIDI_ID,
                CHANNEL,
                PULSES_PER_STEP,
                ACCELERATION,
                REMOTE_SYNC,
                AMOUNT
            };

            enum class analog_t : uint8_t
            {
                ENABLE,
                INVERT,
                TYPE,
                MIDI_ID,
                LOWER_LIMIT,
                UPPER_LIMIT,
                CHANNEL,
                LOWER_OFFSET,
                UPPER_OFFSET,
                AMOUNT
            };

            enum class leds_t : uint8_t
            {
                GLOBAL,
                ACTIVATION_ID,
                RGB_ENABLE,
                CONTROL_TYPE,
                ACTIVATION_VALUE,
                CHANNEL,
                AMOUNT
            };

            enum class i2c_t : uint8_t
            {
                DISPLAY,
                AMOUNT
            };

            enum class touchscreen_t : uint8_t
            {
                SETTING,
                X_POS,
                Y_POS,
                WIDTH,
                HEIGHT,
                ON_SCREEN,
                OFF_SCREEN,
                PAGE_SWITCH_ENABLED,
                PAGE_SWITCH_INDEX,
                AMOUNT
            };
        };

        enum class presetSetting_t : uint8_t
        {
            ACTIVE_PRESET,
            PRESET_PRESERVE,
            AMOUNT
        };
    };
}    // namespace Database
