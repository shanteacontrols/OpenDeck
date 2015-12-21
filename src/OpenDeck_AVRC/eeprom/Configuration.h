#include "UniqueID.h"
#include "..\BitManipulation.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "Blocks.h"

#define BIT_PARAMETER_BYTES     8
#define BYTE_PARAMETER_BYTES    64

#define BYTE_PARAMETER          0
#define BIT_PARAMETER           1

#define RESET_VALUE             128
#define AUTO_INCREMENT          255

#ifndef EEPROM_H_
#define EEPROM_H_

typedef struct {    //hardcoded value based on max subtypes (analog has the most)

    uint8_t sections;
    uint16_t blockStartAddress;
    uint16_t subsectionAddress[6];
    uint8_t sectionParameters[6];
    uint8_t subsectionType[6];
    uint8_t defaultValue[6];

} blockDescriptor;

class Configuration   {

    public:
    Configuration();
    void init();
    void clearEEPROM();
    inline uint8_t readParameter(uint8_t blockID, uint8_t sectionID, uint8_t parameterID)  {

        uint16_t startAddress = getSectionAddress(blockID, sectionID);
        uint8_t parameterType = getParameterType(blockID, sectionID);

        uint8_t arrayIndex;
        uint8_t parameterIndex;

        switch(parameterType)   {

            case BIT_PARAMETER:
            arrayIndex = parameterID/8;
            parameterIndex = parameterID - 8*arrayIndex;
            return bitRead(eeprom_read_byte((uint8_t*)startAddress+arrayIndex), parameterIndex);
            break;

            case BYTE_PARAMETER:
            return eeprom_read_byte((uint8_t*)startAddress+parameterID);
            break;

        }   return 0;

    };
    bool writeParameter(uint8_t blockID, uint8_t sectionID, uint8_t parameterID, uint8_t newValue);

    private:
    blockDescriptor blocks[NUMBER_OF_BLOCKS];
    inline uint16_t getSectionAddress(uint8_t blockID, uint8_t sectionID)   {

        return blocks[blockID].blockStartAddress+blocks[blockID].subsectionAddress[sectionID];

    };

    inline uint8_t getParameterType(uint8_t blockID, uint8_t sectionID) {

        return blocks[blockID].subsectionType[sectionID];

    }
    void createMemoryLayout();
    void createSectionAddresses();
    void writeConfiguration();

};

extern Configuration configuration;

#endif