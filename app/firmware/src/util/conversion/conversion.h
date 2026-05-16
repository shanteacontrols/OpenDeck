/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/database/instance/impl/database.h"
#include "firmware/src/system/config.h"

#include "zlibs/utils/sysex_conf/sysex_conf.h"

namespace opendeck::util
{
    /**
     * @brief Converts runtime system-config sections into database-config sections.
     */
    class Conversion
    {
        public:
        /**
         * @brief Prevents instantiation of this static conversion helper.
         */
        Conversion() = delete;

        /**
         * @brief Helper alias that splits one 14-bit value into transport bytes.
         */
        using Split14Bit = zlibs::utils::sysex_conf::Split14Bit;

        /**
         * @brief Helper alias that merges transport bytes into one 14-bit value.
         */
        using Merge14Bit = zlibs::utils::sysex_conf::Merge14Bit;

        /**
         * @brief Converts a global system-config section into its database equivalent.
         *
         * @param section System-config global section.
         *
         * @return Matching database-config section.
         */
        static constexpr database::Config::Section::Global sys_2_db_section(sys::Config::Section::Global section)
        {
            return SYSEX_2_DB_GLOBAL[static_cast<uint8_t>(section)];
        }

        /**
         * @brief Converts a switch system-config section into its database equivalent.
         *
         * @param section System-config switch section.
         *
         * @return Matching database-config section.
         */
        static constexpr database::Config::Section::Switch sys_2_db_section(sys::Config::Section::Switch section)
        {
            return SYSEX_2_DB_SWITCH[static_cast<uint8_t>(section)];
        }

        /**
         * @brief Converts an encoder system-config section into its database equivalent.
         *
         * @param section System-config encoder section.
         *
         * @return Matching database-config section.
         */
        static constexpr database::Config::Section::Encoder sys_2_db_section(sys::Config::Section::Encoder section)
        {
            return SYSEX_2_DB_ENCODER[static_cast<uint8_t>(section)];
        }

        /**
         * @brief Converts an analog system-config section into its database equivalent.
         *
         * @param section System-config analog section.
         *
         * @return Matching database-config section.
         */
        static constexpr database::Config::Section::Analog sys_2_db_section(sys::Config::Section::Analog section)
        {
            return SYSEX_2_DB_ANALOG[static_cast<uint8_t>(section)];
        }

        /**
         * @brief Converts an OUTPUT system-config section into its database equivalent.
         *
         * @param section System-config OUTPUT section.
         *
         * @return Matching database-config section.
         */
        static constexpr database::Config::Section::Outputs sys_2_db_section(sys::Config::Section::Outputs section)
        {
            return SYSEX_2_DB_OUTPUTS[static_cast<uint8_t>(section)];
        }

        /**
         * @brief Converts an I2C system-config section into its database equivalent.
         *
         * @param section System-config I2C section.
         *
         * @return Matching database-config section.
         */
        static constexpr database::Config::Section::I2c sys_2_db_section(sys::Config::Section::I2c section)
        {
            return SYSEX_2_DB_I2C[static_cast<uint8_t>(section)];
        }

        /**
         * @brief Converts a touchscreen system-config section into its database equivalent.
         *
         * @param section System-config touchscreen section.
         *
         * @return Matching database-config section.
         */
        static constexpr database::Config::Section::Touchscreen sys_2_db_section(sys::Config::Section::Touchscreen section)
        {
            return SYSEX_2_DB_TOUCHSCREEN[static_cast<uint8_t>(section)];
        }

        private:
        static constexpr database::Config::Section::Global SYSEX_2_DB_GLOBAL[static_cast<uint8_t>(sys::Config::Section::Global::Count)] = {
            database::Config::Section::Global::MidiSettings,
            database::Config::Section::Global::Count,    // blank/reserved
            database::Config::Section::Global::Count,    // unused
            database::Config::Section::Global::OscSettings,
            database::Config::Section::Global::Count,
        };

        static constexpr database::Config::Section::Switch SYSEX_2_DB_SWITCH[static_cast<uint8_t>(sys::Config::Section::Switch::Count)] = {
            database::Config::Section::Switch::Type,
            database::Config::Section::Switch::MessageType,
            database::Config::Section::Switch::MidiId,
            database::Config::Section::Switch::Value,
            database::Config::Section::Switch::Channel
        };

        static constexpr database::Config::Section::Encoder SYSEX_2_DB_ENCODER[static_cast<uint8_t>(sys::Config::Section::Encoder::Count)] = {
            database::Config::Section::Encoder::Enable,
            database::Config::Section::Encoder::Invert,
            database::Config::Section::Encoder::Mode,
            database::Config::Section::Encoder::MidiId1,
            database::Config::Section::Encoder::Channel,
            database::Config::Section::Encoder::Count,
            database::Config::Section::Encoder::Acceleration,
            database::Config::Section::Encoder::MidiId1,
            database::Config::Section::Encoder::RemoteSync,
            database::Config::Section::Encoder::LowerLimit,
            database::Config::Section::Encoder::UpperLimit,
            database::Config::Section::Encoder::RepeatedValue,
            database::Config::Section::Encoder::MidiId2,
        };

        static constexpr database::Config::Section::Analog SYSEX_2_DB_ANALOG[static_cast<uint8_t>(sys::Config::Section::Analog::Count)] = {
            database::Config::Section::Analog::Enable,
            database::Config::Section::Analog::Invert,
            database::Config::Section::Analog::Type,
            database::Config::Section::Analog::MidiId,
            database::Config::Section::Analog::MidiId,
            database::Config::Section::Analog::LowerLimit,
            database::Config::Section::Analog::LowerLimit,
            database::Config::Section::Analog::UpperLimit,
            database::Config::Section::Analog::UpperLimit,
            database::Config::Section::Analog::Channel,
            database::Config::Section::Analog::LowerOffset,
            database::Config::Section::Analog::UpperOffset,
        };

        static constexpr database::Config::Section::Outputs SYSEX_2_DB_OUTPUTS[static_cast<uint8_t>(sys::Config::Section::Outputs::Count)] = {
            database::Config::Section::Outputs::Count,
            database::Config::Section::Outputs::Count,
            database::Config::Section::Outputs::Global,
            database::Config::Section::Outputs::ActivationId,
            database::Config::Section::Outputs::RgbEnable,
            database::Config::Section::Outputs::ControlType,
            database::Config::Section::Outputs::ActivationValue,
            database::Config::Section::Outputs::Channel,
        };

        static constexpr database::Config::Section::I2c SYSEX_2_DB_I2C[static_cast<uint8_t>(sys::Config::Section::I2c::Count)] = {
            database::Config::Section::I2c::Display,
        };

        static constexpr database::Config::Section::Touchscreen SYSEX_2_DB_TOUCHSCREEN[static_cast<uint8_t>(sys::Config::Section::Touchscreen::Count)] = {
            database::Config::Section::Touchscreen::Setting,
            database::Config::Section::Touchscreen::XPos,
            database::Config::Section::Touchscreen::YPos,
            database::Config::Section::Touchscreen::Width,
            database::Config::Section::Touchscreen::Height,
            database::Config::Section::Touchscreen::OnScreen,
            database::Config::Section::Touchscreen::OffScreen,
            database::Config::Section::Touchscreen::PageSwitchEnabled,
            database::Config::Section::Touchscreen::PageSwitchIndex,
        };
    };
}    // namespace opendeck::util
