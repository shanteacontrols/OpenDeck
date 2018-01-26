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

#include "Board.h"
#include "Variables.h"

static uint8_t      analogBufferCounter;
volatile uint8_t    analogSampleCounter;
volatile int16_t    analogBuffer[ANALOG_BUFFER_SIZE];

static bool isHysteresisActive(hysteresisType_t type, uint8_t analogID)
{
    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;

    if (type == lowHysteresis)
        return BIT_READ(lowHysteresisActive[arrayIndex], analogIndex);
    else
        return BIT_READ(highHysteresisActive[arrayIndex], analogIndex);
}

static void updateHysteresisState(hysteresisType_t type, uint8_t analogID, bool state)
{
    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;

    if (type == lowHysteresis)
        BIT_WRITE(lowHysteresisActive[arrayIndex], analogIndex, state);
    else
        BIT_WRITE(highHysteresisActive[arrayIndex], analogIndex, state);
}

static uint16_t getHysteresisValue(uint8_t analogID, int16_t value)
{
    if (value > HYSTERESIS_THRESHOLD_HIGH)
    {
        updateHysteresisState(highHysteresis, analogID, true);
        updateHysteresisState(lowHysteresis, analogID, false);

        value += HYSTERESIS_ADDITION;

        if (value > 1023)
            return 1023;

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

                if (value > 1023)
                    return 1023;

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

void Board::initAnalog()
{
    adcConf adcConfiguration;

    adcConfiguration.prescaler = ADC_PRESCALER_128;
    adcConfiguration.vref = ADC_VREF_AVCC;

    setUpADC(adcConfiguration);
    setADCchannel(aInPinMap[0]);

    _delay_ms(2);

    for (int i=0; i<5; i++)
        getADCvalue();  //few dummy reads to init ADC

    adcInterruptEnable();
}

bool Board::analogDataAvailable()
{
    return (analogSampleCounter == NUMBER_OF_ANALOG_SAMPLES);
}

int16_t Board::getAnalogValue(uint8_t analogID)
{
    int16_t value;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        value = analogBuffer[analogID] >> ANALOG_SAMPLE_SHIFT;
        analogBuffer[analogID] = 0;
    }

    return getHysteresisValue(analogID, value);
}

void Board::continueADCreadout()
{
    analogSampleCounter = 0;
    analogBufferCounter = 0;
}

uint16_t Board::scaleADC(uint16_t value, uint16_t maxValue)
{
    if (maxValue == 1023)
    {
        return value;
    }
    else if (maxValue == 127)
    {
        return value >> 3;
    }
    else
    {
        //use mapRange_uint32 to avoif overflow issues
        return mapRange_uint32(value, 0, 1023, 0, maxValue);
    }
}

ISR(ADC_vect)
{
    analogBuffer[analogBufferCounter] += ADC;
    analogBufferCounter++;

    if (analogBufferCounter == MAX_NUMBER_OF_ANALOG)
    {
        analogBufferCounter = 0;
        analogSampleCounter++;
    }

    //always set mux input
    setADCchannel(aInPinMap[analogBufferCounter]);
}
