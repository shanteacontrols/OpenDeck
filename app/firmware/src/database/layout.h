/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/database/config.h"
#include "firmware/src/database/shared/deps.h"
#include "firmware/src/io/analog/shared/common.h"
#include "firmware/src/io/digital/switches/shared/common.h"
#include "firmware/src/io/digital/encoders/shared/common.h"
#include "firmware/src/io/i2c/peripherals/display/shared/common.h"
#include "firmware/src/io/outputs/shared/common.h"
#include "firmware/src/io/touchscreen/shared/common.h"
#include "firmware/src/protocol/midi/shared/common.h"
#include "firmware/src/protocol/mdns/shared/common.h"
#include "firmware/src/protocol/osc/shared/common.h"

#include <array>

namespace opendeck::database
{
    /**
     * @brief Application-specific LessDb layout for common and preset data.
     */
    class AppLayout
    {
        public:
        AppLayout() = default;

        /**
         * @brief Returns the common-data layout blocks.
         *
         * @return Span of LessDb blocks describing common data.
         */
        static std::span<const zlibs::utils::lessdb::Block> common_layout()
        {
            return COMMON_LAYOUT;
        }

        /**
         * @brief Returns the preset-data layout blocks.
         *
         * @return Span of LessDb blocks describing preset data.
         */
        static std::span<const zlibs::utils::lessdb::Block> preset_layout()
        {
            return PRESET_LAYOUT;
        }

        /**
         * @brief Returns the serialized size of the common-data layout.
         *
         * @return Common-data layout size in bytes.
         */
        static constexpr uint32_t common_layout_size()
        {
            return zlibs::utils::lessdb::LessDb::layout_size(COMMON_LAYOUT);
        }

        /**
         * @brief Returns the serialized size of the preset-data layout.
         *
         * @return Preset-data layout size in bytes.
         */
        static constexpr uint32_t preset_layout_size()
        {
            return zlibs::utils::lessdb::LessDb::layout_size(PRESET_LAYOUT);
        }

        /**
         * @brief Returns the storage size needed for the requested number of presets.
         *
         * @param preset_count Number of preset slots to include.
         *
         * @return Required database size in bytes.
         */
        static constexpr uint32_t database_size_for_presets(size_t preset_count)
        {
            return common_layout_size() + (preset_layout_size() * preset_count);
        }

        /**
         * @brief Returns how many preset slots fit in the provided database size.
         *
         * @param database_size Available database size in bytes.
         *
         * @return Supported preset count capped by the application preset limit.
         */
        static constexpr size_t supported_preset_count_for(uint32_t database_size)
        {
            const size_t preset_size = preset_layout_size();
            const size_t common_size = common_layout_size();

            if ((preset_size == 0) || (database_size <= common_size))
            {
                return 0;
            }

            const size_t available_presets = (database_size - common_size) / preset_size;

            return available_presets > CONFIG_PROJECT_DATABASE_MAX_SUPPORTED_PRESETS
                       ? CONFIG_PROJECT_DATABASE_MAX_SUPPORTED_PRESETS
                       : available_presets;
        }

        /**
         * @brief Returns the UID of the common-data layout.
         *
         * @return Common-data layout UID.
         */
        static constexpr uint16_t common_uid()
        {
            return static_cast<uint16_t>(zlibs::utils::lessdb::LessDb::layout_uid(COMMON_LAYOUT));
        }

        /**
         * @brief Returns the UID of the preset-data layout.
         *
         * @return Preset-data layout UID.
         */
        static constexpr uint16_t preset_uid()
        {
            return static_cast<uint16_t>(zlibs::utils::lessdb::LessDb::layout_uid(PRESET_LAYOUT));
        }

        private:
        using Section              = zlibs::utils::lessdb::Section;
        using Block                = zlibs::utils::lessdb::Block;
        using SectionParameterType = zlibs::utils::lessdb::SectionParameterType;
        using PreserveSetting      = zlibs::utils::lessdb::PreserveSetting;
        using AutoIncrementSetting = zlibs::utils::lessdb::AutoIncrementSetting;

