#ifndef RESET_H_
#define RESET_H_

#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#define WDFR 3

void disablePeripherals();
void reboot();

#endif