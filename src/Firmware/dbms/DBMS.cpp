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

#include "DBMS.h"

typedef struct
{
    uint8_t sections;
    uint16_t blockStartAddress;
    uint16_t sectionAddress[MAX_SECTIONS];
    dbSection_t section[MAX_SECTIONS];
} blockDescriptor;

blockDescriptor block[MAX_BLOCKS];

//inline functions

inline uint16_t getSectionAddress(uint8_t blockID, uint8_t sectionID)
{
    return block[blockID].blockStartAddress+block[blockID].sectionAddress[sectionID];
};

inline uint16_t getBlockAddress(uint8_t blockID)
{
    return block[blockID].blockStartAddress;
};

inline sectionParameterType_t getParameterType(uint8_t blockID, uint8_t sectionID)
{
    return block[blockID].section[sectionID].parameterType;
}

DBMS::DBMS()
{
    #ifdef ENABLE_ASYNC_UPDATE
    for (int i=0; i<EEPROM_UPDATE_BUFFER_SIZE; i++)
    {
        eeprom_update_bufer_param_type[i] = 0;
        eeprom_update_bufer_value[i] = 0;
        eeprom_update_bufer_address[i] = 0;
    }

    eeprom_update_buffer_head = 0;
    eeprom_update_buffer_tail = 0;
    #endif
}

int16_t DBMS::read(uint8_t blockID, uint8_t sectionID, uint16_t parameterID)
{
    uint16_t startAddress = getSectionAddress(blockID, sectionID);
    uint8_t parameterType = getParameterType(blockID, sectionID);

    uint8_t arrayIndex;
    uint8_t parameterIndex;

    switch(parameterType)
    {
        case BIT_PARAMETER:
        arrayIndex = parameterID/8;
        parameterIndex = parameterID - 8*arrayIndex;
        startAddress += arrayIndex;
        return bitRead(eeprom_read_byte((uint8_t*)startAddress), parameterIndex);
        break;

        case BYTE_PARAMETER:
        startAddress += parameterID;
        return eeprom_read_byte((uint8_t*)startAddress);
        break;

        case WORD_PARAMETER:
        startAddress += ((uint16_t)parameterID*2);
        return eeprom_read_word((uint16_t*)startAddress);
        break;
    }

    return 0;
}

bool DBMS::update(uint8_t blockID, uint8_t sectionID, int16_t parameterID, int16_t newValue, bool async)
{
    uint16_t startAddress = getSectionAddress(blockID, sectionID);

    if (startAddress > EEPROM_SIZE)
    {
        #ifdef DEBUG
        printf_P(PSTR("Requested address out of EEPROM memory range\n"));
        #endif
        return 0;
    }

    uint8_t parameterType = getParameterType(blockID, sectionID);

    uint8_t arrayIndex;
    uint8_t arrayValue;
    uint8_t parameterIndex;

    switch(parameterType)
    {
        case BIT_PARAMETER:
        arrayIndex = parameterID/8;
        parameterIndex = parameterID - 8*arrayIndex;
        arrayValue = eeprom_read_byte((uint8_t*)startAddress+arrayIndex);
        bitWrite(arrayValue, parameterIndex, newValue);
        #ifdef ENABLE_ASYNC_UPDATE
        if (async)
        {
            queueData(startAddress+arrayIndex, arrayValue, BIT_PARAMETER);
            return true;
        }
        else
        {
            eeprom_update_byte((uint8_t*)startAddress+arrayIndex, arrayValue);
            return (arrayValue == eeprom_read_byte((uint8_t*)startAddress+arrayIndex));
        }
        #else
        eeprom_update_byte((uint8_t*)startAddress+arrayIndex, arrayValue);
        return (arrayValue == eeprom_read_byte((uint8_t*)startAddress+arrayIndex));
        #endif
        break;

        case BYTE_PARAMETER:
        #ifdef ENABLE_ASYNC_UPDATE
        if (async)
        {
            queueData(startAddress+parameterID, newValue, BYTE_PARAMETER);
            return true;
        }
        else
        {
            eeprom_update_byte((uint8_t*)startAddress+parameterID, newValue);
            return (newValue == eeprom_read_byte((uint8_t*)startAddress+parameterID));
        }
        #else
        eeprom_update_byte((uint8_t*)startAddress+parameterID, newValue);
        return (newValue == eeprom_read_byte((uint8_t*)startAddress+parameterID));
        #endif
        break;

        case WORD_PARAMETER:
        #ifdef ENABLE_ASYNC_UPDATE
        if (async)
        {
            queueData(startAddress+parameterID, newValue, WORD_PARAMETER);
            return true;
        }
        else
        {
            eeprom_update_word((uint16_t*)startAddress+parameterID, newValue);
            return (newValue == (int16_t)eeprom_read_word((uint16_t*)startAddress+parameterID));
        }
        #else
        eeprom_update_word((uint16_t*)startAddress+parameterID, newValue);
        return (newValue == (int16_t)eeprom_read_word((uint16_t*)startAddress+parameterID));
        #endif
        break;
    }

    return 0;
}

