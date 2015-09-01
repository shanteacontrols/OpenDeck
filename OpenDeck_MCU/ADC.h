#include <avr/io.h>

#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sfr_defs.h>
#include "Board.h"

void setUpADC();
void set8bitADC();
void setADCprescaler(uint8_t);
int16_t analogReadBlocking(uint8_t);
void setADCchannel(uint8_t);
int16_t getADCvalue();
void disconnectDigitalInADC(uint8_t);
void enableADCinterrupt();
void startADCconversion();

#endif