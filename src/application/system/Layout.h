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

#include "database/Database.h"
#include "sysex/src/SysExConf.h"
#include "io/buttons/Buttons.h"
#include "io/encoders/Encoders.h"
#include "io/analog/Analog.h"
#include "io/leds/LEDs.h"
#include "io/i2c/peripherals/display/Display.h"
#include "io/touchscreen/Touchscreen.h"
#include "protocol/dmx/DMX.h"
#include "protocol/midi/MIDI.h"

namespace System
{
    class Layout
    {
        private:
        std::vector<SysExConf::Section> _globalSections = {
            // midi settings section
            {
                static_cast<uint16_t>(Protocol::MIDI::setting_t::AMOUNT),
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
                static_cast<uint16_t>(Database::Config::presetSetting_t::AMOUNT),
                0,
                0,
            },

            // dmx settings section
            {
                static_cast<uint16_t>(Protocol::DMX::setting_t::AMOUNT),
                0,
                0,
            },

            // dmx channel section
            {
                513,
                0,
                255,
            },
        };

        std::vector<SysExConf::Section> _buttonSections = {
            // type section
            {
                IO::Buttons::Collection::size(),
                0,
                static_cast<uint16_t>(IO::Buttons::type_t::AMOUNT) - 1,
            },

            // message type section
            {
                IO::Buttons::Collection::size(),
                0,
                static_cast<uint16_t>(IO::Buttons::messageType_t::AMOUNT) - 1,
            },

            // midi id section
            {
                IO::Buttons::Collection::size(),
                0,
                127,
            },

            // value section
            {
                IO::Buttons::Collection::size(),
                0,
                255,
            },

            // channel section
            {
                IO::Buttons::Collection::size(),
                1,
                512,
            }
        };

        std::vector<SysExConf::Section> _encoderSections = {
            // encoder enabled section
            {
                IO::Encoders::Collection::size(),
                0,
                1,
            },

            // encoder inverted section
            {
                IO::Encoders::Collection::size(),
                0,
                1,
            },

            // encoding mode section
            {
                IO::Encoders::Collection::size(),
                0,
                static_cast<uint16_t>(IO::Encoders::type_t::AMOUNT) - 1,
            },

            // midi id section, lsb
            {
                IO::Encoders::Collection::size(),
                0,
                16383,
            },

            // channel section
            {
                IO::Encoders::Collection::size(),
                1,
                512,
            },

            // pulses per step section
            {
                IO::Encoders::Collection::size(),
                2,
                4,
            },

            // acceleration section
            {
                IO::Encoders::Collection::size(),
                0,
                static_cast<uint16_t>(IO::Encoders::acceleration_t::AMOUNT) - 1,
            },

            // midi id section, msb
            {
                IO::Encoders::Collection::size(),
                0,
                127,
            },

            // remote sync section
            {
                IO::Encoders::Collection::size(),
                0,
                1,
            },
        };

        std::vector<SysExConf::Section> _analogSections = {
            // analog enabled section
            {
                IO::Analog::Collection::size(),
                0,
                1,
            },

            // analog inverted section
            {
                IO::Analog::Collection::size(),
                0,
                1,
            },

            // analog type section
            {
                IO::Analog::Collection::size(),
                0,
                static_cast<uint16_t>(IO::Analog::type_t::AMOUNT) - 1,
            },

            // midi id section, lsb
            {
                IO::Analog::Collection::size(),
                0,
                16383,
            },

            // midi id section, msb
            {
                IO::Analog::Collection::size(),
                0,
                127,
            },

            // lower value limit, lsb
            {
                IO::Analog::Collection::size(),
                0,
                16383,
            },

            // lower value limit, msb
            {
                IO::Analog::Collection::size(),
                0,
                127,
            },

            // upper value limit, lsb
            {
                IO::Analog::Collection::size(),
                0,
                16383,
            },

            // upper value limit, msb
            {
                IO::Analog::Collection::size(),
                0,
                127,
            },

            // channel section
            {
                IO::Analog::Collection::size(),
                1,
                512,
            },

            // lower adc percentage offset
            {
                IO::Analog::Collection::size(),
                0,
                100,
            },

            // upper adc percentage offset
            {
                IO::Analog::Collection::size(),
                0,
                100,
            },
        };

        std::vector<SysExConf::Section> _ledSections = {
            // led color test section
            {
                IO::LEDs::Collection::size(),
                0,
                static_cast<uint16_t>(IO::LEDs::color_t::AMOUNT) - 1,
            },

            // led blink test section
            {
                IO::LEDs::Collection::size(),
                0,
                1,
            },

            // global parameters section
            {
                static_cast<uint16_t>(IO::LEDs::setting_t::AMOUNT),
                0,
                0,
            },

            // activation note section
            {
                IO::LEDs::Collection::size(),
                0,
                127,
            },

            // rgb enabled section
            {
                IO::LEDs::Collection::size(),
                0,
                1,
            },

            // led control type section
            {
                IO::LEDs::Collection::size(),
                0,
                static_cast<uint16_t>(IO::LEDs::controlType_t::AMOUNT) - 1,
            },

            // single led velocity value section
            {
                IO::LEDs::Collection::size(),
                1,
                127,
            },

            // channel section
            {
                IO::LEDs::Collection::size(),
                1,
                512,
            }
        };

        std::vector<SysExConf::Section> _i2cSections = {
            // display section
            {
                static_cast<uint16_t>(IO::Display::setting_t::AMOUNT),
                0,
                0,
            },
        };

        std::vector<SysExConf::Section> _touchscreenSections = {
            // setting section
            {
                static_cast<uint16_t>(IO::Touchscreen::setting_t::AMOUNT),
                0,
                0,
            },

            // x position section
            {
                IO::Touchscreen::Collection::size(),
                0,
                0,
            },

            // y position section
            {
                IO::Touchscreen::Collection::size(),
                0,
                0,
            },

            // width section
            {
                IO::Touchscreen::Collection::size(),
                0,
                1024,
            },

            // height section
            {
                IO::Touchscreen::Collection::size(),
                0,
                600,
            },

            // on screen section
            {
                IO::Touchscreen::Collection::size(),
                0,
                15,
            },

            // off screen section
            {
                IO::Touchscreen::Collection::size(),
                0,
                15,
            },

            // page switch enabled section
            {
                IO::Touchscreen::Collection::size(),
                0,
                1,
            },

            // page switch index section
            {
                IO::Touchscreen::Collection::size(),
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
}    // namespace System