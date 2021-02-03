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
#include "io/encoders/Constants.h"
#include "io/analog/Analog.h"
#include "io/display/Display.h"

namespace
{
    SysExConf::section_t globalSections[static_cast<uint8_t>(System::Section::global_t::AMOUNT)] = {
        //midi feature section
        {
            .numberOfParameters = static_cast<uint8_t>(System::midiFeature_t::AMOUNT),
            .newValueMin        = 0,
            .newValueMax        = 1,
        },

        //midi merge section
        {
            .numberOfParameters = static_cast<uint8_t>(System::midiMerge_t::AMOUNT),
            .newValueMin        = 0,
            .newValueMax        = 0,
        },

        //preset section
        {
            .numberOfParameters = static_cast<uint8_t>(System::presetSetting_t::AMOUNT),
            .newValueMin        = 0,
            .newValueMax        = 0,
        },
    };

    SysExConf::section_t buttonSections[static_cast<uint8_t>(System::Section::button_t::AMOUNT)] = {
        //type section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = static_cast<SysExConf::sysExParameter_t>(IO::Buttons::type_t::AMOUNT) - 1,
        },

        //midi message type section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = static_cast<SysExConf::sysExParameter_t>(IO::Buttons::messageType_t::AMOUNT) - 1,
        },

        //midi id section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 127,
        },

        //midi velocity section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 1,
            .newValueMax        = 127,
        },

        //midi channel section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 1,
            .newValueMax        = 16,
        }
    };

    SysExConf::section_t encoderSections[static_cast<uint8_t>(System::Section::encoder_t::AMOUNT)] = {
        //encoder enabled section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin        = 0,
            .newValueMax        = 1,
        },

        //encoder inverted section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin        = 0,
            .newValueMax        = 1,
        },

        //encoding mode section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin        = 0,
            .newValueMax        = static_cast<SysExConf::sysExParameter_t>(IO::Encoders::type_t::AMOUNT) - 1,
        },

        //midi id section, lsb
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin        = 0,
            .newValueMax        = 16383,
        },

        //midi channel section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin        = 1,
            .newValueMax        = 16,
        },

        //pulses per step section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin        = 2,
            .newValueMax        = 4,
        },

        //acceleration section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin        = 0,
            .newValueMax        = static_cast<uint8_t>(IO::Encoders::acceleration_t::AMOUNT) - 1,
        },

        //midi id section, msb
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin        = 0,
            .newValueMax        = 127,
        },

        //remote sync section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin        = 0,
            .newValueMax        = 1,
        },
    };

    SysExConf::section_t analogSections[static_cast<uint8_t>(System::Section::analog_t::AMOUNT)] = {
        //analog enabled section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 1,
        },

        //analog inverted section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 1,
        },

        //analog type section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = static_cast<SysExConf::sysExParameter_t>(IO::Analog::type_t::AMOUNT) - 1,
        },

        //midi id section, lsb
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 16383,
        },

        //midi id section, msb
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 127,
        },

        //lower cc limit, lsb
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 16383,
        },

        //lower cc limit, msb
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 127,
        },

        //upper cc limit, lsb
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 16383,
        },

        //upper cc limit, msb
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 127,
        },

        //midi channel section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 1,
            .newValueMax        = 16,
        }
    };

    SysExConf::section_t ledSections[static_cast<uint8_t>(System::Section::leds_t::AMOUNT)] = {
        //led color test section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = static_cast<SysExConf::sysExParameter_t>(IO::LEDs::color_t::AMOUNT) - 1,
        },

        //led blink test section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 1,
        },

        //global parameters section
        {
            .numberOfParameters = static_cast<SysExConf::sysExParameter_t>(IO::LEDs::setting_t::AMOUNT),
            .newValueMin        = 0,
            .newValueMax        = 0,
        },

        //activation note section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 127,
        },

        //rgb enabled section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 1,
        },

        //led control type section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = static_cast<SysExConf::sysExParameter_t>(IO::LEDs::controlType_t::AMOUNT) - 1,
        },

        //single led velocity value section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 1,
            .newValueMax        = 127,
        },

        //midi channel section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 1,
            .newValueMax        = 16,
        }
    };

    SysExConf::section_t displaySections[static_cast<uint8_t>(System::Section::display_t::AMOUNT)] = {
        //features section
        {
            .numberOfParameters = static_cast<uint8_t>(IO::Display::feature_t::AMOUNT),
            .newValueMin        = 0,
            .newValueMax        = 1,
        },

        //settings section
        {
            .numberOfParameters = static_cast<uint8_t>(IO::Display::setting_t::AMOUNT),
            .newValueMin        = 0,
            .newValueMax        = 0,
        }
    };

    SysExConf::section_t touchscreenSections[static_cast<uint8_t>(System::Section::touchscreen_t::AMOUNT)] = {
        //setting section
        {
            .numberOfParameters = static_cast<uint8_t>(IO::Touchscreen::setting_t::AMOUNT),
            .newValueMin        = 0,
            .newValueMax        = 0,
        },

        //x position section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 0,
        },

        //y position section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 0,
        },

        //width section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 1024,
        },

        //height section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 600,
        },

        //on screen section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 15,
        },

        //off screen section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 15,
        },

        //page switch enabled section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 1,
        },

        //page switch index section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 15,
        },

        //analog page section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 15,
        },

        //analog start x coordinate section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 0,
        },

        //analog end x coordinate section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 0,
        },

        //analog start y coordinate section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 0,
        },

        //analog end y coordinate section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 0,
        },

        //analog type section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 1,
        },

        //analog reset on release section
        {
            .numberOfParameters = MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS,
            .newValueMin        = 0,
            .newValueMax        = 1,
        },
    };

    SysExConf::block_t sysExLayout[static_cast<uint8_t>(System::block_t::AMOUNT)] = {
        //global block
        {
            .numberOfSections = static_cast<uint8_t>(System::Section::global_t::AMOUNT),
            .section          = globalSections,
        },

        //buttons block
        {
            .numberOfSections = static_cast<uint8_t>(System::Section::button_t::AMOUNT),
            .section          = buttonSections,
        },

        //encoder block
        {
            .numberOfSections = static_cast<uint8_t>(System::Section::encoder_t::AMOUNT),
            .section          = encoderSections,
        },

        //analog block
        {
            .numberOfSections = static_cast<uint8_t>(System::Section::analog_t::AMOUNT),
            .section          = analogSections,
        },

        //led block
        {
            .numberOfSections = static_cast<uint8_t>(System::Section::leds_t::AMOUNT),
            .section          = ledSections,
        },

        //display block
        {
            .numberOfSections = static_cast<uint8_t>(System::Section::display_t::AMOUNT),
            .section          = displaySections,
        },

        //touchscreen block
        {
            .numberOfSections = static_cast<uint8_t>(System::Section::touchscreen_t::AMOUNT),
            .section          = touchscreenSections,
        }
    };

    SysExConf::customRequest_t customRequests[NUMBER_OF_CUSTOM_REQUESTS] = {
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
            .requestID     = SYSEX_CR_ENABLE_PROCESSING,
            .connOpenCheck = true,
        },

        {
            .requestID     = SYSEX_CR_DISABLE_PROCESSING,
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
            .requestID     = SYSEX_CR_REBOOT_CDC,
            .connOpenCheck = true,
        },
    };
}    // namespace