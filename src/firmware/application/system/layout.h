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
#include "application/database/database.h"
#include "application/io/buttons/buttons.h"
#include "application/io/encoders/encoders.h"
#include "application/io/analog/analog.h"
#include "application/io/leds/leds.h"
#include "application/io/i2c/peripherals/display/display.h"
#include "application/io/touchscreen/touchscreen.h"
#include "application/protocol/midi/midi.h"

#include "lib/sysexconf/sysexconf.h"

namespace sys
{
    class Layout
    {
        private:
        std::vector<lib::sysexconf::Section> _globalSections = {
            // midi settings section
            {
                static_cast<uint16_t>(protocol::midi::setting_t::AMOUNT),
                0,
                0,
            },

            // blank for compatibility
            {
                1,
                0,
                1,
            },

            // system settings section
            {
                static_cast<uint16_t>(Config::systemSetting_t::AMOUNT),
                0,
                0,
            },
        };

        std::vector<lib::sysexconf::Section> _buttonSections = {
            // type section
            {
                io::buttons::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::buttons::type_t::AMOUNT) - 1,
            },

            // message type section
            {
                io::buttons::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::buttons::messageType_t::AMOUNT) - 1,
            },

            // midi id section
            {
                io::buttons::Collection::SIZE(),
                0,
                127,
            },

            // value section
            {
                io::buttons::Collection::SIZE(),
                0,
                127,
            },

