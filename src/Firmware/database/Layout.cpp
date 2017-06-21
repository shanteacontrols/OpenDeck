/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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

#include "Database.h"
#include "../board/Board.h"

///
/// \brief Creates database layout by defining blocks and sections.
///
void Database::createLayout()
{
    DBMS::addBlocks(DB_BLOCKS);

    dbSection_t section;

    {
        //MIDI block

        //midi feature section
        section.numberOfParameters = MIDI_FEATURES;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_MIDI, section);

        //midi channel section
        section.numberOfParameters = MIDI_CHANNELS;
        section.defaultValue = 1;
        section.autoIncrement = false;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_MIDI, section);
    }

    {
        //button block

        //type section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_BUTTON, section);

        //midi message type section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_BUTTON, section);

        //midi id section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.autoIncrement = true;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_BUTTON, section);
    }

    {
        //encoder block

        //encoder enabled section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ENCODER, section);

        //encoder inverted section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ENCODER, section);

        //encoding mode section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ENCODER, section);

        //midi id section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.defaultValue = 0;
        section.autoIncrement = true;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ENCODER, section);
    }

    {
        //analog block

        //analog enabled section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ANALOG, section);

        //analog inverted section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ANALOG, section);

        //analog type section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ANALOG, section);

        //midi id section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.autoIncrement = true;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ANALOG, section);

        //lower cc limit
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ANALOG, section);

        //upper cc limit
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 127;
        section.autoIncrement = false;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ANALOG, section);
    }

    {
        //led block

        //hardware parameters section
        section.numberOfParameters = LED_HARDWARE_PARAMETERS;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_LED, section);

        //activation id section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.defaultValue = 0;
        section.autoIncrement = true;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_LED, section);

        //rgb enabled section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_LED, section);

        //local led control enabled section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.defaultValue = 0;
        section.autoIncrement = false;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_LED, section);
    }

    {
        //ID block

        section.numberOfParameters = ID_BYTES;
        section.defaultValue = UNIQUE_ID;
        section.autoIncrement = false;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ID, section);
    }
}
