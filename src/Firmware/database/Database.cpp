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

Database::Database()
{
    
}

void Database::init()
{
    createLayout();
    DBMS::commitLayout();

    if (!signatureValid())
    {
        DBMS::initData(initWipe);
    }
}

void Database::factoryReset(initType_t type)
{
    DBMS::initData(type);
}

bool Database::signatureValid()
{
    //check if all bytes up to START_OFFSET address match unique id

    for (int i=0; i<ID_OFFSET; i++)
    {
        if (DBMS::read(DB_BLOCK_ID, 0, i) != UNIQUE_ID)
            return false;
    }

    return true;
}

Database database;