#include <avr/io.h>
#include <avr/interrupt.h>
#include "ADC.h"
#include <util/delay.h>
#include <avr/cpufunc.h>

bool adc8bitEnabled()    {

    return (ADMUX >> ADLAR) & 0x01;

}

void setUpADC() {

    //clear bits in registers
    ADMUX = 0x00;
    ADCSRA = 0x0;

    //set analogue reference voltage to 5V
    //ADMUX |= (1<<REFS0);

    //set prescaler to 128 and enable ADC
    ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
    ADCSRB |= (1<<ADHSM);

}

void set8bitADC()   {

    ADMUX |= (1<<ADLAR);

}

void setADCprescaler(uint8_t prescaler) {

    //disable ADC before setting new prescaler
    ADCSRA &= (0<<ADEN);

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

int16_t analogReadOwnduino(uint8_t adcChannel)  {

    //check for valid channel
    if ((adcChannel < 0) || (adcChannel > 7))   return -1;

    //select ADC channel with safety mask
    ADMUX = (ADMUX & 0xF0) | (adcChannel & 0x0F);

    //single conversion mode
    ADCSRA |= (1<<ADSC);

    //wait until ADC conversion is complete
    while (ADCSRA & (1<<ADSC));

    if (adc8bitEnabled())    return ADCH;
    return ADC;

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

    if (adc8bitEnabled())    return ADCH;
    return ADC;

}

void disconnectDigitalInADC(uint8_t adcChannel) {

    if (adcChannel < 6)
        DIDR0 |= (1<<adcChannel);

}

void enableADCinterrupt()   {

    ADCSRA |= (1<<ADIE);

}

void startADCconversion()   {

    ADCSRA |= (1<<ADSC);
    ADCSRB |= (1<<ADHSM);

}