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

#include "Analog.h"

//potentiometer must exceed this value before sending new value
#define POTENTIOMETER_CC_STEP 8

void Analog::checkPotentiometerValue(uint8_t analogID, int16_t tempValue)
{
    //calculate difference between current and previous reading
    int16_t analogDiff = tempValue - lastAnalogueValue[analogID];

    //get absolute difference
    if (analogDiff < 0)
        analogDiff *= -1;

    if (!(analogDiff >= POTENTIOMETER_CC_STEP))
        return;

    uint8_t ccValue = tempValue >> 3;

    //invert CC data if potInverted is true
    if (database.readParameter(CONF_BLOCK_ANALOG, analogInvertedSection, analogID))
        ccValue = 127 - ccValue;

    uint8_t lowerCClimit = database.readParameter(CONF_BLOCK_ANALOG, analogCClowerLimitSection, analogID);
    uint8_t upperCClimit = database.readParameter(CONF_BLOCK_ANALOG, analogCCupperLimitSection, analogID);

    //only use map when cc limits are different from defaults
    if ((lowerCClimit != 0) || (upperCClimit != 127))
    {
        midi.sendNoteOn(database.readParameter(CONF_BLOCK_ANALOG, analogMIDIidSection, analogID), mapAnalog_uint8(ccValue, 0, 127, lowerCClimit, upperCClimit));
        //if (sysEx.configurationEnabled())
            //sysEx.sendComponentID(CONF_BLOCK_ANALOG, analogID);
    }
    else
    {
        midi.sendNoteOn(database.readParameter(CONF_BLOCK_ANALOG, analogMIDIidSection, analogID), ccValue);
        //if (sysEx.configurationEnabled())
            //sysEx.sendComponentID(CONF_BLOCK_ANALOG, analogID);
    }

    //update values
    lastAnalogueValue[analogID] = tempValue;
}
