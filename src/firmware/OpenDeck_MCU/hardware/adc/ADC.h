#ifndef ADC_H_
#define ADC_H_

//ADC setup and manipulation

#include <avr/io.h>

#define startADCconversion() (ADCSRA |= (1<<ADSC))
#define adcInterruptEnable() (ADCSRA |= (1<<ADIE))

void setUpADC();
void setADCprescaler(uint8_t);
void setADCchannel(uint8_t);
int16_t getADCvalue();
void disconnectDigitalInADC(uint8_t);

#endif