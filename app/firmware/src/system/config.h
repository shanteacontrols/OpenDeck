/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "database/config.h"

#include "zlibs/utils/sysex_conf/sysex_conf.h"

namespace opendeck::sys
{
    /**
     * @brief Compile-time identifiers and limits for the system SysEx configuration protocol.
     */
    class Config
    {
        public:
        /**
         * @brief Prevents instantiation of this compile-time configuration holder.
         */
        Config() = delete;

        /**
         * @brief First byte of the OpenDeck SysEx manufacturer identifier.
         */
        static constexpr uint8_t SYSEX_MANUFACTURER_ID_0 = 0x00;

        /**
         * @brief Second byte of the OpenDeck SysEx manufacturer identifier.
         */
        static constexpr uint8_t SYSEX_MANUFACTURER_ID_1 = 0x53;

        /**
         * @brief Third byte of the OpenDeck SysEx manufacturer identifier.
         */
        static constexpr uint8_t SYSEX_MANUFACTURER_ID_2 = 0x43;

        /**
         * @brief Identifies one top-level SysEx configuration block.
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
         * @brief Groups section identifiers by SysEx configuration block.
         */
        class Section
        {
            public:
            /**
             * @brief Prevents instantiation of this section-identifier namespace.
             */
            Section() = delete;

            /**
             * @brief Identifies global SysEx configuration sections.
             */
            enum class Global : uint8_t
            {
                MidiSettings,
                Reserved,    // compatibility
                SystemSettings,
                OscSettings,
                MdnsHostname,
                Count
            };

            /**
             * @brief Identifies button SysEx configuration sections.
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
             * @brief Identifies encoder SysEx configuration sections.
             */
            enum class Encoder : uint8_t
            {
                Enable,
                Invert,
                Mode,
                MidiId1,
                Channel,
                Reserved2,
                Acceleration,
                Reserved1,
                RemoteSync,
                LowerLimit,
                UpperLimit,
                RepeatedValue,
                MidiId2,
                Count
            };

            /**
             * @brief Identifies analog SysEx configuration sections.
             */
            enum class Analog : uint8_t
            {
                Enable,
                Invert,
                Type,
                MidiId,
                Reserved1,
                LowerLimit,
                Reserved2,
                UpperLimit,
                Reserved3,
                Channel,
                LowerOffset,
                UpperOffset,
                Count
            };

            /**
             * @brief Identifies LED SysEx configuration sections.
             */
            enum class Leds : uint8_t
            {
                TestColor,
                TestBlink,
                Global,
                ActivationId,
                RgbEnable,
                ControlType,
                ActivationValue,
                Channel,
                Count
            };

            /**
             * @brief Identifies I2C SysEx configuration sections.
             */
            enum class I2c : uint8_t
            {
                Display,
                Count
            };

            /**
             * @brief Identifies touchscreen SysEx configuration sections.
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
         * @brief Identifies system-wide settings exposed through the SysEx protocol.
         */
        enum class SystemSetting : uint8_t
        {
            ActivePreset                          = static_cast<uint8_t>(database::Config::CommonSetting::ActivePreset),
            PresetPreserve                        = static_cast<uint8_t>(database::Config::CommonSetting::PresetPreserve),
            DisableForcedRefreshAfterPresetChange = static_cast<uint8_t>(database::Config::CommonSetting::CustomCommonSettingStart),
            EnablePresetChangeWithProgramChangeIn,
            Count
        };

        /**
         * @brief Numeric status codes returned by SysEx configuration read and write handlers.
         */
        struct Status
        {
            /**
             * @brief Status values returned by SysEx configuration handlers.
             *
             * These codes mirror the base SysEx configuration protocol values and extend them
             * with OpenDeck-specific errors.
             */
            enum
            {
                Request,               // 0x00
                Ack,                   // 0x01
                ErrorStatus,           // 0x02
                ErrorConnection,       // 0x03
                ErrorWish,             // 0x04
                ErrorAmount,           // 0x05
                ErrorBlock,            // 0x06
                ErrorSection,          // 0x07
                ErrorPart,             // 0x08
                ErrorIndex,            // 0x09
                ErrorNewValue,         // 0x0A
                ErrorMessageLength,    // 0x0B
                ErrorWrite,            // 0x0C
                ErrorNotSupported,     // 0x0D
                ErrorRead,             // 0x0E
                SerialPeripheralAllocatedError = 80,
            };
        };
    };
}    // namespace opendeck::sys
