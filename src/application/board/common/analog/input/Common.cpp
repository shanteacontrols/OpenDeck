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

#include <util/atomic.h>
#include "board/Board.h"
#include "Variables.h"
#include "core/src/general/BitManipulation.h"
#include "core/src/HAL/avr/adc/ADC.h"


uint8_t             analogIndex;
volatile uint8_t    analogSampleCounter;
volatile int16_t    analogBuffer[MAX_NUMBER_OF_ANALOG];

///
/// Due to non-linearity of standard potentiometers on their extremes (low and high values),
/// hysteresis is used to avoid incorrect values. These arrays hold information on whether
/// low or high hysteresis values should be used.
/// @{

static uint8_t      lowHysteresisActive[MAX_NUMBER_OF_ANALOG];
static uint8_t      highHysteresisActive[MAX_NUMBER_OF_ANALOG];

/// @}

bool Board::isHysteresisActive(hysteresisType_t type, uint8_t analogID)
{
    if (type == lowHysteresis)
        return lowHysteresisActive[analogID];
    else
        return highHysteresisActive[analogID];
}

void Board::updateHysteresisState(hysteresisType_t type, uint8_t analogID, bool state)
{
    if (type == lowHysteresis)
        lowHysteresisActive[analogID] = state;
    else
        highHysteresisActive[analogID] = state;
}

int16_t Board::getAnalogValue(uint8_t analogID)
{
    int16_t value;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        value = analogBuffer[analogID] >> ANALOG_SAMPLE_SHIFT;
        analogBuffer[analogID] = 0;
    }

    if (value > HYSTERESIS_THRESHOLD_HIGH)
    {
        updateHysteresisState(highHysteresis, analogID, true);
        updateHysteresisState(lowHysteresis, analogID, false);

        value += HYSTERESIS_ADDITION;

        if (value > ADC_MAX_VALUE)
            return ADC_MAX_VALUE;

        return value;
    }
    else
    {
        if (value < (HYSTERESIS_THRESHOLD_HIGH - HYSTERESIS_ADDITION))
        {
            //value is now either in non-hysteresis area or low hysteresis area

            updateHysteresisState(highHysteresis, analogID, false);

            if (value < (HYSTERESIS_THRESHOLD_LOW + HYSTERESIS_SUBTRACTION))
            {
                if (value < HYSTERESIS_THRESHOLD_LOW)
                {
                    updateHysteresisState(lowHysteresis, analogID, true);
                    value -= HYSTERESIS_SUBTRACTION;

                    if (value < 0)
                        value = 0;

                    return value;
                }
                else
                {
                    if (isHysteresisActive(lowHysteresis, analogID))
                    {
                        value -= HYSTERESIS_SUBTRACTION;

                        if (value < 0)
                            return 0;
                    }

                    return value;
                }
            }

            updateHysteresisState(lowHysteresis, analogID, false);
            updateHysteresisState(highHysteresis, analogID, false);

            return value;
        }
        else
        {
            if (isHysteresisActive(highHysteresis, analogID))
            {
                //high hysteresis still enabled
                value += HYSTERESIS_ADDITION;

                if (value > ADC_MAX_VALUE)
                    return ADC_MAX_VALUE;

                return value;
            }
            else
            {
                updateHysteresisState(highHysteresis, analogID, false);
                return value;
            }
        }
    }
}

bool Board::analogDataAvailable()
{
    return (analogSampleCounter == NUMBER_OF_ANALOG_SAMPLES);
}

void Board::continueADCreadout()
{
    analogSampleCounter = 0;
    analogIndex = 0;
    startADCconversion();
}