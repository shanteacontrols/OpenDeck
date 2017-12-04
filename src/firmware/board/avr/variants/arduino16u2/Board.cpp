/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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

#include "Board.h"

Board::Board()
{
    
}

void Board::init()
{
    cli();
    //disable watchdog
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    initPins();
    initUART_MIDI();
    initUSB_MIDI();

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        //set timer0 to ctc, used to show midi tx/rx status using on-board leds
        TCCR0A |= (1<<WGM01);           //CTC mode
        TCCR0B |= (1<<CS01)|(1<<CS00);  //prescaler 64
        OCR0A = 249;                     //1ms
        TIMSK0 |= (1<<OCIE0A);          //compare match interrupt
    }
}

void Board::initPins()
{
    //active low logic for these leds
    setOutput(LED_OUT_PORT, LED_OUT_PIN);
    setHigh(LED_OUT_PORT, LED_OUT_PIN);

    setOutput(LED_IN_PORT, LED_IN_PIN);
    setHigh(LED_IN_PORT, LED_IN_PIN);
}