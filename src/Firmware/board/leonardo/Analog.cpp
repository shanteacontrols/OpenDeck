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

#ifdef BOARD_A_LEO

#include "Board.h"
#include "Variables.h"

uint8_t             analogBufferCounter;
volatile int16_t    analogBuffer[ANALOG_BUFFER_SIZE];

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
    return true;
}

int16_t Board::getAnalogValue(uint8_t analogID)
{
    int16_t value;

    if (analogBufferCounter == analogID)
        return -1; //conversion still in progress for requested analog id

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        value = analogBuffer[analogID];
        analogBuffer[analogID] = -1;
    }

    return value;
}

ISR(ADC_vect)
{
    analogBuffer[analogBufferCounter] = ADC;
    analogBufferCounter++;

    if (analogBufferCounter == MAX_NUMBER_OF_ANALOG)
    {
        analogBufferCounter = 0;
    }

    //always set mux input
    setADCchannel(aInPinMap[analogBufferCounter]);
}

#endif