/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "application/database/Database.h"
#include "SysExConf/SysExConf.h"
#include "application/io/buttons/Buttons.h"
#include "application/io/encoders/Encoders.h"
#include "application/io/analog/Analog.h"
#include "application/io/leds/LEDs.h"
#include "application/io/i2c/peripherals/display/Display.h"
#include "application/io/touchscreen/Touchscreen.h"
#include "application/protocol/midi/MIDI.h"

namespace sys
{
    class Layout
    {
        private:
        std::vector<SysExConf::Section> _globalSections = {
            // midi settings section
            {
                static_cast<uint16_t>(protocol::MIDI::setting_t::AMOUNT),
                0,
                0,
            },

            // blank for compatibility
            {
                1,
                0,
                1,
            },

            // preset section
            {
                static_cast<uint16_t>(database::Config::presetSetting_t::AMOUNT),
                0,
                0,
            },
        };

        std::vector<SysExConf::Section> _buttonSections = {
            // type section
            {
                io::Buttons::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::Buttons::type_t::AMOUNT) - 1,
            },

            // message type section
            {
                io::Buttons::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::Buttons::messageType_t::AMOUNT) - 1,
            },

            // midi id section
            {
                io::Buttons::Collection::SIZE(),
                0,
                127,
            },

            // value section
            {
                io::Buttons::Collection::SIZE(),
                0,
                255,
            },