            // channel section
            {
                io::buttons::Collection::SIZE(),
                1,
                17,
            }
        };

        std::vector<lib::sysexconf::Section> _encoderSections = {
            // encoder enabled section
            {
                io::encoders::Collection::SIZE(),
                0,
                1,
            },

            // encoder inverted section
            {
                io::encoders::Collection::SIZE(),
                0,
                1,
            },

            // encoding mode section
            {
                io::encoders::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::encoders::type_t::AMOUNT) - 1,
            },

            // midi id 1 section
            {
                io::encoders::Collection::SIZE(),
                0,
                16383,
            },

            // channel section
            {
                io::encoders::Collection::SIZE(),
                1,
                17,
            },

            // pulses per step section
            {
                io::encoders::Collection::SIZE(),
                2,
                4,
            },

            // acceleration section
            {
                io::encoders::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::encoders::acceleration_t::AMOUNT) - 1,
            },

            // unused, reserved for compatibility
            {
                io::encoders::Collection::SIZE(),
                0,
                127,
            },

            // remote sync section
            {
                io::encoders::Collection::SIZE(),
                0,
                1,
            },

            // upper value limit section
            {
                io::encoders::Collection::SIZE(),
                0,
                16383,
            },

            // upper value limit section
            {
                io::encoders::Collection::SIZE(),
                0,
                16383,
            },

            // repeated value section
            {
                io::encoders::Collection::SIZE(),
                0,
                16383,
            },

            // midi id 2 section
            {
                io::encoders::Collection::SIZE(),
                0,
                16383,
            },
        };

        std::vector<lib::sysexconf::Section> _analogSections = {
            // analog enabled section
            {
                io::analog::Collection::SIZE(),
                0,
                1,
            },

            // analog inverted section
            {
                io::analog::Collection::SIZE(),
                0,
                1,
            },

            // analog type section
            {
                io::analog::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::analog::type_t::AMOUNT) - 1,
            },

            // midi id section
            {
                io::analog::Collection::SIZE(),
                0,
                16383,
            },

            // unused, reserved for compatibility
            {
                io::analog::Collection::SIZE(),
                0,
                127,
            },

            // lower value limit section
            {
                io::analog::Collection::SIZE(),
                0,
                16383,
            },

            // unused, reserved for compatibility
            {
                io::analog::Collection::SIZE(),
                0,
                127,
            },

            // upper value limit section
            {
                io::analog::Collection::SIZE(),
                0,
                16383,
            },

            // unused, reserved for compatibility
            {
                io::analog::Collection::SIZE(),
                0,
                127,
            },

            // channel section
            {
                io::analog::Collection::SIZE(),
                1,
                17,
            },

            // lower adc percentage offset section
            {
                io::analog::Collection::SIZE(),
                0,
                100,
            },

            // upper adc percentage offset section
            {
                io::analog::Collection::SIZE(),
                0,
                100,
            },
        };

        std::vector<lib::sysexconf::Section> _ledSections = {
            // led color test section
            {
                io::leds::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::leds::color_t::AMOUNT) - 1,
            },

            // led blink test section
            {
                io::leds::Collection::SIZE(),
                0,
                1,
            },

            // global parameters section
            {
                static_cast<uint16_t>(io::leds::setting_t::AMOUNT),
                0,
                0,
            },

            // activation id section
            {
                io::leds::Collection::SIZE(),
                0,
                127,
            },

            // rgb enabled section
            {
                io::leds::Collection::SIZE(),
                0,
                1,
            },

            // led control type section
            {
                io::leds::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::leds::controlType_t::AMOUNT) - 1,
            },

            // single led activation value section
            {
                io::leds::Collection::SIZE(),
                0,
                127,
            },

            // channel section
            {
                io::leds::Collection::SIZE(),
                1,
                17,
            }
        };

        std::vector<lib::sysexconf::Section> _i2cSections = {
            // display section
            {
                static_cast<uint16_t>(io::i2c::display::setting_t::AMOUNT),
                0,
                0,
            },
        };

        std::vector<lib::sysexconf::Section> _touchscreenSections = {
            // setting section
            {
                static_cast<uint16_t>(io::touchscreen::setting_t::AMOUNT),
                0,
                0,
            },

            // x position section
            {
                io::touchscreen::Collection::SIZE(),
                0,
                0,
            },

            // y position section
            {
                io::touchscreen::Collection::SIZE(),
                0,
                0,
            },

            // width section
            {
                io::touchscreen::Collection::SIZE(),
                0,
                1024,
            },

            // height section
            {
                io::touchscreen::Collection::SIZE(),
                0,
                600,
            },

            // on screen section
            {
                io::touchscreen::Collection::SIZE(),
                0,
                15,
            },

            // off screen section
            {
                io::touchscreen::Collection::SIZE(),
                0,
                15,
            },

            // page switch enabled section
            {
                io::touchscreen::Collection::SIZE(),
                0,
                1,
            },

            // page switch index section
            {
                io::touchscreen::Collection::SIZE(),
                0,
                15,
            },
        };

        std::vector<lib::sysexconf::Block> _layout = {
            // global block
            {
                _globalSections,
            },

            // buttons block
            {
                _buttonSections,
            },

            // encoder block
            {
                _encoderSections,
            },

            // analog block
            {
                _analogSections,
            },

            // led block
            {
                _ledSections,
            },

            // display block
            {
                _i2cSections,
            },

            // touchscreen block
            {
                _touchscreenSections,
            }
        };

        std::vector<lib::sysexconf::CustomRequest> _customRequests = {
            {
                .requestId     = SYSEX_CR_FIRMWARE_VERSION,
                .connOpenCheck = true,
            },

            {
                .requestId     = SYSEX_CR_HARDWARE_UID,
                .connOpenCheck = false,
            },

            {
                .requestId     = SYSEX_CR_FIRMWARE_VERSION_HARDWARE_UID,
                .connOpenCheck = true,
            },

            {
                .requestId     = SYSEX_CR_REBOOT_APP,
                .connOpenCheck = true,
            },

            {
                .requestId     = SYSEX_CR_REBOOT_BTLDR,
                .connOpenCheck = true,
            },

            {
                .requestId     = SYSEX_CR_FACTORY_RESET,
                .connOpenCheck = true,
            },

            {
                .requestId     = SYSEX_CR_MAX_COMPONENTS,
                .connOpenCheck = true,
            },

            {
                .requestId     = SYSEX_CR_SUPPORTED_PRESETS,
                .connOpenCheck = true,
            },

            {
                .requestId     = SYSEX_CR_BOOTLOADER_SUPPORT,
                .connOpenCheck = true,
            },

            {
                .requestId     = SYSEX_CR_FULL_BACKUP,
                .connOpenCheck = true,
            },

            {
                .requestId     = SYSEX_CR_RESTORE_START,
                .connOpenCheck = true,
            },

            {
                .requestId     = SYSEX_CR_RESTORE_END,
                .connOpenCheck = true,
            },
        };

        public:
        Layout() = default;

        std::vector<lib::sysexconf::Block>& layout()
        {
            return _layout;
        }

        std::vector<lib::sysexconf::CustomRequest>& customRequests()
        {
            return _customRequests;
        }
    };
}    // namespace sys