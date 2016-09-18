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

#include "ADC.h"
#include "Config.h"
#include <avr/cpufunc.h>

void setUpADC() {

    ADMUX = 0x00;
    ADCSRA = 0x0;

    //setup adc prescaler
    #if ADC_PRESCALER == 128
        ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
    #elif ADC_PRESCALER == 64
        ADCSRA |= (1<<ADPS2)|(1<<ADPS1);
    #elif ADC_PRESCALER == 32
        ADCSRA |= (1<<ADPS2)|(1<<ADPS0);
    #elif ADC_PRESCALER == 16
        ADCSRA |= (1<<ADPS2);
    #else
        #error Incorrect ADC prescaler setting. Valid values are 128, 64, 32 and 16.
    #endif

    //setup voltage reference
    #ifdef VREF_AREF
        //nothing, this is default setting
    #elif defined(VREF_AVCC)
        ADMUX |= (1<<REFS0);
    #elif defined(VREF_INTERNAL_2V56)
        ADMUX |= (1<<REFS0) | (1<<REFS1);
    #elif defined(VREF_INTERNAL_1V1)
        ADMUX |= (1<<REFS1);
    #else
        #error "No ADC reference selected or setting is invalid. Valid options are VREF_AREF, VREF_AVCC, VREF_INTERNAL_2V56 and VREF_INTERNAL_1V1"
    #endif

    //enable ADC
    ADCSRA |= (1<<ADEN);

}

void setADCchannel(uint8_t adcChannel)  {

    //check for valid channel
    if ((adcChannel < 0) || (adcChannel > 7))   return;

    //select ADC channel with safety mask
    ADMUX = (ADMUX & 0xF0) | (adcChannel & 0x0F);

    _NOP();

}

int16_t getADCvalue()   {

    //single conversion mode
    ADCSRA |= (1<<ADSC);

    //wait until ADC conversion is complete
    while (ADCSRA & (1<<ADSC));

    return ADC;

}

void disconnectDigitalInADC(uint8_t adcChannel) {

    if (adcChannel < 6)
        DIDR0 |= (1<<adcChannel);

}