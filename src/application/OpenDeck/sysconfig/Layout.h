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
#include "OpenDeck/sysconfig/blocks/Global.h"
#include "interface/digital/output/leds/LEDs.h"
#include "interface/digital/input/buttons/Buttons.h"
#include "interface/digital/input/encoders/Encoders.h"
#include "interface/digital/input/encoders/Constants.h"
#include "interface/analog/Analog.h"

namespace
{
    SysExConf::section_t globalSections[SYSEX_SECTIONS_GLOBAL] =
    {
        //midi feature section
        {
            .numberOfParameters = MIDI_FEATURES,
            .newValueMin = 0,
            .newValueMax = 1,
        },

        //midi merge section
        {
            .numberOfParameters = MIDI_MERGE_OPTIONS,
            .newValueMin = 0,
            .newValueMax = 0,
        },

        //global settings section
        {
            .numberOfParameters = SYSTEM_OPTIONS,
            .newValueMin = 0,
            .newValueMax = 0,
        },
    };

    SysExConf::section_t buttonSections[SYSEX_SECTIONS_BUTTONS] =
    {
        //type section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS,
            .newValueMin = 0,
            .newValueMax = static_cast<SysExConf::sysExParameter_t>(Interface::digital::input::Buttons::type_t::AMOUNT)-1,
        },

