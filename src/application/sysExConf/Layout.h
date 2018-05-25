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

#include "SysExConf.h"
#include "../interface/Interface.h"

static sysExSection_t midiSections[SYSEX_SECTIONS_MIDI] =
{
    //midi feature section
    {
        .numberOfParameters = MIDI_FEATURES,
        .newValueMin = 0,
        .newValueMax = 1,
    },

    //midi thru section
    {
        .numberOfParameters = MIDI_MERGE_OPTIONS,
        .newValueMin = 0,
        .newValueMax = 0,
    }
};

static sysExSection_t buttonSections[SYSEX_SECTIONS_BUTTONS] =
{
    //type section
    {
        .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG,
        .newValueMin = 0,
        .newValueMax = BUTTON_TYPES-1,
    },

    //midi message type section
    {
        .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG,
        .newValueMin = 0,
        .newValueMax = BUTTON_MESSAGE_TYPES-1,
    },

    //midi id section
    {
        .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG,
        .newValueMin = 0,
        .newValueMax = 127,
    },

    //midi velocity section
    {
        .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG,
        .newValueMin = 1,
        .newValueMax = 127,
    },

    //midi channel section
    {
        .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG,
        .newValueMin = 1,
        .newValueMax = 16,
    }
};

static sysExSection_t encoderSections[SYSEX_SECTIONS_ENCODERS] =
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
        .newValueMax = ENCODING_MODES-1,
    },

    //midi id section
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
    }
};

static sysExSection_t analogSections[SYSEX_SECTIONS_ANALOG] =
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
        .newValueMax = ANALOG_TYPES-1,
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

static sysExSection_t ledSections[SYSEX_SECTIONS_LEDS] =
{
    //led color test section
    {
        .numberOfParameters = MAX_NUMBER_OF_LEDS,
        .newValueMin = 0,
        .newValueMax = LED_COLORS-1,
    },

    //led blink test section
    {
        .numberOfParameters = MAX_NUMBER_OF_LEDS,
        .newValueMin = 0,
        .newValueMax = 1,
    },

    //hardware parameters section
    {
        .numberOfParameters = LED_HARDWARE_PARAMETERS,
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

    //local led control enabled section
    {
        .numberOfParameters = MAX_NUMBER_OF_LEDS,
        .newValueMin = 0,
        .newValueMax = 1,
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

static sysExSection_t displaySections[SYSEX_SECTIONS_DISPLAY] =
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

sysExBlock_t sysExLayout[SYSEX_BLOCKS] =
{
    //midi block
    {
        .numberOfSections = SYSEX_SECTIONS_MIDI,
        .section = midiSections,
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
    },
};