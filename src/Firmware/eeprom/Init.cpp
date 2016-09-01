#ifndef INIT_H_
#define INIT_H_

#include "Configuration.h"

void Configuration::initSettings(bool partialReset) {

    for (int i=0; i<CONF_BLOCKS; i++)  {

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
                while (numberOfParameters--)    {

                    if (defaultValue == AUTO_INCREMENT)
                        eeprom_update_byte((uint8_t*)startAddress+numberOfParameters, numberOfParameters);
                    else eeprom_update_byte((uint8_t*)startAddress+numberOfParameters, defaultValue);

                }
                break;

            }

        }

    }

}

#endif