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
#include <vector>
#include <functional>
#include <optional>
#include "SysExConf/SysExConf.h"

namespace sys
{
    class Config
    {
        public:
        Config() = delete;

        static constexpr uint8_t SYSEX_MANUFACTURER_ID_0 = 0x00;
        static constexpr uint8_t SYSEX_MANUFACTURER_ID_1 = 0x53;
        static constexpr uint8_t SYSEX_MANUFACTURER_ID_2 = 0x43;

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

            enum class global_t : uint8_t
            {
                MIDI_SETTINGS,
                RESERVED,    // compatibility
                PRESETS,
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
                MIDI_ID_MSB,
                REMOTE_SYNC,
                LOWER_LIMIT,
                UPPER_LIMIT,
                AMOUNT
            };

            enum class analog_t : uint8_t
            {
                ENABLE,
                INVERT,
                TYPE,
                MIDI_ID,
                MIDI_ID_MSB,
                LOWER_LIMIT,
                LOWER_LIMIT_MSB,
                UPPER_LIMIT,
                UPPER_LIMIT_MSB,
                CHANNEL,
                LOWER_OFFSET,
                UPPER_OFFSET,
                AMOUNT
            };

            enum class leds_t : uint8_t
            {
                TEST_COLOR,
                TEST_BLINK,
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

        struct status_t
        {
            // Since get/set config messages return uint8_t to allow for custom, user statuses,
            // redefine same status enum as in SysExConf.h but without enum class.
            // This enum also contains added custom errors.
            enum
            {
                REQUEST,                 // 0x00
                ACK,                     // 0x01
                ERROR_STATUS,            // 0x02
                ERROR_CONNECTION,        // 0x03
                ERROR_WISH,              // 0x04
                ERROR_AMOUNT,            // 0x05
                ERROR_BLOCK,             // 0x06
                ERROR_SECTION,           // 0x07
                ERROR_PART,              // 0x08
                ERROR_INDEX,             // 0x09
                ERROR_NEW_VALUE,         // 0x0A
                ERROR_MESSAGE_LENGTH,    // 0x0B
                ERROR_WRITE,             // 0x0C
                ERROR_NOT_SUPPORTED,     // 0x0D
                ERROR_READ,              // 0x0E
                SERIAL_PERIPHERAL_ALLOCATED_ERROR = 80,
                CDC_ALLOCATED_ERROR               = 81,
            };
        };
    };
}    // namespace sys