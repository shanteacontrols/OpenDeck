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

#include "sysex/src/SysExConf.h"
#include "io/leds/LEDs.h"
#include "io/encoders/Encoders.h"
#include "io/analog/Analog.h"
#include "io/display/Display.h"

namespace
{
    std::vector<SysExConf::Section> globalSections = {
        // midi feature section
        {
            static_cast<uint16_t>(System::midiFeature_t::AMOUNT),
            0,
            1,
        },

        // midi merge section
        {
            static_cast<uint16_t>(System::midiMerge_t::AMOUNT),
            0,
            0,
        },

        // preset section
        {
            static_cast<uint16_t>(System::presetSetting_t::AMOUNT),
            0,
            0,
        },

        // dmx section
        {
            static_cast<uint16_t>(System::dmxSetting_t::AMOUNT),
            0,
            0,
        },
    };

    std::vector<SysExConf::Section> buttonSections = {
        // type section
        {
            MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            static_cast<uint16_t>(IO::Buttons::type_t::AMOUNT) - 1,
        },

        // midi message type section
        {
            MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            static_cast<uint16_t>(IO::Buttons::messageType_t::AMOUNT) - 1,
        },

        // midi id section
        {
            MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            127,
        },

        // midi velocity section
        {
            MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            1,
            127,
        },

        // midi channel section
        {
            MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            1,
            16,
        }
    };

    std::vector<SysExConf::Section> encoderSections = {
        // encoder enabled section
        {
            MAX_NUMBER_OF_ENCODERS,
            0,
            1,
        },

        // encoder inverted section
        {
            MAX_NUMBER_OF_ENCODERS,
            0,
            1,
        },

        // encoding mode section
        {
            MAX_NUMBER_OF_ENCODERS,
            0,
            static_cast<uint16_t>(IO::Encoders::type_t::AMOUNT) - 1,
        },

        // midi id section, lsb
        {
            MAX_NUMBER_OF_ENCODERS,
            0,
            16383,
        },

        // midi channel section
        {
            MAX_NUMBER_OF_ENCODERS,
            1,
            16,
        },

        // pulses per step section
        {
            MAX_NUMBER_OF_ENCODERS,
            2,
            4,
        },

        // acceleration section
        {
            MAX_NUMBER_OF_ENCODERS,
            0,
            static_cast<uint16_t>(IO::Encoders::acceleration_t::AMOUNT) - 1,
        },

        // midi id section, msb
        {
            MAX_NUMBER_OF_ENCODERS,
            0,
            127,
        },

        // remote sync section
        {
            MAX_NUMBER_OF_ENCODERS,
            0,
            1,
        },
    };

    std::vector<SysExConf::Section> analogSections = {
        // analog enabled section
        {
            MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            1,
        },

        // analog inverted section
        {
            MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            1,
        },

        // analog type section
        {
            MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            static_cast<uint16_t>(IO::Analog::type_t::AMOUNT) - 1,
        },

        // midi id section, lsb
        {
            MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            16383,
        },

        // midi id section, msb
        {
            MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            127,
        },

        // lower cc limit, lsb
        {
            MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            16383,
        },

        // lower cc limit, msb
        {
            MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            127,
        },

        // upper cc limit, lsb
        {
            MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            16383,
        },

        // upper cc limit, msb
        {
            MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            127,
        },

        // midi channel section
        {
            MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            1,
            16,
        }
    };

    std::vector<SysExConf::Section> ledSections = {
        // led color test section
        {
            MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            static_cast<uint16_t>(IO::LEDs::color_t::AMOUNT) - 1,
        },

        // led blink test section
        {
            MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
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
            MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            127,
        },

        // rgb enabled section
        {
            MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            1,
        },

        // led control type section
        {
            MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            static_cast<uint16_t>(IO::LEDs::controlType_t::AMOUNT) - 1,
        },

        // single led velocity value section
        {
            MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            1,
            127,
        },

        // midi channel section
        {
            MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            1,
            16,
        }
    };

    std::vector<SysExConf::Section> displaySections = {
        // features section
        {
            static_cast<uint16_t>(IO::Display::feature_t::AMOUNT),
            0,
            1,
        },

        // settings section
        {
            static_cast<uint16_t>(IO::Display::setting_t::AMOUNT),
            0,
            0,
        }
    };

    std::vector<SysExConf::Section> touchscreenSections = {
        // setting section
        {
            static_cast<uint16_t>(IO::Touchscreen::setting_t::AMOUNT),
            0,
            0,
        },

        // x position section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            0,
        },

        // y position section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            0,
        },

        // width section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            1024,
        },

        // height section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            600,
        },

        // on screen section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            15,
        },

        // off screen section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            15,
        },

        // page switch enabled section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            1,
        },

        // page switch index section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            15,
        },

        // analog page section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            15,
        },

        // analog start x coordinate section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            0,
        },

        // analog end x coordinate section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            0,
        },

        // analog start y coordinate section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            0,
        },

        // analog end y coordinate section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            0,
        },

        // analog type section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            1,
        },

        // analog reset on release section
        {
            MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            0,
            1,
        },
    };

    std::vector<SysExConf::block_t> sysExLayout = {
        // global block
        {
            .section = globalSections,
        },

        // buttons block
        {
            .section = buttonSections,
        },

        // encoder block
        {
            .section = encoderSections,
        },

        // analog block
        {
            .section = analogSections,
        },

        // led block
        {
            .section = ledSections,
        },

        // display block
        {
            .section = displaySections,
        },

        // touchscreen block
        {
            .section = touchscreenSections,
        }
    };

    std::vector<SysExConf::customRequest_t> customRequests = {
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
}    // namespace