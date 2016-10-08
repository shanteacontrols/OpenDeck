/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015, 2016 Igor Petrovic

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

#include "Database.h"

void Database::initSettings(bool partialReset)
{
    for (int i=0; i<CONF_BLOCKS; i++)
    {
        for (int j=0; j<blocks[i].sections; j++)
        {
            uint16_t startAddress = getSectionAddress(i, j);
            uint8_t parameterType = getParameterType(i, j);
            uint8_t defaultValue = blocks[i].defaultValue[j];
            uint8_t numberOfParameters = blocks[i].sectionParameters[j];

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
            }
        }
    }
}