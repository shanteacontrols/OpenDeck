/*

Copyright 2015-2018 Igor Petrovic

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

#include "pins/Map.h"
#include "interface/digital/output/leds/Helpers.h"
#include "board/common/constants/LEDs.h"
#include "board/common/digital/input/Variables.h"
#include "board/common/digital/output/Variables.h"
#include "core/src/HAL/avr/PinManipulation.h"
#include "core/src/general/BitManipulation.h"

namespace Board
{
    namespace detail
    {
        ///
        /// Acquires data by reading all inputs on specified digital input pins.
        ///
        inline void storeDigitalIn()
        {
            for (int i=0; i<DIGITAL_IN_ARRAY_SIZE; i++)
            {
                for (int j=0; j<8; j++)
                {
                    uint8_t buttonIndex = i*8 + j;

                    if (buttonIndex >= MAX_NUMBER_OF_BUTTONS)
                        break; //done

                    BIT_WRITE(digitalInBuffer[dIn_head][i], j, !readPin(*dInPins[buttonIndex].port, dInPins[buttonIndex].pin));
                }
            }
        }

        ///
        /// \brief Checks if any LED state has been changed and writes changed state to digital output pin.
        ///
        inline void checkLEDs()
        {
            for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            {
                uint8_t ledStateSingle = LED_ON(ledState[i]);

                if (ledStateSingle != lastLEDstate[i])
                {
                    if (ledStateSingle)
                    {
                        EXT_LED_ON(*dOutPins[i].port, dOutPins[i].pin);
                    }
                    else
                    {
                        EXT_LED_OFF(*dOutPins[i].port, dOutPins[i].pin);
                    }

                    lastLEDstate[i] = ledStateSingle;
                }
            }
        }
    }
}