            // channel section
            {
                io::Buttons::Collection::SIZE(),
                1,
                16,
            }
        };

        std::vector<SysExConf::Section> _encoderSections = {
            // encoder enabled section
            {
                io::Encoders::Collection::SIZE(),
                0,
                1,
            },

            // encoder inverted section
            {
                io::Encoders::Collection::SIZE(),
                0,
                1,
            },

            // encoding mode section
            {
                io::Encoders::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::Encoders::type_t::AMOUNT) - 1,
            },

            // midi id section, lsb
            {
                io::Encoders::Collection::SIZE(),
                0,
                16383,
            },

            // channel section
            {
                io::Encoders::Collection::SIZE(),
                1,
                16,
            },

            // pulses per step section
            {
                io::Encoders::Collection::SIZE(),
                2,
                4,
            },

            // acceleration section
            {
                io::Encoders::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::Encoders::acceleration_t::AMOUNT) - 1,
            },

            // midi id section, msb
            {
                io::Encoders::Collection::SIZE(),
                0,
                127,
            },

            // remote sync section
            {
                io::Encoders::Collection::SIZE(),
                0,
                1,
            },
        };

        std::vector<SysExConf::Section> _analogSections = {
            // analog enabled section
            {
                io::Analog::Collection::SIZE(),
                0,
                1,
            },

            // analog inverted section
            {
                io::Analog::Collection::SIZE(),
                0,
                1,
            },

            // analog type section
            {
                io::Analog::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::Analog::type_t::AMOUNT) - 1,
            },

            // midi id section, lsb
            {
                io::Analog::Collection::SIZE(),
                0,
                16383,
            },

            // midi id section, msb
            {
                io::Analog::Collection::SIZE(),
                0,
                127,
            },

            // lower value limit, lsb
            {
                io::Analog::Collection::SIZE(),
                0,
                16383,
            },

            // lower value limit, msb
            {
                io::Analog::Collection::SIZE(),
                0,
                127,
            },

            // upper value limit, lsb
            {
                io::Analog::Collection::SIZE(),
                0,
                16383,
            },

            // upper value limit, msb
            {
                io::Analog::Collection::SIZE(),
                0,
                127,
            },

            // channel section
            {
                io::Analog::Collection::SIZE(),
                1,
                16,
            },

            // lower adc percentage offset
            {
                io::Analog::Collection::SIZE(),
                0,
                100,
            },

            // upper adc percentage offset
            {
                io::Analog::Collection::SIZE(),
                0,
                100,
            },
        };

        std::vector<SysExConf::Section> _ledSections = {
            // led color test section
            {
                io::LEDs::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::LEDs::color_t::AMOUNT) - 1,
            },

            // led blink test section
            {
                io::LEDs::Collection::SIZE(),
                0,
                1,
            },

            // global parameters section
            {
                static_cast<uint16_t>(io::LEDs::setting_t::AMOUNT),
                0,
                0,
            },

            // activation id section
            {
                io::LEDs::Collection::SIZE(),
                0,
                127,
            },

            // rgb enabled section
            {
                io::LEDs::Collection::SIZE(),
                0,
                1,
            },

            // led control type section
            {
                io::LEDs::Collection::SIZE(),
                0,
                static_cast<uint16_t>(io::LEDs::controlType_t::AMOUNT) - 1,
            },

            // single led activation value section
            {
                io::LEDs::Collection::SIZE(),
                1,
                127,
            },

            // channel section
            {
                io::LEDs::Collection::SIZE(),
                1,
                16,
            }
        };

        std::vector<SysExConf::Section> _i2cSections = {
            // display section
            {
                static_cast<uint16_t>(io::Display::setting_t::AMOUNT),
                0,
                0,
            },
        };

        std::vector<SysExConf::Section> _touchscreenSections = {
            // setting section
            {
                static_cast<uint16_t>(io::Touchscreen::setting_t::AMOUNT),
                0,
                0,
            },

            // x position section
            {
                io::Touchscreen::Collection::SIZE(),
                0,
                0,
            },

            // y position section
            {
                io::Touchscreen::Collection::SIZE(),
                0,
                0,
            },

            // width section
            {
                io::Touchscreen::Collection::SIZE(),
                0,
                1024,
            },

            // height section
            {
                io::Touchscreen::Collection::SIZE(),
                0,
                600,
            },

            // on screen section
            {
                io::Touchscreen::Collection::SIZE(),
                0,
                15,
            },

            // off screen section
            {
                io::Touchscreen::Collection::SIZE(),
                0,
                15,
            },

            // page switch enabled section
            {
                io::Touchscreen::Collection::SIZE(),
                0,
                1,
            },

            // page switch index section
            {
                io::Touchscreen::Collection::SIZE(),
                0,
                15,
            },
        };

        std::vector<SysExConf::Block> _layout = {
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

        std::vector<SysExConf::customRequest_t> _customRequests = {
            {
                .requestID     = SYSEX_CR_FIRMWARE_VERSION,
                .connOpenCheck = true,
            },

            {
                .requestID     = SYSEX_CR_HARDWARE_UID,
                .connOpenCheck = false,
            },

            {
                .requestID     = SYSEX_CR_FIRMWARE_VERSION_HARDWARE_UID,
                .connOpenCheck = true,
            },

            {
                .requestID     = SYSEX_CR_REBOOT_APP,
                .connOpenCheck = true,
            },

            {
                .requestID     = SYSEX_CR_REBOOT_BTLDR,
                .connOpenCheck = true,
            },

            {
                .requestID     = SYSEX_CR_FACTORY_RESET,
                .connOpenCheck = true,
            },

            {
                .requestID     = SYSEX_CR_MAX_COMPONENTS,
                .connOpenCheck = true,
            },

            {
                .requestID     = SYSEX_CR_SUPPORTED_PRESETS,
                .connOpenCheck = true,
            },

            {
                .requestID     = SYSEX_CR_BOOTLOADER_SUPPORT,
                .connOpenCheck = true,
            },

            {
                .requestID     = SYSEX_CR_FULL_BACKUP,
                .connOpenCheck = true,
            },

            {
                .requestID     = SYSEX_CR_RESTORE_START,
                .connOpenCheck = true,
            },

            {
                .requestID     = SYSEX_CR_RESTORE_END,
                .connOpenCheck = true,
            },
        };

        public:
        Layout() = default;

        std::vector<SysExConf::Block>& layout()
        {
            return _layout;
        }

        std::vector<SysExConf::customRequest_t>& customRequests()
        {
            return _customRequests;
        }
    };
}    // namespace sys