        //midi message type section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS,
            .newValueMin = 0,
            .newValueMax = static_cast<SysExConf::sysExParameter_t>(Interface::digital::input::Buttons::messageType_t::AMOUNT)-1,
        },

        //midi id section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS,
            .newValueMin = 0,
            .newValueMax = 127,
        },

        //midi velocity section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS,
            .newValueMin = 1,
            .newValueMax = 127,
        },

        //midi channel section
        {
            .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS,
            .newValueMin = 1,
            .newValueMax = 16,
        }
    };

    SysExConf::section_t encoderSections[SYSEX_SECTIONS_ENCODERS] =
    {
        //encoder enabled section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin = 0,
            .newValueMax = 1,
        },

        //encoder inverted section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin = 0,
            .newValueMax = 1,
        },

        //encoding mode section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin = 0,
            .newValueMax = static_cast<SysExConf::sysExParameter_t>(Interface::digital::input::Encoders::type_t::AMOUNT)-1,
        },

        //midi id section, lsb
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin = 0,
            .newValueMax = 127,
        },

        //midi channel section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin = 1,
            .newValueMax = 16,
        },

        //pulses per step section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin = 2,
            .newValueMax = 4,
        },

        //acceleration section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin = 0,
            .newValueMax = ENCODERS_MAX_ACCELERATION_OPTIONS,
        },

        //midi id section, msb
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin = 0,
            .newValueMax = 127,
        },

        //remote sync section
        {
            .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
            .newValueMin = 0,
            .newValueMax = 1,
        },
    };

    SysExConf::section_t analogSections[SYSEX_SECTIONS_ANALOG] =
    {
        //analog enabled section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .newValueMin = 0,
            .newValueMax = 1,
        },

        //analog inverted section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .newValueMin = 0,
            .newValueMax = 1,
        },

        //analog type section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .newValueMin = 0,
            .newValueMax = static_cast<SysExConf::sysExParameter_t>(Interface::analog::Analog::type_t::AMOUNT)-1,
        },

        //midi id section, lsb
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .newValueMin = 0,
            .newValueMax = 127,
        },

        //midi id section, msb
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .newValueMin = 0,
            .newValueMax = 127,
        },

        //lower cc limit, lsb
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .newValueMin = 0,
            .newValueMax = 127,
        },

        //lower cc limit, msb
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .newValueMin = 0,
            .newValueMax = 127,
        },

        //upper cc limit, lsb
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .newValueMin = 0,
            .newValueMax = 127,
        },

        //upper cc limit, msb
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .newValueMin = 0,
            .newValueMax = 127,
        },

        //midi channel section
        {
            .numberOfParameters = MAX_NUMBER_OF_ANALOG,
            .newValueMin = 1,
            .newValueMax = 16,
        }
    };

    SysExConf::section_t ledSections[SYSEX_SECTIONS_LEDS] =
    {
        //led color test section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS,
            .newValueMin = 0,
            .newValueMax = static_cast<SysExConf::sysExParameter_t>(Interface::digital::output::LEDs::color_t::AMOUNT)-1,
        },

        //led blink test section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS,
            .newValueMin = 0,
            .newValueMax = 1,
        },

        //global parameters section
        {
            .numberOfParameters = static_cast<SysExConf::sysExParameter_t>(Interface::digital::output::LEDs::setting_t::AMOUNT),
            .newValueMin = 0,
            .newValueMax = 0,
        },

        //activation note section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS,
            .newValueMin = 0,
            .newValueMax = 127,
        },

        //rgb enabled section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS,
            .newValueMin = 0,
            .newValueMax = 1,
        },

        //led control type section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS,
            .newValueMin = 0,
            .newValueMax = static_cast<SysExConf::sysExParameter_t>(Interface::digital::output::LEDs::controlType_t::AMOUNT)-1,
        },

        //single led velocity value section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS,
            .newValueMin = 1,
            .newValueMax = 127,
        },

        //midi channel section
        {
            .numberOfParameters = MAX_NUMBER_OF_LEDS,
            .newValueMin = 1,
            .newValueMax = 16,
        }
    };

    SysExConf::section_t displaySections[SYSEX_SECTIONS_DISPLAY] =
    {
        //features section
        {
            .numberOfParameters = DISPLAY_FEATURES,
            .newValueMin = 0,
            .newValueMax = 0,
        },

        //hw section
        {
            .numberOfParameters = DISPLAY_HW_PARAMETERS,
            .newValueMin = 0,
            .newValueMax = 0,
        }
    };

    SysExConf::block_t sysExLayout[SYSEX_BLOCKS] =
    {
        //global block
        {
            .numberOfSections = SYSEX_SECTIONS_GLOBAL,
            .section = globalSections,
        },

        //buttons block
        {
            .numberOfSections = SYSEX_SECTIONS_BUTTONS,
            .section = buttonSections,
        },

        //encoder block
        {
            .numberOfSections = SYSEX_SECTIONS_ENCODERS,
            .section = encoderSections,
        },

        //analog block
        {
            .numberOfSections = SYSEX_SECTIONS_ANALOG,
            .section = analogSections,
        },

        //led block
        {
            .numberOfSections = SYSEX_SECTIONS_LEDS,
            .section = ledSections,
        },

        //display block
        {
            .numberOfSections = SYSEX_SECTIONS_DISPLAY,
            .section = displaySections,
        }
    };

    SysExConf::customRequest_t customRequests[NUMBER_OF_CUSTOM_REQUESTS] =
    {
        {
            .requestID = SYSEX_CR_FIRMWARE_VERSION,
            .connOpenCheck = true
        },

        {
            .requestID = SYSEX_CR_HARDWARE_VERSION,
            .connOpenCheck = false
        },

        {
            .requestID = SYSEX_CR_FIRMWARE_HARDWARE_VERSION,
            .connOpenCheck = true
        },

        {
            .requestID = SYSEX_CR_REBOOT_APP,
            .connOpenCheck = true
        },

        {
            .requestID = SYSEX_CR_REBOOT_BTLDR,
            .connOpenCheck = true
        },

        {
            .requestID = SYSEX_CR_FACTORY_RESET,
            .connOpenCheck = true
        },

        {
            .requestID = SYSEX_CR_MAX_COMPONENTS,
            .connOpenCheck = true
        },

        {
            .requestID = SYSEX_CR_ENABLE_PROCESSING,
            .connOpenCheck = true
        },

        {
            .requestID = SYSEX_CR_DISABLE_PROCESSING,
            .connOpenCheck = true
        },

        {
            .requestID = SYSEX_CR_DAISY_CHAIN,
            .connOpenCheck = false
        },

        {
            .requestID = SYSEX_CR_SUPPORTED_PRESETS,
            .connOpenCheck = true
        },
    };
}