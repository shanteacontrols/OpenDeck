#include "EEPROMsettings.h"
#include "..\sysex\SysEx.h"

EEPROMsettings::EEPROMsettings()    {

    //def const

}

void EEPROMsettings::init() {

    //if ID bytes haven't been written to EEPROM on specified address,
    //write default configuration to EEPROM
    if  (!(

    (eeprom_read_byte((uint8_t*)ID_LOCATION_0) == UNIQUE_ID) &&
    (eeprom_read_byte((uint8_t*)ID_LOCATION_1) == UNIQUE_ID) &&
    (eeprom_read_byte((uint8_t*)ID_LOCATION_2) == UNIQUE_ID)

    )) {

        clearEEPROM();
        resetConfiguration();

    }

}

void EEPROMsettings::resetConfiguration()  {

    //write default configuration stored in PROGMEM to EEPROM
    for (int i=0; i<(int16_t)sizeof(defConf); i++)
        eeprom_update_byte((uint8_t*)i, pgm_read_byte(&(defConf[i])));

}

void EEPROMsettings::clearEEPROM()    {

    for (int i=0; i<1024; i++) eeprom_write_byte((uint8_t*)i, 0xFF);

}

bool EEPROMsettings::writeParameter(int16_t startAddress, uint8_t parameterID, uint8_t newParameter, uint8_t parameterType)  {

    uint8_t arrayIndex,
            parameterIndex,
            changedArray;

    switch(parameterType)   {

        case BIT_PARAMETER:
        arrayIndex = parameterID/8;
        parameterIndex = parameterID - 8*arrayIndex;
        startAddress += arrayIndex;
        changedArray = eeprom_read_byte((uint8_t*)startAddress);

        if (newParameter == RESET_VALUE)    bitWrite(changedArray, parameterIndex, bitRead(pgm_read_byte(&(defConf[startAddress])), parameterIndex));
        else                                bitWrite(changedArray, parameterIndex, newParameter);

        eeprom_update_byte((uint8_t*)startAddress, changedArray);

        return (changedArray == eeprom_read_byte((uint8_t*)startAddress));
        break;

        case BYTE_PARAMETER:
        startAddress += parameterID;

        if (newParameter == RESET_VALUE) newParameter = pgm_read_byte(&(defConf[startAddress]));

        eeprom_update_byte((uint8_t*)startAddress, newParameter);

        return (newParameter == eeprom_read_byte((uint8_t*)startAddress));
        break;

    }   return false;

}

EEPROMsettings eepromSettings;