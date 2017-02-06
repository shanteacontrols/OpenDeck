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

#pragma once

//simple database management system (dbms)

#include "Config.h"
#include "DataTypes.h"

#ifndef EEPROM_SIZE
#error EEPROM size not defined
#endif

class DBMS
{
    public:
    DBMS();
    void init();
    #ifdef ENABLE_ASYNC_UPDATE
    bool checkQueue();
    #endif
    void clear();
    int16_t read(uint8_t blockID, uint8_t sectionID, uint16_t parameterID = 0);
    bool update(uint8_t blockID, uint8_t sectionID, int16_t parameterID, int16_t newValue, bool async = false);

    protected:
    bool addBlock();
    bool addBlocks(uint8_t numberOfBlocks);
    bool addSection(uint8_t blockID, dbSection_t section);
    void commitLayout();
    void initData(initType_t type = initWipe);

    private:
    #ifdef ENABLE_ASYNC_UPDATE
    void queueData(uint16_t eepromAddress, uint16_t data, uint8_t parameterType);
    #endif

    #ifdef ENABLE_ASYNC_UPDATE
    //update buffer
    uint8_t     eeprom_update_bufer_param_type[EEPROM_UPDATE_BUFFER_SIZE];
    uint16_t    eeprom_update_bufer_value[EEPROM_UPDATE_BUFFER_SIZE];
    uint16_t    eeprom_update_bufer_address[EEPROM_UPDATE_BUFFER_SIZE];
    uint8_t     eeprom_update_buffer_head;
    uint8_t     eeprom_update_buffer_tail;
    #endif

    uint8_t blockCounter;
};
