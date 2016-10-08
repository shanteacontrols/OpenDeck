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

#define DIGITAL_VALUE_THRESHOLD 0x3E8

Analog::Analog()
{
    //def const
    analogDebounceCounter = 0;
}

void Analog::update()
{
    if (!Board::analogDataAvailable())
        return;

    addAnalogSamples();
    bool sampled = analogValuesSampled();

    int16_t analogData;

    //check values
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //don't process component if it's not enabled
        if (!database.read(CONF_BLOCK_ANALOG, analogEnabledSection, i))
            continue;

        if (!database.read(CONF_BLOCK_ANALOG, analogDigitalEnabledSection, i))
        {
            //three samples are needed
            if (!sampled)
                continue;

            //get median value from three analog samples for better accuracy
            analogData = getMedianValue(i);
            analogType_t type = (analogType_t)database.read(CONF_BLOCK_ANALOG, analogTypeSection, i);

            switch(type)
            {
                case potentiometer:
                checkPotentiometerValue(i, analogData);
                break;

                case fsr:
                checkFSRvalue(i, analogData);
                break;

                case ldr:
                break;

                default:
                break;
            }
        }
        else
        {
            analogData = Board::getAnalogValue(i);
            bool state = analogData > DIGITAL_VALUE_THRESHOLD;
            buttons.processButton(i+MAX_NUMBER_OF_BUTTONS, state, false);
        }
    }
}

void Analog::addAnalogSamples()
{
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analogSample[i][analogDebounceCounter] = Board::getAnalogValue(i); //get raw analog reading

    analogDebounceCounter++;
}

bool Analog::analogValuesSampled()
{
    if (analogDebounceCounter == NUMBER_OF_SAMPLES)
    {
        analogDebounceCounter = 0;
        return true;
    }

    return false;
}

int16_t Analog::getMedianValue(uint8_t analogID)
{
    int16_t medianValue = 0;

    if ((analogSample[analogID][0] <= analogSample[analogID][1]) && (analogSample[analogID][0] <= analogSample[analogID][2]))
        medianValue = (analogSample[analogID][1] <= analogSample[analogID][2]) ? analogSample[analogID][1] : analogSample[analogID][2];
    else if ((analogSample[analogID][1] <= analogSample[analogID][0]) && (analogSample[analogID][1] <= analogSample[analogID][2]))
        medianValue = (analogSample[analogID][0] <= analogSample[analogID][2]) ? analogSample[analogID][0] : analogSample[analogID][2];
    else
        medianValue = (analogSample[analogID][0] <= analogSample[analogID][1]) ? analogSample[analogID][0] : analogSample[analogID][1];

    return medianValue;
}

void Analog::debounceReset(uint16_t index)
{
    lastAnalogueValue[index] = 0;
    fsrLastAfterTouchValue[index] = 0;

    uint8_t arrayIndex = index/8;
    uint8_t fsrIndex = index - 8*arrayIndex;

    bitWrite(fsrPressed[arrayIndex], fsrIndex, false);
}

Analog analog;
