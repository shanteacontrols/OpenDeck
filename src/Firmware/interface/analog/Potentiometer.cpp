/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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
#include "../../OpenDeck.h"

void Analog::checkPotentiometerValue(uint8_t analogID, uint16_t tempValue)
{
    //calculate difference between current and previous reading
    uint16_t analogDiff = abs(tempValue - lastAnalogueValue[analogID]);

    if (!(analogDiff >= POTENTIOMETER_CC_STEP))
        return;

    uint8_t ccValue = RAW_ADC_2_MIDI(tempValue);
    uint8_t oldCCvalue = RAW_ADC_2_MIDI(lastAnalogueValue[analogID]);

    if (ccValue == oldCCvalue)
        return;

    //invert CC data if potInverted is true
    if (database.read(CONF_BLOCK_ANALOG, analogInvertedSection, analogID))
        ccValue = 127 - ccValue;

    uint8_t lowerCClimit = database.read(CONF_BLOCK_ANALOG, analogCClowerLimitSection, analogID);
    uint8_t upperCClimit = database.read(CONF_BLOCK_ANALOG, analogCCupperLimitSection, analogID);

    //only use map when cc limits are different from defaults
    if ((lowerCClimit != 0) || (upperCClimit != 127))
    {
        midi.sendControlChange(database.read(CONF_BLOCK_ANALOG, analogMIDIidSection, analogID), mapAnalog_uint8(ccValue, 0, 127, lowerCClimit, upperCClimit), database.read(CONF_BLOCK_MIDI, midiChannelSection, CCchannel));
        if (sysEx.configurationEnabled())
        {
            if ((rTimeMs() - getLastCinfoMsgTime(CONF_BLOCK_ANALOG)) > COMPONENT_INFO_TIMEOUT)
            {
                sysEx.startResponse();
                sysEx.addToResponse(COMPONENT_ID_STRING);
                sysEx.addToResponse(CONF_BLOCK_ANALOG);
                sysEx.addToResponse(analogID);
                sysEx.sendResponse();
                updateCinfoTime(CONF_BLOCK_ANALOG);
            }
        }
    }
    else
    {
        midi.sendControlChange(database.read(CONF_BLOCK_ANALOG, analogMIDIidSection, analogID), ccValue, database.read(CONF_BLOCK_MIDI, midiChannelSection, CCchannel));
        if (sysEx.configurationEnabled())
        {
            if ((rTimeMs() - getLastCinfoMsgTime(CONF_BLOCK_ANALOG)) > COMPONENT_INFO_TIMEOUT)
            {
                sysEx.startResponse();
                sysEx.addToResponse(COMPONENT_ID_STRING);
                sysEx.addToResponse(CONF_BLOCK_ANALOG);
                sysEx.addToResponse(analogID);
                sysEx.sendResponse();
                updateCinfoTime(CONF_BLOCK_ANALOG);
            }
        }
    }

    //update values
    lastAnalogueValue[analogID] = tempValue;
}
