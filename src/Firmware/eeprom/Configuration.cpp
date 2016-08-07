#include "Configuration.h"
#include "../sysex/SysEx.h"
#include "../interface/settings/Settings.h"

Configuration::Configuration()    {

    //def const

}

void Configuration::init() {

    createMemoryLayout();
    createSectionAddresses();

    //if ID bytes haven't been written to EEPROM on specified address,
    //write default configuration to EEPROM
    if  (!(

    (eeprom_read_byte((uint8_t*)ID_LOCATION_0) == UNIQUE_ID) &&
    (eeprom_read_byte((uint8_t*)ID_LOCATION_1) == UNIQUE_ID) &&
    (eeprom_read_byte((uint8_t*)ID_LOCATION_2) == UNIQUE_ID)

    ))  factoryReset();

}

void Configuration::factoryReset()  {

    clearEEPROM();
    writeConfiguration();

}

void Configuration::clearEEPROM()    {

    for (int i=0; i<1024; i++) eeprom_update_byte((uint8_t*)i, 0xFF);

}

bool Configuration::writeParameter(uint8_t blockID, uint8_t sectionID, uint8_t parameterID, uint8_t newValue)    {

    uint16_t startAddress = getSectionAddress(blockID, sectionID);
    uint8_t parameterType = getParameterType(blockID, sectionID);

    uint8_t arrayIndex;
    uint8_t arrayValue;
    uint8_t parameterIndex;

    switch(parameterType)   {

        case BIT_PARAMETER:
        arrayIndex = parameterID/8;
        parameterIndex = parameterID - 8*arrayIndex;
        arrayValue = eeprom_read_byte((uint8_t*)startAddress+arrayIndex);
        if (newValue == DEFAULT_VALUE)
            bitWrite(arrayValue, parameterIndex, (blocks[blockID].defaultValue[sectionID] & 0x01));
        else bitWrite(arrayValue, parameterIndex, newValue);
        eeprom_update_byte((uint8_t*)startAddress+arrayIndex, arrayValue);
        return (arrayValue == eeprom_read_byte((uint8_t*)startAddress+arrayIndex));
        break;

        case BYTE_PARAMETER:
        if (newValue == DEFAULT_VALUE)    {

            eeprom_update_byte((uint8_t*)startAddress+parameterID, blocks[blockID].defaultValue[sectionID]);
            return (blocks[blockID].defaultValue[sectionID] == eeprom_read_byte((uint8_t*)startAddress+parameterID));

        }   else {

            eeprom_update_byte((uint8_t*)startAddress+parameterID, newValue);
            return (newValue == eeprom_read_byte((uint8_t*)startAddress+parameterID));

        }
        break;

    }   return 0;

}

void Configuration::createSectionAddresses()   {

    for (int i=0; i<NUMBER_OF_BLOCKS; i++)  {

        uint16_t memory_usage = 0;

        for (int j=0; j<blocks[i].sections; j++)    {

            if (!j) {

                //first section address is always 0
                blocks[i].subsectionAddress[0] = 0;

                }   else {

                switch(blocks[i].subsectionType[j-1])   {

                    //calculate address of current section by adding number of parameters in last section
                    //and adding last section address

                    case BIT_PARAMETER:
                    blocks[i].subsectionAddress[j] = ((blocks[i].sectionParameters[j]%8 != 0) + blocks[i].sectionParameters[j-1]/8) + blocks[i].subsectionAddress[j-1];
                    break;

                    case BYTE_PARAMETER:
                    blocks[i].subsectionAddress[j] = blocks[i].sectionParameters[j-1] + blocks[i].subsectionAddress[j-1];
                    break;

                }

            }

        }

        uint8_t lastSection = blocks[i].sections-1;

        switch(blocks[i].subsectionType[lastSection]) {

            case BIT_PARAMETER:
            memory_usage = blocks[i].subsectionAddress[lastSection]+((blocks[i].sectionParameters[lastSection]%8 != 0)+blocks[i].sectionParameters[lastSection]/8);
            break;

            case BYTE_PARAMETER:
            memory_usage = blocks[i].subsectionAddress[lastSection]+blocks[i].sectionParameters[lastSection];
            break;

        }

        if (i < NUMBER_OF_BLOCKS-1) {

            blocks[i+1].blockStartAddress = blocks[i].blockStartAddress + memory_usage;

        }

    }

}

void Configuration::writeConfiguration()   {

    for (int i=0; i<NUMBER_OF_BLOCKS; i++)  {

        for (int j=0; j<blocks[i].sections; j++) {

            uint16_t startAddress = getSectionAddress(i, j);
            uint8_t parameterType = getParameterType(i, j);
            uint8_t defaultValue = blocks[i].defaultValue[j];
            uint8_t numberOfParameters = blocks[i].sectionParameters[j];

            switch(parameterType)   {

                case BIT_PARAMETER:
                for (int i=0; i<numberOfParameters/8+1; i++)
                eeprom_update_byte((uint8_t*)startAddress+i, defaultValue);
                break;

                case BYTE_PARAMETER:
                for (int i=0; i<numberOfParameters; i++)    {

                    if (defaultValue == AUTO_INCREMENT)
                        eeprom_update_byte((uint8_t*)startAddress+i, i);
                    else eeprom_update_byte((uint8_t*)startAddress+i, defaultValue);

                }
                break;

            }

        }

    }

    eeprom_update_byte((uint8_t*)ID_LOCATION_0, UNIQUE_ID);
    eeprom_update_byte((uint8_t*)ID_LOCATION_1, UNIQUE_ID);
    eeprom_update_byte((uint8_t*)ID_LOCATION_2, UNIQUE_ID);

}

Configuration configuration;
