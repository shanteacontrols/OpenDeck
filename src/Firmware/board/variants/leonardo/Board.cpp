#ifdef BOARD_A_LEO

#include "Board.h"

Board::Board()
{
    //default constructor
}

void Board::reboot(rebootType_t type)
{
    switch(type)
    {
        case rebootApp:
        eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, APP_REBOOT_VALUE);
        break;

        case rebootBtldr:
        eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, BTLDR_REBOOT_VALUE);
        break;
    }

    mcuReset();
}

Board board;

#endif