#include "Firmware.h"
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#define VERSION_POINT_LOCATION  (FLASH_SIZE - 8)
#define CRC_LOCATION_FLASH      (FLASH_SIZE - 2)
#define CRC_LOCATION_EEPROM     (EEPROM_SIZE - 2)   //write crc to last eeprom location

bool checkNewRevision() {

    //current app crc is written to last flash location
    //previous crc is stored into eeprom
    //if two differ, app has changed

    uint16_t crc_eeprom = eeprom_read_word((uint16_t*)CRC_LOCATION_EEPROM);
    uint16_t crc_flash = pgm_read_word_far(CRC_LOCATION_FLASH);

    if (crc_eeprom != crc_flash)   {

        eeprom_update_word((uint16_t*)CRC_LOCATION_EEPROM, crc_flash);
        return true;

    }   return false;

    return false;

}

int16_t getSWversion(swVersion_t point) {

    switch(point)   {

        case swVersion_major:
        case swVersion_minor:
        case swVersion_revision:
        case swVersion_development:
        return pgm_read_word_far(VERSION_POINT_LOCATION+(uint8_t)point*2);

        default:
        return -1;

    }

}