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

#include "Reset.h"

void disablePeripherals(void)
{
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

    //write low to all pins
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    PORTE = 0;
    PORTF = 0;

    //set all pins to inputs
    DDRB = 0;
    DDRC = 0;
    DDRD = 0;
    DDRE = 0;
    DDRF = 0;
}

void reboot()
{
    cli();
    //stop watchdog timer, if running
    MCUSR &= ~(1<<WDFR);
    WDTCSR |= (1<<WDCE);
    WDTCSR = 0;
    _delay_ms(5);
    UDCON = 1;
    USBCON = (1<<FRZCLK);
    _delay_ms(2000);
    disablePeripherals();

    wdt_enable(WDTO_250MS);
    for (;;);
}
