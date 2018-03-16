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

#include "dbms/src/DBMS.h"
#include "blocks/Blocks.h"

///
/// \addtogroup eeprom
/// @{
///

class Database : public DBMS
{
    public:
    Database();
    static void init();
    static void factoryReset(initType_t type);

    private:
    static bool signatureValid();
    static void writeCustomValues();
};

extern Database database;

/// @}