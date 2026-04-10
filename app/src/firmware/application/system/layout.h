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

#include "custom_ids.h"
#include "application/io/digital/buttons/common.h"
#include "application/io/digital/encoders/common.h"
#include "application/io/analog/common.h"
#include "application/io/leds/common.h"
#include "application/io/i2c/peripherals/display/common.h"
#include "application/io/touchscreen/common.h"
#include "application/protocol/midi/common.h"
#include "application/system/config.h"

namespace sys
{
    class Layout
    {
        private:
        static constexpr auto _globalSections = lib::sysexconf::make_block(std::array<lib::sysexconf::Section, 3>{
            // midi settings section
            lib::sysexconf::Section(static_cast<uint16_t>(protocol::midi::setting_t::AMOUNT),
                                    0,
                                    0),

            // blank for compatibility
            lib::sysexconf::Section(1,
                                    0,
                                    1),

            // system settings section
            lib::sysexconf::Section(static_cast<uint16_t>(Config::systemSetting_t::AMOUNT),
                                    0,
                                    0),
        });

        static constexpr auto _buttonSections = lib::sysexconf::make_block(std::array<lib::sysexconf::Section, 5>{
            // type section
            lib::sysexconf::Section(io::buttons::Collection::SIZE(),
                                    0,
                                    static_cast<uint16_t>(io::buttons::type_t::AMOUNT) - 1),

            // message type section
            lib::sysexconf::Section(io::buttons::Collection::SIZE(),
                                    0,
                                    static_cast<uint16_t>(io::buttons::messageType_t::AMOUNT) - 1),

            // midi id section
            lib::sysexconf::Section(io::buttons::Collection::SIZE(),
                                    0,
                                    127),

            // value section
            lib::sysexconf::Section(io::buttons::Collection::SIZE(),
                                    0,
                                    127),

            // channel section
            lib::sysexconf::Section(io::buttons::Collection::SIZE(),
                                    1,
                                    17),
        });

        static constexpr auto _encoderSections = lib::sysexconf::make_block(std::array<lib::sysexconf::Section, 13>{
            // encoder enabled section
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    0,
                                    1),

            // encoder inverted section
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    0,
                                    1),

            // encoding mode section
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    0,
                                    static_cast<uint16_t>(io::encoders::type_t::AMOUNT) - 1),

