#include "Board.h"
#include <util/delay.h>
#include <avr/wdt.h>

//original code here:
//http://www.fourwalledcubicle.com/files/LUFA/Doc/120219/html/_page__software_bootloader_start.html

//lufa bootloader has a bit different setup from teensy bootloader
//disable_peripherals copied from core teensy libs since it can't be included

#define ATTR_NO_INIT                        __attribute__((section (".noinit")))
#define ATTR_INIT_SECTION(SectionIndex)     __attribute__ ((used, naked, section (".init" #SectionIndex )))
#define WDFR 3

uint32_t Boot_Key ATTR_NO_INIT;

#define MAGIC_BOOT_KEY            0xDC42ACCA
#define BOOTLOADER_START_ADDRESS  0x7000

void Bootloader_Jump_Check(void) ATTR_INIT_SECTION(3);

void Bootloader_Jump_Check(void)    {

    // If the reset source was the bootloader and the key is correct, clear it and jump to the bootloader
    if ((MCUSR & (1 << WDRF)) && (Boot_Key == MAGIC_BOOT_KEY))  {
        Boot_Key = 0;
        ((void (*)(void))BOOTLOADER_START_ADDRESS)();

    }
}

#ifdef BOARD_TANNIN
    void disable_peripherals(void)   {

        //disable eeprom
        EECR = 0;

        //disable analog comparator
        ACSR = 0;

        //disable SPI
        SPCR = 0;

        //disable external interrupts
        EIMSK = 0;

        //disable pin change interrupts
        PCICR = 0;

        //disable ADC
        ADCSRA = 0;

        //disable timers
        TIMSK0 = 0;
        TIMSK1 = 0;
        TIMSK3 = 0;
        TIMSK4 = 0;

        //disable USART
        UCSR1B = 0;

        //disable I2C
        TWCR = 0;

        //set all pins to inputs
        DDRB = 0;
        DDRC = 0;
        DDRD = 0;
        DDRE = 0;
        DDRF = 0;

        //write low to all pins
        PORTB = 0;
        PORTC = 0;
        PORTD = 0;
        PORTE = 0;
        PORTF = 0;

    }

    void Board::resetBoard()    {

        cli();
        // stop watchdog timer, if running
        MCUSR &= ~(1<<WDFR);
        WDTCSR |= (1<<WDCE);
        WDTCSR = 0;
        _delay_ms(5);
        UDCON = 1;
        USBCON = (1<<FRZCLK);
        _delay_ms(2000);
        disable_peripherals();

        // Set the bootloader key to the magic value and force a reset
        Boot_Key = MAGIC_BOOT_KEY;
        wdt_enable(WDTO_250MS);
        for (;;);

    }
#endif