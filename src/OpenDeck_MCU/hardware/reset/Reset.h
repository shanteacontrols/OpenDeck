#ifndef RESET_H_
#define RESET_H_

#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#define ATTR_NO_INIT                        __attribute__((section (".noinit")))
#define ATTR_INIT_SECTION(SectionIndex)     __attribute__ ((used, naked, section (".init" #SectionIndex )))
#define WDFR 3

void Bootloader_Jump_Check(void) ATTR_INIT_SECTION(3);
void disablePeripherals();
void reboot();

#endif