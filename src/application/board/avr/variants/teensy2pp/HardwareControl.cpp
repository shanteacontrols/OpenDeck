/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

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

#include "pins/Map.h"
#include "../../../../interface/digital/output/leds/Variables.h"
#include "../../../../interface/digital/output/leds/Helpers.h"
#include "board/common/constants/LEDs.h"
#include "board/common/digital/input/Variables.h"
#include "board/common/digital/output/Variables.h"


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

inline void checkLEDs()
{
    //if there is an active LED in current column, turn on LED row
    //do fancy transitions here
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
