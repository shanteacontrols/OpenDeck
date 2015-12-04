#include "EEPROMsettings.h"

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

    eeprom_write_byte((uint8_t*)ID_LOCATION_0, UNIQUE_ID);
    eeprom_write_byte((uint8_t*)ID_LOCATION_1, UNIQUE_ID);
    eeprom_write_byte((uint8_t*)ID_LOCATION_2, UNIQUE_ID);

}

EEPROMsettings eepromSettings;