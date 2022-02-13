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
#include "sysex/src/SysExConf.h"

namespace System
{
    class Config
    {
        public:
        Config() = delete;

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

            enum class global_t : uint8_t
            {
                midiFeatures,
                midiMerge,
                presets,
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
                midiID_MSB,
                remoteSync,
                AMOUNT
            };

            enum class analog_t : uint8_t
            {
                enable,
                invert,
                type,
                midiID,
                midiID_MSB,
                lowerLimit,
                lowerLimit_MSB,
                upperLimit,
                upperLimit_MSB,
                midiChannel,
                lowerOffset,
                upperOffset,
                AMOUNT
            };

            enum class leds_t : uint8_t
            {
                testColor,
                testBlink,
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

        class status_t
        {
            public:
            status_t() = delete;

            // Since get/set config messages return uint8_t to allow for custom, user statuses,
            // redefine same status enum as in SysExConf.h but without enum class.
            // This enum also contains added custom errors.
            enum
            {
                request,               // 0x00
                ack,                   // 0x01
                errorStatus,           // 0x02
                errorConnection,       // 0x03
                errorWish,             // 0x04
                errorAmount,           // 0x05
                errorBlock,            // 0x06
                errorSection,          // 0x07
                errorPart,             // 0x08
                errorIndex,            // 0x09
                errorNewValue,         // 0x0A
                errorMessageLength,    // 0x0B
                errorWrite,            // 0x0C
                errorNotSupported,     // 0x0D
                errorRead,             // 0x0E
                serialPeripheralAllocatedError = 80,
                cdcAllocatedError              = 81,
            };
        };
    };
}    // namespace System