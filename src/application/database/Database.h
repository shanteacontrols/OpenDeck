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
    Database(bool (*readCallback)(uint32_t address, sectionParameterType_t type, int32_t &value), bool (*writeCallback)(uint32_t address, int32_t value, sectionParameterType_t type)) :
    DBMS(readCallback, writeCallback)
    {}

    void init();
    void factoryReset(initType_t type);
    bool signatureValid();

    private:
    void writeCustomValues();
};