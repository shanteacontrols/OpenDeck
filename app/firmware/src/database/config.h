/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>
#include <stddef.h>

namespace database
{
    /**
     * @brief Compile-time identifiers and limits for database blocks and sections.
     */
    class Config
    {
        public:
        /**
         * @brief Prevents instantiation of this compile-time database configuration holder.
         */
        Config() = delete;

        /** @brief Maximum number of custom common settings reserved for applications. */
        static constexpr size_t MAX_CUSTOM_COMMON_SETTINGS = 10;

        /**
         * @brief Identifies one top-level database block.
         */
        enum class Block : uint8_t
        {
            Global,
            Buttons,
            Encoders,
            Analog,
            Leds,
            I2c,
            Touchscreen,
            Count
        };

        /**
         * @brief Groups section identifiers by database block.
         */
        class Section
        {
            public:
            /**
             * @brief Prevents instantiation of this section-identifier namespace.
             */
            Section() = delete;

            /**
             * @brief Identifies sections stored in the common block.
             */
            enum class Common : uint8_t
            {
                CommonSettings,
                Count
            };

            /**
             * @brief Identifies sections stored in the global preset block.
             */
            enum class Global : uint8_t
            {
                MidiSettings,
                Count
            };

            /**
             * @brief Identifies sections stored in the button preset block.
             */
            enum class Button : uint8_t
            {
                Type,
                MessageType,
                MidiId,
                Value,
                Channel,
                Count
            };

            /**
             * @brief Identifies sections stored in the encoder preset block.
             */
            enum class Encoder : uint8_t
            {
                Enable,
                Invert,
                Mode,
                MidiId1,
                Channel,
                PulsesPerStep,
                Acceleration,
                RemoteSync,
                LowerLimit,
                UpperLimit,
                RepeatedValue,
                MidiId2,
                Count
            };

            /**
             * @brief Identifies sections stored in the analog preset block.
             */
            enum class Analog : uint8_t
            {
                Enable,
                Invert,
                Type,
                MidiId,
                LowerLimit,
                UpperLimit,
                Channel,
                LowerOffset,
                UpperOffset,
                Count
            };

            /**
             * @brief Identifies sections stored in the LED preset block.
             */
            enum class Leds : uint8_t
            {
                Global,
                ActivationId,
                RgbEnable,
                ControlType,
                ActivationValue,
                Channel,
                Count
            };

            /**
             * @brief Identifies sections stored in the I2C preset block.
             */
            enum class I2c : uint8_t
            {
                Display,
                Count
            };

            /**
             * @brief Identifies sections stored in the touchscreen preset block.
             */
            enum class Touchscreen : uint8_t
            {
                Setting,
                XPos,
                YPos,
                Width,
                Height,
                OnScreen,
                OffScreen,
                PageSwitchEnabled,
                PageSwitchIndex,
                Count
            };
        };

        /**
         * @brief Identifies common settings stored outside preset-specific data.
         */
        enum class CommonSetting : uint8_t
        {
            ActivePreset,
            PresetPreserve,
            CustomCommonSettingStart,
            CustomCommonSettingEnd = CustomCommonSettingStart + MAX_CUSTOM_COMMON_SETTINGS,
            Uid,
            Count
        };
    };
}    // namespace database