void DBMS::clear()
{
    for (int i=0; i<EEPROM_SIZE; i++)
        eeprom_update_byte((uint8_t*)i, 0xFF);
}

bool DBMS::addBlock()
{
    if (blockCounter >= MAX_BLOCKS)
        return false;

    blockCounter++;
    return true;
}

bool DBMS::addBlocks(uint8_t numberOfBlocks)
{
    if (blockCounter+numberOfBlocks >= MAX_BLOCKS)
        return false;

    blockCounter += numberOfBlocks;
    return true;
}

bool DBMS::addSection(uint8_t blockID, dbSection_t section)
{
    if (block[blockID].sections >= MAX_SECTIONS)
        return false;

    block[blockID].section[block[blockID].sections].parameterType = section.parameterType;
    block[blockID].section[block[blockID].sections].preserveOnPartialReset = section.preserveOnPartialReset;
    block[blockID].section[block[blockID].sections].defaultValue = section.defaultValue;
    block[blockID].section[block[blockID].sections].parameters = section.parameters;

    block[blockID].sections++;
    return true;
}

void DBMS::commitLayout()
{
    for (int i=0; i<blockCounter; i++)
    {
        uint16_t memory_usage = 0;

        for (int j=0; j<block[i].sections; j++)
        {
            if (!j)
            {
                //first section address is always 0
                block[i].sectionAddress[0] = 0;
            }
            else
            {
                switch(block[i].section[j-1].parameterType)
                {
                    case BIT_PARAMETER:
                    block[i].sectionAddress[j] = ((block[i].section[j].parameters % 8 != 0) + block[i].section[j-1].parameters/8) + block[i].sectionAddress[j-1];
                    break;

                    case BYTE_PARAMETER:
                    block[i].sectionAddress[j] = block[i].section[j-1].parameters + block[i].sectionAddress[j-1];
                    break;

                    case WORD_PARAMETER:
                    block[i].sectionAddress[j] = 2*block[i].section[j-1].parameters + block[i].sectionAddress[j-1];
                    break;
                }
            }
        }

        uint8_t lastSection = block[i].sections-1;

        switch(block[i].section[lastSection].parameterType)
        {
            case BIT_PARAMETER:
            memory_usage = block[i].sectionAddress[lastSection]+((block[i].section[lastSection].parameters%8 != 0) + block[i].section[lastSection].parameters/8);
            break;

            case BYTE_PARAMETER:
            memory_usage = block[i].sectionAddress[lastSection] + block[i].section[lastSection].parameters;
            break;

            case WORD_PARAMETER:
            memory_usage = block[i].sectionAddress[lastSection] + 2*block[i].section[lastSection].parameters;
            break;
        }

        if (i < blockCounter-1)
            block[i+1].blockStartAddress = block[i].blockStartAddress + memory_usage;
    }
}

void DBMS::initData(initType_t type)
{
    for (int i=0; i<blockCounter; i++)
    {
        for (int j=0; j<block[i].sections; j++)
        {
            if (block[i].section[j].preserveOnPartialReset && (type == initPartial))
                continue;

            uint16_t startAddress = getSectionAddress(i, j);
            uint8_t parameterType = getParameterType(i, j);
            uint8_t defaultValue = block[i].section[j].defaultValue;
            uint8_t numberOfParameters = block[i].section[j].parameters;

            switch(parameterType)
            {
                case BIT_PARAMETER:
                for (int i=0; i<numberOfParameters/8+1; i++)
                    eeprom_update_byte((uint8_t*)startAddress+i, defaultValue);
                break;

                case BYTE_PARAMETER:
                while (numberOfParameters--)
                {
                    if (defaultValue == AUTO_INCREMENT)
                        eeprom_update_byte((uint8_t*)startAddress+numberOfParameters, numberOfParameters);
                    else
                        eeprom_update_byte((uint8_t*)startAddress+numberOfParameters, defaultValue);
                }
                break;

                case WORD_PARAMETER:
                while (numberOfParameters--)
                {
                    if (defaultValue == AUTO_INCREMENT)
                        eeprom_update_word((uint16_t*)(uint16_t)(startAddress+(numberOfParameters*2)), numberOfParameters);
                    else
                        eeprom_update_word((uint16_t*)(uint16_t)(startAddress+(numberOfParameters*2)), (uint16_t)defaultValue);
                }
            }
        }
    }
}
