/*

Copyright 2015-2019 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "Analog.h"
#include "core/src/general/BitManipulation.h"

void Analog::update()
{
    if (!Board::analogDataAvailable())
        return;

    //check values
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //don't process component if it's not enabled
        if (!database.read(DB_BLOCK_ANALOG, dbSection_analog_enable, i))
            continue;

        int16_t analogData = Board::getAnalogValue(i);
        analogType_t type = static_cast<analogType_t>(database.read(DB_BLOCK_ANALOG, dbSection_analog_type, i));

        if (type != aType_button)
        {
            switch(type)
            {
                case aType_potentiometer_cc:
                case aType_potentiometer_note:
                case aType_NRPN_7:
                case aType_NRPN_14:
                case aType_PitchBend:
                checkPotentiometerValue(type, i, analogData);
                break;

                case aType_fsr:
                checkFSRvalue(i, analogData);
                break;

                default:
                break;
            }
        }
        else
        {
            if (buttonHandler != nullptr)
                (*buttonHandler)(i, analogData);
        }
    }

    Board::continueADCreadout();
}

void Analog::debounceReset(uint16_t index)
{
    lastAnalogueValue[index] = 0;
    fsrPressed[index] = false;
}

///
/// \param fptr [in]    Pointer to function.
///
void Analog::setButtonHandler(void(*fptr)(uint8_t adcIndex, uint16_t adcValue))
{
    buttonHandler = fptr;
}