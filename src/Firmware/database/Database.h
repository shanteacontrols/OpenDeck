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

#include "Config.h"
#include "blocks/Blocks.h"

#ifdef ENABLE_ASYNC_UPDATE
#define EEPROM_UPDATE_BUFFER_SIZE   64
#endif

typedef enum
{
    BIT_PARAMETER,
    BYTE_PARAMETER,
    WORD_PARAMETER
} sectionParameterType_t;

typedef enum
{
    factoryReset_wipeRestore, //clear eeprom, restore defaults
    factoryReset_restore, //update eeprom with defaults
    factoryReset_partial //partially restore defaults
} factoryResetType_t;

#if VALUE_BYTES == 1
typedef uint8_t eepromValue_t;
#elif VALUE_BYTES == 2
typedef uint16_t eepromValue_t;
#else
#error Incorrect EEPROM value size
#endif

typedef struct
{
    uint8_t sections;
    uint16_t blockStartAddress;
    uint16_t sectionAddress[MAX_SECTIONS];
    uint16_t sectionParameters[MAX_SECTIONS];
    sectionParameterType_t sectionParameterType[MAX_SECTIONS];
    bool preserveOnPartialReset[MAX_SECTIONS];
    eepromValue_t defaultValue[MAX_SECTIONS];
} blockDescriptor;

//default controller settings

class Database
{
    public:
    Database();
    #ifdef ENABLE_ASYNC_UPDATE
    bool checkQueue();
    #endif
    void clearEEPROM();
    void init();
    void initSettings(bool partialReset);
    void factoryReset(factoryResetType_t type);
    void createMemoryLayout();
    void createSectionAddresses();
    uint16_t read(uint8_t blockID, uint8_t sectionID, uint16_t parameterID = 0);
    bool update(uint8_t blockID, uint8_t sectionID, int16_t parameterID, int16_t newValue, bool async = false);
    blockDescriptor blocks[CONF_BLOCKS];

    private:
    inline uint16_t getSectionAddress(uint8_t blockID, uint8_t sectionID)
    {
        return blocks[blockID].blockStartAddress+blocks[blockID].sectionAddress[sectionID] + START_OFFSET;
    };
    inline uint16_t getBlockAddress(uint8_t blockID)
    {
        return blocks[blockID].blockStartAddress+START_OFFSET;
    };
    inline uint8_t getParameterType(uint8_t blockID, uint8_t sectionID)
    {
        return blocks[blockID].sectionParameterType[sectionID];
    }
    void initProgramSettings(bool partialReset);
    void initUserScales(bool partialReset);
    void initPadCalibration(bool partialReset);
    void initMIDIsettings(bool partialReset);
    void checkReset();
    void writeSignature();
    #ifdef ENABLE_ASYNC_UPDATE
    void queueData(uint16_t eepromAddress, uint16_t data, uint8_t parameterType);
    #endif

    struct
    {
        uint8_t major;
        uint8_t minor;
        uint8_t revision;
        uint16_t crc;
    } firmwareVersion;

    #ifdef ENABLE_ASYNC_UPDATE
    //update buffer
    uint8_t     eeprom_update_bufer_param_type[EEPROM_UPDATE_BUFFER_SIZE];
    uint16_t    eeprom_update_bufer_value[EEPROM_UPDATE_BUFFER_SIZE];
    uint16_t    eeprom_update_bufer_address[EEPROM_UPDATE_BUFFER_SIZE];
    uint8_t     eeprom_update_buffer_head;
    uint8_t     eeprom_update_buffer_tail;
    #endif
};

extern Database database;
