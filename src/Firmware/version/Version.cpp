#include "Version.h"

bool checkNewRevision()
{
    //current app crc is written to last flash location
    //previous crc is stored into eeprom
    //if two differ, app has changed

    uint16_t crc_eeprom = eeprom_read_word((uint16_t*)SW_CRC_LOCATION_EEPROM);
    uint16_t crc_flash = pgm_read_word_far(SW_CRC_LOCATION_FLASH);

    if (crc_eeprom != crc_flash)
    {
        eeprom_update_word((uint16_t*)SW_CRC_LOCATION_EEPROM, crc_flash);
        return true;
    }

    return false;
}

uint8_t getSWversion(swVersion_t point)
{
    uint32_t address;

    switch(point)
    {
        case swVersion_major:
        case swVersion_minor:
        case swVersion_revision:
        address = SW_VERSION_POINT_LOCATION+(uint32_t)point*(uint32_t)2;
        return (uint8_t)pgm_read_word_far(address);

        default:
        return 0;
    }
}

void getGitHash(char *hash)
{
    uint32_t address = 2 + SW_VERSION_POINT_LOCATION+(uint32_t)swVersion_revision*(uint32_t)2;

    for (int i=0; i<7; i++)
    {
        hash[i] = (uint8_t)pgm_read_word_far(address);
        address++;
    }

    hash[7] = '\0';
}