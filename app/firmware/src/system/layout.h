/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "custom_ids.h"
#include "io/digital/buttons/common.h"
#include "io/digital/encoders/common.h"
#include "io/analog/common.h"
#include "io/leds/common.h"
#include "io/i2c/peripherals/display/common.h"
#include "io/touchscreen/common.h"
#include "protocol/midi/common.h"
#include "system/config.h"

namespace opendeck::sys
{
    /**
     * @brief SysEx configuration layout used by the system backup and restore flow.
     */
    class Layout
    {
        private:
        static constexpr size_t GLOBAL_SECTION_COUNT      = 3;
        static constexpr size_t BUTTON_SECTION_COUNT      = 5;
        static constexpr size_t ENCODER_SECTION_COUNT     = 13;
        static constexpr size_t ANALOG_SECTION_COUNT      = 12;
        static constexpr size_t LED_SECTION_COUNT         = 8;
        static constexpr size_t I2C_SECTION_COUNT         = 1;
        static constexpr size_t TOUCHSCREEN_SECTION_COUNT = 9;
        static constexpr size_t BLOCK_COUNT               = 7;
        static constexpr size_t CUSTOM_REQUEST_COUNT      = 12;
        static constexpr size_t GLOBAL_BLOCK_INDEX        = 0;
        static constexpr size_t BUTTON_BLOCK_INDEX        = 1;
        static constexpr size_t ENCODER_BLOCK_INDEX       = 2;
        static constexpr size_t ANALOG_BLOCK_INDEX        = 3;
        static constexpr size_t LED_BLOCK_INDEX           = 4;
        static constexpr size_t I2C_BLOCK_INDEX           = 5;
        static constexpr size_t TOUCHSCREEN_BLOCK_INDEX   = 6;

        static constexpr auto GLOBAL_SECTIONS = zlibs::utils::sysex_conf::make_block(std::array<zlibs::utils::sysex_conf::Section, GLOBAL_SECTION_COUNT>{
            // midi settings section
            zlibs::utils::sysex_conf::Section(static_cast<uint16_t>(protocol::midi::Setting::Count),
                                              0,
                                              0),

            // blank for compatibility
            zlibs::utils::sysex_conf::Section(1,
                                              0,
                                              1),

            // system settings section
            zlibs::utils::sysex_conf::Section(static_cast<uint16_t>(Config::SystemSetting::Count),
                                              0,
                                              0),
        });

        static constexpr auto BUTTON_SECTIONS = zlibs::utils::sysex_conf::make_block(std::array<zlibs::utils::sysex_conf::Section, BUTTON_SECTION_COUNT>{
            // type section
            zlibs::utils::sysex_conf::Section(io::buttons::Collection::size(),
                                              0,
                                              static_cast<uint16_t>(io::buttons::Type::Count) - 1),

            // message type section
            zlibs::utils::sysex_conf::Section(io::buttons::Collection::size(),
                                              0,
                                              static_cast<uint16_t>(io::buttons::MessageType::Count) - 1),

            // midi id section
            zlibs::utils::sysex_conf::Section(io::buttons::Collection::size(),
                                              0,
                                              127),

            // value section
            zlibs::utils::sysex_conf::Section(io::buttons::Collection::size(),
                                              0,
                                              127),

            // channel section
            zlibs::utils::sysex_conf::Section(io::buttons::Collection::size(),
                                              1,
                                              17),
        });

        static constexpr auto ENCODER_SECTIONS = zlibs::utils::sysex_conf::make_block(std::array<zlibs::utils::sysex_conf::Section, ENCODER_SECTION_COUNT>{
            // encoder enabled section
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              0,
                                              1),

            // encoder inverted section
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              0,
                                              1),

            // encoding mode section
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              0,
                                              static_cast<uint16_t>(io::encoders::Type::Count) - 1),