        inline static constexpr auto COMMON_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            // CommonSettings
            Section(
                static_cast<uint8_t>(database::Config::CommonSetting::Count),
                SectionParameterType::Word,
                PreserveSetting::Disable,
                AutoIncrementSetting::Disable,
                0),
            // MdnsHostname
            Section(
                static_cast<uint8_t>(protocol::mdns::CUSTOM_HOSTNAME_DB_SIZE),
                SectionParameterType::Byte,
                PreserveSetting::Disable,
                AutoIncrementSetting::Disable,
                0),
        });

        inline static constexpr auto GLOBAL_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            // MidiSettings
            Section(
                static_cast<uint8_t>(protocol::midi::Setting::Count),
                SectionParameterType::Byte,
                PreserveSetting::Disable,
                AutoIncrementSetting::Disable,
                0),
            // OscSettings
            Section(
                static_cast<uint8_t>(protocol::osc::Setting::Count),
                SectionParameterType::Word,
                PreserveSetting::Disable,
                AutoIncrementSetting::Disable,
                0),
        });

        inline static constexpr auto SWITCH_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            // Type
            Section(io::switches::Collection::size(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // MessageType
            Section(io::switches::Collection::size(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // MidiId
            Section(io::switches::Collection::size(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // Value
            Section(io::switches::Collection::size(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    127),
            // Channel
            Section(io::switches::Collection::size(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    1),
        });

        inline static constexpr auto ENCODER_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            // Enable
            Section(io::encoders::Collection::size(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // Invert
            Section(io::encoders::Collection::size(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // Mode
            Section(io::encoders::Collection::size(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // MidiId1
            Section(io::encoders::Collection::size(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Enable,
                    0),
            // Channel
            Section(io::encoders::Collection::size(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    1),
            // Acceleration
            Section(io::encoders::Collection::size(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // RemoteSync
            Section(io::encoders::Collection::size(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // LowerLimit
            Section(io::encoders::Collection::size(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // UpperLimit
            Section(io::encoders::Collection::size(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    16383),
            // RepeatedValue
            Section(io::encoders::Collection::size(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    127),
            // MidiId2
            Section(io::encoders::Collection::size(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Enable,
                    0),
        });

        inline static constexpr auto ANALOG_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            // Enable
            Section(io::analog::Collection::size(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // Invert
            Section(io::analog::Collection::size(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // Type
            Section(io::analog::Collection::size(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // MidiId
            Section(io::analog::Collection::size(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Enable,
                    0),
            // LowerLimit
            Section(io::analog::Collection::size(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // UpperLimit
            Section(io::analog::Collection::size(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    16383),
            // Channel
            Section(io::analog::Collection::size(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    1),
            // LowerOffset
            Section(io::analog::Collection::size(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // UpperOffset
            Section(io::analog::Collection::size(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
        });

        inline static constexpr auto OUTPUT_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            // Global
            Section(static_cast<size_t>(io::outputs::Setting::Count),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // ActivationId
            Section(io::outputs::Collection::size(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // RgbEnable
            Section((io::outputs::Collection::size() / 3) + (io::touchscreen::Collection::size() / 3),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // ControlType
            Section(io::outputs::Collection::size(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // ActivationValue
            Section(io::outputs::Collection::size(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    127),
            // Channel
            Section(io::outputs::Collection::size(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    1),
        });

        inline static constexpr auto I2C_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            // Display
            Section(
                static_cast<uint8_t>(io::i2c::display::Setting::Count),
                SectionParameterType::Byte,
                PreserveSetting::Disable,
                AutoIncrementSetting::Disable,
                0),
        });

        inline static constexpr auto TOUCHSCREEN_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            // Setting
            Section(static_cast<uint8_t>(io::touchscreen::Setting::Count), SectionParameterType::Byte, PreserveSetting::Disable, AutoIncrementSetting::Disable, 0),
            // XPos
            Section(io::touchscreen::Collection::size(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // YPos
            Section(io::touchscreen::Collection::size(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // Width
            Section(io::touchscreen::Collection::size(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // Height
            Section(io::touchscreen::Collection::size(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // OnScreen
            Section(io::touchscreen::Collection::size(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // OffScreen
            Section(io::touchscreen::Collection::size(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // PageSwitchEnabled
            Section(io::touchscreen::Collection::size(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            // PageSwitchIndex
            Section(io::touchscreen::Collection::size(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
        });

        inline static constexpr auto COMMON_LAYOUT = zlibs::utils::lessdb::make_layout(std::array{
            Block(COMMON_SECTIONS),
        });

        inline static constexpr auto PRESET_LAYOUT = zlibs::utils::lessdb::make_layout(std::array{
            Block(GLOBAL_SECTIONS),
            Block(SWITCH_SECTIONS),
            Block(ENCODER_SECTIONS),
            Block(ANALOG_SECTIONS),
            Block(OUTPUT_SECTIONS),
            Block(I2C_SECTIONS),
            Block(TOUCHSCREEN_SECTIONS),
        });
    };
}    // namespace opendeck::database
