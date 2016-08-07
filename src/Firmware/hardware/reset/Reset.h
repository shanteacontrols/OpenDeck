#ifndef RESET_H_
#define RESET_H_

#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#define ATTR_NO_INIT_RESET                  __attribute__((section (".noinit")))
#define ATTR_INIT_SECTION(SectionIndex)     __attribute__ ((used, naked, section (".init" #SectionIndex )))
#define WDFR 3

#define APP_REBOOT      0
#define BTLDR_REBOOT    1

void Bootloader_Jump_Check(void) ATTR_INIT_SECTION(3);
void disablePeripherals();
void reboot(uint8_t mode);

#endif