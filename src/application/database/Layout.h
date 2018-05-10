/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Database.h"
#include "../board/Board.h"

static dbSection_t midiSections[DB_SECTIONS_MIDI] =
{
    //midi feature section
    {
        .numberOfParameters = MIDI_FEATURES,
        .parameterType = BIT_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //midi thru section
    {
        .numberOfParameters = MIDI_MERGE_OPTIONS,
        .parameterType = HALFBYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    }
};

static dbSection_t buttonSections[DB_SECTIONS_BUTTONS] =
{
    //type section
    {
        .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG,
        .parameterType = BIT_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //midi message type section
    {
        .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG,
        .parameterType = BYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //midi id section
    {
        .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG,
        .parameterType = BYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = true,
        .address = 0
    },

    //midi velocity section
    {
        .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG,
        .parameterType = BYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 127,
        .autoIncrement = false,
        .address = 0
    },

    //midi channel section
    {
        .numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG,
        .parameterType = HALFBYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    }
};

static dbSection_t encoderSections[DB_SECTIONS_ENCODERS] =
{
    //encoder enabled section
    {
        .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
        .parameterType = BIT_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //encoder inverted section
    {
        .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
        .parameterType = BIT_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //encoding mode section
    {
        .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
        .parameterType = BYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //midi id section
    {
        .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
        .parameterType = BYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = true,
        .address = 0
    },

    //midi channel section
    {
        .numberOfParameters = MAX_NUMBER_OF_ENCODERS,
        .parameterType = HALFBYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    }
};

static dbSection_t analogSections[DB_SECTIONS_ANALOG] =
{
    //analog enabled section
    {
        .numberOfParameters = MAX_NUMBER_OF_ANALOG,
        .parameterType = BIT_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //analog inverted section
    {
        .numberOfParameters = MAX_NUMBER_OF_ANALOG,
        .parameterType = BIT_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //analog type section
    {
        .numberOfParameters = MAX_NUMBER_OF_ANALOG,
        .parameterType = BYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //midi id section
    {
        .numberOfParameters = MAX_NUMBER_OF_ANALOG,
        .parameterType = WORD_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = true,
        .address = 0
    },

    //lower cc limit
    {
        .numberOfParameters = MAX_NUMBER_OF_ANALOG,
        .parameterType = WORD_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //upper cc limit
    {
        .numberOfParameters = MAX_NUMBER_OF_ANALOG,
        .parameterType = WORD_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 16383,
        .autoIncrement = false,
        .address = 0
    },

    //midi channel section
    {
        .numberOfParameters = MAX_NUMBER_OF_ANALOG,
        .parameterType = HALFBYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    }
};

static dbSection_t ledSections[DB_SECTIONS_LEDS] =
{
    //hardware parameters section
    {
        .numberOfParameters = LED_HARDWARE_PARAMETERS,
        .parameterType = BYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //activation id section
    {
        .numberOfParameters = MAX_NUMBER_OF_LEDS,
        .parameterType = BYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = true,
        .address = 0
    },

    //rgb enabled section
    {
        .numberOfParameters = MAX_NUMBER_OF_RGB_LEDS,
        .parameterType = BIT_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //local led control enabled section
    {
        .numberOfParameters = MAX_NUMBER_OF_LEDS,
        .parameterType = BIT_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //single velocity value section
    {
        .numberOfParameters = MAX_NUMBER_OF_LEDS,
        .parameterType = BYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 127,
        .autoIncrement = false,
        .address = 0
    },

    //midi channel section
    {
        .numberOfParameters = MAX_NUMBER_OF_LEDS,
        .parameterType = HALFBYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    }
};

static dbSection_t displaySections[DB_SECTIONS_DISPLAY] =
{
    //features section
    {
        .numberOfParameters = DISPLAY_FEATURES,
        .parameterType = BYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    },

    //hw section
    {
        .numberOfParameters = DISPLAY_HW_PARAMETERS,
        .parameterType = BYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = 0,
        .autoIncrement = false,
        .address = 0
    }
};

static dbSection_t idSections[1] =
{
    {
        .numberOfParameters = ID_BYTES,
        .parameterType = BYTE_PARAMETER,
        .preserveOnPartialReset = 0,
        .defaultValue = UNIQUE_ID,
        .autoIncrement = false,
        .address = 0
    },
};

dbBlock_t dbLayout[DB_BLOCKS] =
{
    //midi block
    {
        .address = 0,
        .numberOfSections = DB_SECTIONS_MIDI,
        .section = midiSections,
    },

    //buttons block
    {
        .address = 0,
        .numberOfSections = DB_SECTIONS_BUTTONS,
        .section = buttonSections,
    },

    //encoder block
    {
        .address = 0,
        .numberOfSections = DB_SECTIONS_ENCODERS,
        .section = encoderSections,
    },

    //analog block
    {
        .address = 0,
        .numberOfSections = DB_SECTIONS_ANALOG,
        .section = analogSections,
    },

    //led block
    {
        .address = 0,
        .numberOfSections = DB_SECTIONS_LEDS,
        .section = ledSections,
    },

    //display block
    {
        .address = 0,
        .numberOfSections = DB_SECTIONS_DISPLAY,
        .section = displaySections,
    },

    //id block
    {
        .address = 0,
        .numberOfSections = 1,
        .section = idSections,
    },
};
