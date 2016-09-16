#include "ADC.h"
#include <avr/cpufunc.h>
#include "Config.h"

void setUpADC() {

    ADMUX = 0x00;
    ADCSRA = 0x0;

    //default ADC voltage is set to AREF

    //set prescaler to 128 and enable ADC
    ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);

}

void setADCprescaler(uint8_t prescaler) {

    //disable ADC before setting new prescaler
    ADCSRA &= ~(1<<ADEN);

    switch(prescaler)   {

        case 16:
        ADCSRA |= (1<<ADPS2);
        break;

        case 32:
        ADCSRA |= (1<<ADPS2)|(1<<ADPS0);
        break;

        case 64:
        ADCSRA |= (1<<ADPS2)|(1<<ADPS1);
        break;

        case 128:
        ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
        break;

        default:
        return;
        break;

    }   ADCSRA |= (1<<ADEN);

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