            // midi id 1 section
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              0,
                                              16383),

            // channel section
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              1,
                                              17),

            // pulses per step section
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              2,
                                              4),

            // acceleration section
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              0,
                                              static_cast<uint16_t>(io::encoders::Acceleration::Count) - 1),

            // unused, reserved for compatibility
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              0,
                                              127),

            // remote sync section
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              0,
                                              1),

            // lower value limit section
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              0,
                                              16383),

            // upper value limit section
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              0,
                                              16383),

            // repeated value section
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              0,
                                              16383),

            // midi id 2 section
            zlibs::utils::sysex_conf::Section(io::encoders::Collection::size(),
                                              0,
                                              16383),
        });

        static constexpr auto ANALOG_SECTIONS = zlibs::utils::sysex_conf::make_block(std::array<zlibs::utils::sysex_conf::Section, ANALOG_SECTION_COUNT>{
            // analog enabled section
            zlibs::utils::sysex_conf::Section(io::analog::Collection::size(),
                                              0,
                                              1),

            // analog inverted section
            zlibs::utils::sysex_conf::Section(io::analog::Collection::size(),
                                              0,
                                              1),

            // analog type section
            zlibs::utils::sysex_conf::Section(io::analog::Collection::size(),
                                              0,
                                              static_cast<uint16_t>(io::analog::Type::Count) - 1),

            // midi id section
            zlibs::utils::sysex_conf::Section(io::analog::Collection::size(),
                                              0,
                                              16383),

            // unused, reserved for compatibility
            zlibs::utils::sysex_conf::Section(io::analog::Collection::size(),
                                              0,
                                              127),

            // lower value limit section
            zlibs::utils::sysex_conf::Section(io::analog::Collection::size(),
                                              0,
                                              16383),

            // unused, reserved for compatibility
            zlibs::utils::sysex_conf::Section(io::analog::Collection::size(),
                                              0,
                                              127),

            // upper value limit section
            zlibs::utils::sysex_conf::Section(io::analog::Collection::size(),
                                              0,
                                              16383),

            // unused, reserved for compatibility
            zlibs::utils::sysex_conf::Section(io::analog::Collection::size(),
                                              0,
                                              127),

            // channel section
            zlibs::utils::sysex_conf::Section(io::analog::Collection::size(),
                                              1,
                                              17),

            // lower adc percentage offset section
            zlibs::utils::sysex_conf::Section(io::analog::Collection::size(),
                                              0,
                                              100),

            // upper adc percentage offset section
            zlibs::utils::sysex_conf::Section(io::analog::Collection::size(),
                                              0,
                                              100),
        });

        static constexpr auto LED_SECTIONS = zlibs::utils::sysex_conf::make_block(std::array<zlibs::utils::sysex_conf::Section, LED_SECTION_COUNT>{
            // led color test section
            zlibs::utils::sysex_conf::Section(io::leds::Collection::size(),
                                              0,
                                              static_cast<uint16_t>(io::leds::Color::Count) - 1),

            // led blink test section
            zlibs::utils::sysex_conf::Section(io::leds::Collection::size(),
                                              0,
                                              1),

            // global parameters section
            zlibs::utils::sysex_conf::Section(static_cast<uint16_t>(io::leds::Setting::Count),
                                              0,
                                              0),

            // activation id section
            zlibs::utils::sysex_conf::Section(io::leds::Collection::size(),
                                              0,
                                              127),

            // rgb enabled section
            zlibs::utils::sysex_conf::Section(io::leds::Collection::size(),
                                              0,
                                              1),

            // led control type section
            zlibs::utils::sysex_conf::Section(io::leds::Collection::size(),
                                              0,
                                              static_cast<uint16_t>(io::leds::ControlType::Count) - 1),

            // single led activation value section
            zlibs::utils::sysex_conf::Section(io::leds::Collection::size(),
                                              0,
                                              127),

            // channel section
            zlibs::utils::sysex_conf::Section(io::leds::Collection::size(),
                                              1,
                                              17),
        });

        static constexpr auto I2C_SECTIONS = zlibs::utils::sysex_conf::make_block(std::array<zlibs::utils::sysex_conf::Section, I2C_SECTION_COUNT>{
            // display section
            zlibs::utils::sysex_conf::Section(static_cast<uint16_t>(io::i2c::display::Setting::Count),
                                              0,
                                              0),
        });

        static constexpr auto TOUCHSCREEN_SECTIONS = zlibs::utils::sysex_conf::make_block(std::array<zlibs::utils::sysex_conf::Section, TOUCHSCREEN_SECTION_COUNT>{
            // setting section
            zlibs::utils::sysex_conf::Section(static_cast<uint16_t>(io::touchscreen::Setting::Count),
                                              0,
                                              0),

            // x position section
            zlibs::utils::sysex_conf::Section(io::touchscreen::Collection::size(),
                                              0,
                                              0),

            // y position section
            zlibs::utils::sysex_conf::Section(io::touchscreen::Collection::size(),
                                              0,
                                              0),

            // width section
            zlibs::utils::sysex_conf::Section(io::touchscreen::Collection::size(),
                                              0,
                                              1024),

            // height section
            zlibs::utils::sysex_conf::Section(io::touchscreen::Collection::size(),
                                              0,
                                              600),

            // on screen section
            zlibs::utils::sysex_conf::Section(io::touchscreen::Collection::size(),
                                              0,
                                              15),

            // off screen section
            zlibs::utils::sysex_conf::Section(io::touchscreen::Collection::size(),
                                              0,
                                              15),

            // page switch enabled section
            zlibs::utils::sysex_conf::Section(io::touchscreen::Collection::size(),
                                              0,
                                              1),

            // page switch index section
            zlibs::utils::sysex_conf::Section(io::touchscreen::Collection::size(),
                                              0,
                                              15),
        });

        static constexpr auto LAYOUT = zlibs::utils::sysex_conf::make_layout(std::array<zlibs::utils::sysex_conf::Block, BLOCK_COUNT>{
            zlibs::utils::sysex_conf::Block(GLOBAL_SECTIONS),
            zlibs::utils::sysex_conf::Block(BUTTON_SECTIONS),
            zlibs::utils::sysex_conf::Block(ENCODER_SECTIONS),
            zlibs::utils::sysex_conf::Block(ANALOG_SECTIONS),
            zlibs::utils::sysex_conf::Block(LED_SECTIONS),
            zlibs::utils::sysex_conf::Block(I2C_SECTIONS),
            zlibs::utils::sysex_conf::Block(TOUCHSCREEN_SECTIONS),
        });

        static constexpr std::array<zlibs::utils::sysex_conf::CustomRequest, CUSTOM_REQUEST_COUNT> CUSTOM_REQUESTS = { {
            {
                .request_id      = SYSEX_CR_FIRMWARE_VERSION,
                .conn_open_check = true,
            },
            {
                .request_id      = SYSEX_CR_HARDWARE_UID,
                .conn_open_check = false,
            },
            {
                .request_id      = SYSEX_CR_FIRMWARE_VERSION_HARDWARE_UID,
                .conn_open_check = true,
            },
            {
                .request_id      = SYSEX_CR_REBOOT_APP,
                .conn_open_check = true,
            },
            {
                .request_id      = SYSEX_CR_REBOOT_BTLDR,
                .conn_open_check = true,
            },
            {
                .request_id      = SYSEX_CR_FACTORY_RESET,
                .conn_open_check = true,
            },
            {
                .request_id      = SYSEX_CR_MAX_COMPONENTS,
                .conn_open_check = true,
            },
            {
                .request_id      = SYSEX_CR_SUPPORTED_PRESETS,
                .conn_open_check = true,
            },
            {
                .request_id      = SYSEX_CR_BOOTLOADER_SUPPORT,
                .conn_open_check = true,
            },
            {
                .request_id      = SYSEX_CR_FULL_BACKUP,
                .conn_open_check = true,
            },
            {
                .request_id      = SYSEX_CR_RESTORE_START,
                .conn_open_check = true,
            },
            {
                .request_id      = SYSEX_CR_RESTORE_END,
                .conn_open_check = true,
            },
        } };

        public:
        Layout() = default;

        constexpr std::span<const zlibs::utils::sysex_conf::Block> layout() const
        {
            return LAYOUT;
        }

        constexpr std::span<const zlibs::utils::sysex_conf::CustomRequest> custom_requests() const
        {
            return CUSTOM_REQUESTS;
        }

        constexpr size_t blocks() const
        {
            return LAYOUT.size();
        }

        constexpr size_t sections(size_t block) const
        {
            switch (block)
            {
            case GLOBAL_BLOCK_INDEX:
                return GLOBAL_SECTIONS.size();
            case BUTTON_BLOCK_INDEX:
                return BUTTON_SECTIONS.size();
            case ENCODER_BLOCK_INDEX:
                return ENCODER_SECTIONS.size();
            case ANALOG_BLOCK_INDEX:
                return ANALOG_SECTIONS.size();
            case LED_BLOCK_INDEX:
                return LED_SECTIONS.size();
            case I2C_BLOCK_INDEX:
                return I2C_SECTIONS.size();
            case TOUCHSCREEN_BLOCK_INDEX:
                return TOUCHSCREEN_SECTIONS.size();
            default:
                return 0;
            }
        }
    };
}    // namespace opendeck::sys
