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

#include "../interface/display/Config.h"
#include "../interface/digital/output/leds/Constants.h"
#include "Layout.h"

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
    DBMS::init(dbLayout, DB_BLOCKS);
    setHandleRead(board.memoryRead);
    setHandleWrite(board.memoryWrite);

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
    update(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwController, DISPLAY_CONTROLLERS);
    update(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwResolution, DISPLAY_RESOLUTIONS);

    update(DB_BLOCK_LEDS, dbSection_leds_hw, ledHwParameterBlinkTime, BLINK_TIME_MIN);
}

Database database;