            // midi id 1 section
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    0,
                                    16383),

            // channel section
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    1,
                                    17),

            // pulses per step section
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    2,
                                    4),

            // acceleration section
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    0,
                                    static_cast<uint16_t>(io::encoders::acceleration_t::AMOUNT) - 1),

            // unused, reserved for compatibility
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    0,
                                    127),

            // remote sync section
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    0,
                                    1),

            // lower value limit section
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    0,
                                    16383),

            // upper value limit section
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    0,
                                    16383),

            // repeated value section
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    0,
                                    16383),

            // midi id 2 section
            lib::sysexconf::Section(io::encoders::Collection::SIZE(),
                                    0,
                                    16383),
        });

        static constexpr auto _analogSections = lib::sysexconf::make_block(std::array<lib::sysexconf::Section, 12>{
            // analog enabled section
            lib::sysexconf::Section(io::analog::Collection::SIZE(),
                                    0,
                                    1),

            // analog inverted section
            lib::sysexconf::Section(io::analog::Collection::SIZE(),
                                    0,
                                    1),

            // analog type section
            lib::sysexconf::Section(io::analog::Collection::SIZE(),
                                    0,
                                    static_cast<uint16_t>(io::analog::type_t::AMOUNT) - 1),

            // midi id section
            lib::sysexconf::Section(io::analog::Collection::SIZE(),
                                    0,
                                    16383),

            // unused, reserved for compatibility
            lib::sysexconf::Section(io::analog::Collection::SIZE(),
                                    0,
                                    127),

            // lower value limit section
            lib::sysexconf::Section(io::analog::Collection::SIZE(),
                                    0,
                                    16383),

            // unused, reserved for compatibility
            lib::sysexconf::Section(io::analog::Collection::SIZE(),
                                    0,
                                    127),

            // upper value limit section
            lib::sysexconf::Section(io::analog::Collection::SIZE(),
                                    0,
                                    16383),

            // unused, reserved for compatibility
            lib::sysexconf::Section(io::analog::Collection::SIZE(),
                                    0,
                                    127),

            // channel section
            lib::sysexconf::Section(io::analog::Collection::SIZE(),
                                    1,
                                    17),

            // lower adc percentage offset section
            lib::sysexconf::Section(io::analog::Collection::SIZE(),
                                    0,
                                    100),

            // upper adc percentage offset section
            lib::sysexconf::Section(io::analog::Collection::SIZE(),
                                    0,
                                    100),
        });

        static constexpr auto _ledSections = lib::sysexconf::make_block(std::array<lib::sysexconf::Section, 8>{
            // led color test section
            lib::sysexconf::Section(io::leds::Collection::SIZE(),
                                    0,
                                    static_cast<uint16_t>(io::leds::color_t::AMOUNT) - 1),

            // led blink test section
            lib::sysexconf::Section(io::leds::Collection::SIZE(),
                                    0,
                                    1),

            // global parameters section
            lib::sysexconf::Section(static_cast<uint16_t>(io::leds::setting_t::AMOUNT),
                                    0,
                                    0),

            // activation id section
            lib::sysexconf::Section(io::leds::Collection::SIZE(),
                                    0,
                                    127),

            // rgb enabled section
            lib::sysexconf::Section(io::leds::Collection::SIZE(),
                                    0,
                                    1),

            // led control type section
            lib::sysexconf::Section(io::leds::Collection::SIZE(),
                                    0,
                                    static_cast<uint16_t>(io::leds::controlType_t::AMOUNT) - 1),

            // single led activation value section
            lib::sysexconf::Section(io::leds::Collection::SIZE(),
                                    0,
                                    127),

            // channel section
            lib::sysexconf::Section(io::leds::Collection::SIZE(),
                                    1,
                                    17),
        });

        static constexpr auto _i2cSections = lib::sysexconf::make_block(std::array<lib::sysexconf::Section, 1>{
            // display section
            lib::sysexconf::Section(static_cast<uint16_t>(io::i2c::display::setting_t::AMOUNT),
                                    0,
                                    0),
        });

        static constexpr auto _touchscreenSections = lib::sysexconf::make_block(std::array<lib::sysexconf::Section, 9>{
            // setting section
            lib::sysexconf::Section(static_cast<uint16_t>(io::touchscreen::setting_t::AMOUNT),
                                    0,
                                    0),

            // x position section
            lib::sysexconf::Section(io::touchscreen::Collection::SIZE(),
                                    0,
                                    0),

            // y position section
            lib::sysexconf::Section(io::touchscreen::Collection::SIZE(),
                                    0,
                                    0),

            // width section
            lib::sysexconf::Section(io::touchscreen::Collection::SIZE(),
                                    0,
                                    1024),

            // height section
            lib::sysexconf::Section(io::touchscreen::Collection::SIZE(),
                                    0,
                                    600),

            // on screen section
            lib::sysexconf::Section(io::touchscreen::Collection::SIZE(),
                                    0,
                                    15),

            // off screen section
            lib::sysexconf::Section(io::touchscreen::Collection::SIZE(),
                                    0,
                                    15),

            // page switch enabled section
            lib::sysexconf::Section(io::touchscreen::Collection::SIZE(),
                                    0,
                                    1),

            // page switch index section
            lib::sysexconf::Section(io::touchscreen::Collection::SIZE(),
                                    0,
                                    15),
        });

        static constexpr auto _layout = lib::sysexconf::make_layout(std::array<lib::sysexconf::Block, 7>{
            lib::sysexconf::Block(_globalSections),
            lib::sysexconf::Block(_buttonSections),
            lib::sysexconf::Block(_encoderSections),
            lib::sysexconf::Block(_analogSections),
            lib::sysexconf::Block(_ledSections),
            lib::sysexconf::Block(_i2cSections),
            lib::sysexconf::Block(_touchscreenSections),
        });

        static constexpr std::array<lib::sysexconf::CustomRequest, 12> _customRequests = { {
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

        constexpr std::span<const lib::sysexconf::Block> layout() const
        {
            return _layout;
        }

        constexpr std::span<const lib::sysexconf::CustomRequest> customRequests() const
        {
            return _customRequests;
        }

        constexpr size_t blocks() const
        {
            return _layout.size();
        }

        constexpr size_t sections(size_t block) const
        {
            switch (block)
            {
            case 0:
                return _globalSections.size();
            case 1:
                return _buttonSections.size();
            case 2:
                return _encoderSections.size();
            case 3:
                return _analogSections.size();
            case 4:
                return _ledSections.size();
            case 5:
                return _i2cSections.size();
            case 6:
                return _touchscreenSections.size();
            default:
                return 0;
            }
        }
    };
}    // namespace sys
