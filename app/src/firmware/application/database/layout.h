/*

Copyright Igor Petrovic

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

#include "deps.h"
#include "application/io/analog/common.h"
#include "application/io/digital/buttons/common.h"
#include "application/io/digital/encoders/common.h"
#include "application/io/i2c/peripherals/display/common.h"
#include "application/io/leds/common.h"
#include "application/io/touchscreen/common.h"
#include "application/protocol/midi/common.h"

#include <array>

namespace database
{
    class AppLayout : public database::Layout
    {
        public:
        AppLayout() = default;

        std::span<const zlibs::utils::lessdb::Block> commonLayout() const override
        {
            return COMMON_LAYOUT;
        }

        std::span<const zlibs::utils::lessdb::Block> presetLayout() const override
        {
            return PRESET_LAYOUT;
        }

        constexpr uint32_t commonLayoutSize() const override
        {
            return zlibs::utils::lessdb::LessDb::layout_size(COMMON_LAYOUT);
        }

        constexpr uint32_t presetLayoutSize() const override
        {
            return zlibs::utils::lessdb::LessDb::layout_size(PRESET_LAYOUT);
        }

        constexpr uint16_t commonUid() const override
        {
            return static_cast<uint16_t>(zlibs::utils::lessdb::LessDb::layout_uid(COMMON_LAYOUT));
        }

        constexpr uint16_t presetUid() const override
        {
            return static_cast<uint16_t>(zlibs::utils::lessdb::LessDb::layout_uid(PRESET_LAYOUT));
        }

        private:
        using Section = zlibs::utils::lessdb::Section;
        using Block   = zlibs::utils::lessdb::Block;

        using SectionParameterType = zlibs::utils::lessdb::SectionParameterType;
        using PreserveSetting      = zlibs::utils::lessdb::PreserveSetting;
        using AutoIncrementSetting = zlibs::utils::lessdb::AutoIncrementSetting;

        inline static constexpr auto COMMON_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            Section(
                static_cast<uint8_t>(database::Config::commonSetting_t::AMOUNT),
                SectionParameterType::Word,
                PreserveSetting::Disable,
                AutoIncrementSetting::Disable,
                0),
        });

        inline static constexpr auto GLOBAL_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            Section(
                static_cast<uint8_t>(protocol::midi::setting_t::AMOUNT),
                SectionParameterType::Byte,
                PreserveSetting::Disable,
                AutoIncrementSetting::Disable,
                0),
        });

        inline static constexpr auto BUTTON_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            Section(io::buttons::Collection::SIZE(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::buttons::Collection::SIZE(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::buttons::Collection::SIZE(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::buttons::Collection::SIZE(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    127),
            Section(io::buttons::Collection::SIZE(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    1),
        });

        inline static constexpr auto ENCODER_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            Section(io::encoders::Collection::SIZE(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::encoders::Collection::SIZE(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::encoders::Collection::SIZE(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::encoders::Collection::SIZE(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Enable,
                    0),
            Section(io::encoders::Collection::SIZE(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    1),
            Section(io::encoders::Collection::SIZE(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    4),
            Section(io::encoders::Collection::SIZE(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::encoders::Collection::SIZE(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::encoders::Collection::SIZE(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::encoders::Collection::SIZE(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    16383),
            Section(io::encoders::Collection::SIZE(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    127),
            Section(io::encoders::Collection::SIZE(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Enable,
                    0),
        });

        inline static constexpr auto ANALOG_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            Section(io::analog::Collection::SIZE(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::analog::Collection::SIZE(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::analog::Collection::SIZE(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::analog::Collection::SIZE(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Enable,
                    0),
            Section(io::analog::Collection::SIZE(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::analog::Collection::SIZE(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    16383),
            Section(io::analog::Collection::SIZE(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    1),
            Section(io::analog::Collection::SIZE(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::analog::Collection::SIZE(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
        });

        inline static constexpr auto LED_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            Section(static_cast<size_t>(io::leds::setting_t::AMOUNT),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::leds::Collection::SIZE(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section((io::leds::Collection::SIZE() / 3) + (io::touchscreen::Collection::SIZE() / 3),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::leds::Collection::SIZE(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::leds::Collection::SIZE(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    127),
            Section(io::leds::Collection::SIZE(),
                    SectionParameterType::Byte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    1),
        });

        inline static constexpr auto I2C_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            Section(
                static_cast<uint8_t>(io::i2c::display::setting_t::AMOUNT),
                SectionParameterType::Byte,
                PreserveSetting::Disable,
                AutoIncrementSetting::Disable,
                0),
        });

        inline static constexpr auto TOUCHSCREEN_SECTIONS = zlibs::utils::lessdb::make_block(std::array{
            Section(static_cast<uint8_t>(io::touchscreen::setting_t::AMOUNT), SectionParameterType::Byte, PreserveSetting::Disable, AutoIncrementSetting::Disable, 0),
            Section(io::touchscreen::Collection::SIZE(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::touchscreen::Collection::SIZE(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::touchscreen::Collection::SIZE(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::touchscreen::Collection::SIZE(),
                    SectionParameterType::Word,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::touchscreen::Collection::SIZE(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::touchscreen::Collection::SIZE(),
                    SectionParameterType::HalfByte,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::touchscreen::Collection::SIZE(),
                    SectionParameterType::Bit,
                    PreserveSetting::Disable,
                    AutoIncrementSetting::Disable,
                    0),
            Section(io::touchscreen::Collection::SIZE(),
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
            Block(BUTTON_SECTIONS),
            Block(ENCODER_SECTIONS),
            Block(ANALOG_SECTIONS),
            Block(LED_SECTIONS),
            Block(I2C_SECTIONS),
            Block(TOUCHSCREEN_SECTIONS),
        });
    };
}    // namespace database
