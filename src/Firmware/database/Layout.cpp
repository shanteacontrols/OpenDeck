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
        section.parameters = MIDI_FEATURES;
        section.defaultValue = 0;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_MIDI, section);

        section.parameters = MIDI_CHANNELS;
        section.defaultValue = 1;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_MIDI, section);
    }

    {
        section.parameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_BUTTON, section);

        section.parameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_BUTTON, section);

        section.parameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.defaultValue = AUTO_INCREMENT;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_BUTTON, section);
    }

    {
        section.parameters = MAX_NUMBER_OF_ENCODERS;
        section.defaultValue = 0;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ENCODER, section);

        section.parameters = MAX_NUMBER_OF_ENCODERS;
        section.defaultValue = 0;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ENCODER, section);

        section.parameters = MAX_NUMBER_OF_ENCODERS;
        section.defaultValue = 0;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ENCODER, section);

        section.parameters = MAX_NUMBER_OF_ENCODERS;
        section.defaultValue = AUTO_INCREMENT;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ENCODER, section);
    }

    {
        section.parameters = MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ANALOG, section);

        section.parameters = MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ANALOG, section);

        section.parameters = MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ANALOG, section);

        section.parameters = MAX_NUMBER_OF_ANALOG;
        section.defaultValue = AUTO_INCREMENT;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ANALOG, section);

        section.parameters = MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 0;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ANALOG, section);

        section.parameters = MAX_NUMBER_OF_ANALOG;
        section.defaultValue = 127;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ANALOG, section);
    }

    {
        section.parameters = LED_HARDWARE_PARAMETERS;
        section.defaultValue = 0;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_LED, section);

        section.parameters = MAX_NUMBER_OF_LEDS;
        section.defaultValue = AUTO_INCREMENT;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_LED, section);

        section.parameters = MAX_NUMBER_OF_LEDS;
        section.defaultValue = 0;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_LED, section);

        section.parameters = MAX_NUMBER_OF_LEDS;
        section.defaultValue = 0;
        section.parameterType = BIT_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_LED, section);
    }

    {
        section.parameters = ID_OFFSET;
        section.defaultValue = UNIQUE_ID;
        section.parameterType = BYTE_PARAMETER;
        section.preserveOnPartialReset = 0;

        DBMS::addSection(DB_BLOCK_ID, section);
    }
}
