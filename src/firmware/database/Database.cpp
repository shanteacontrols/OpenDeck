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

#include "Database.h"
#include "../interface/display/Config.h"
#include <avr/eeprom.h>
#include "blocks/LEDs.h"
#include "../interface/digital/output/leds/DataTypes.h"
#include "../interface/digital/output/leds/Constants.h"
#include "Layout.h"

bool memoryRead(uint32_t address, sectionParameterType_t type, int32_t &value)
{
    switch(type)
    {
        case BIT_PARAMETER:
        case BYTE_PARAMETER:
        case HALFBYTE_PARAMETER:
        value = eeprom_read_byte((uint8_t*)address);
        break;

        case WORD_PARAMETER:
        value = eeprom_read_word((uint16_t*)address);
        break;

        default:
        // case DWORD_PARAMETER:
        value = eeprom_read_dword((uint32_t*)address);
        break;
    }

    return true;
}

bool memoryWrite(uint32_t address, int32_t value, sectionParameterType_t type)
{
    switch(type)
    {
        case BIT_PARAMETER:
        case BYTE_PARAMETER:
        case HALFBYTE_PARAMETER:
        eeprom_update_byte((uint8_t*)address, value);
        break;

        case WORD_PARAMETER:
        eeprom_update_word((uint16_t*)address, value);
        break;

        default:
        // case DWORD_PARAMETER:
        eeprom_update_dword((uint32_t*)address, value);
        break;
    }

    return true;
}

///
/// \brief Default constructor.
///
Database::Database()
{
    
}

///
/// \brief Initializes database.
///
void Database::init()
{
    DBMS::init();
    setHandleRead(memoryRead);
    setHandleWrite(memoryWrite);
    commitLayout(dbLayout, DB_BLOCKS);

    if (!signatureValid())
    {
        factoryReset(initFull);
    }
}

///
/// \brief Performs factory reset of data in database.
/// @param [in] type Factory reset type. See initType_t enumeration.
///
void Database::factoryReset(initType_t type)
{
    if (type == initFull)
        DBMS::clear();

    initData(type);
    writeCustomValues();
}

///
/// \brief Checks if database has been already initialized by checking DB_BLOCK_ID.
/// \returns True if valid, false otherwise.
///
bool Database::signatureValid()
{
    //check if all bytes up to START_OFFSET address match unique id

    for (int i=0; i<ID_BYTES; i++)
    {
        if (read(DB_BLOCK_ID, 0, i) != UNIQUE_ID)
            return false;
    }

    return true;
}

///
/// \brief Writes custom values to specific indexes which can't be generalized within database section.
///
void Database::writeCustomValues()
{
    update(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureMIDIeventTime, MIN_MESSAGE_RETENTION_TIME);
    update(DB_BLOCK_LEDS, dbSection_leds_hw, ledHwParameterBlinkTime, BLINK_TIME_MIN);
}

Database database;