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
            global,
            buttons,
            encoders,
            analog,
            leds,
            i2c,
            touchscreen,
            AMOUNT
        };

        class Section
        {
            public:
            Section() = delete;

            // internal
            enum class system_t : uint8_t
            {
                uid,
                presets,
                AMOUNT
            };

            enum class global_t : uint8_t
            {
                midiSettings,
                dmx,
                AMOUNT
            };

            enum class button_t : uint8_t
            {
                type,
                midiMessage,
                midiID,
                velocity,
                midiChannel,
                AMOUNT
            };

            enum class encoder_t : uint8_t
            {
                enable,
                invert,
                mode,
                midiID,
                midiChannel,
                pulsesPerStep,
                acceleration,
                remoteSync,
                AMOUNT
            };

            enum class analog_t : uint8_t
            {
                enable,
                invert,
                type,
                midiID,
                lowerLimit,
                upperLimit,
                midiChannel,
                lowerOffset,
                upperOffset,
                AMOUNT
            };

            enum class leds_t : uint8_t
            {
                global,
                activationID,
                rgbEnable,
                controlType,
                activationValue,
                midiChannel,
                AMOUNT
            };

            enum class i2c_t : uint8_t
            {
                display,
                AMOUNT
            };

            enum class touchscreen_t : uint8_t
            {
                setting,
                xPos,
                yPos,
                width,
                height,
                onScreen,
                offScreen,
                pageSwitchEnabled,
                pageSwitchIndex,
                analogPage,
                analogStartXCoordinate,
                analogEndXCoordinate,
                analogStartYCoordinate,
                analogEndYCoordinate,
                analogType,
                analogResetOnRelease,
                AMOUNT
            };
        };

        enum class presetSetting_t : uint8_t
        {
            activePreset,
            presetPreserve,
            AMOUNT
        };
    };
}    // namespace Database
