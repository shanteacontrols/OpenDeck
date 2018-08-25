/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

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

#include "board/Board.h"
#include "Pins.h"
#include "board/common/constants/LEDs.h"
#include "Variables.h"

USBMIDIpacket_t usbMIDIpacket;


void Board::initPins()
{
    //bootloader/midi leds
    setOutput(LED_IN_PORT, LED_IN_PIN);
    setOutput(LED_OUT_PORT, LED_OUT_PIN);

    INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
    INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
}

void Board::configureTimers()
{
    //set timer0 to ctc, used to show midi tx/rx status using on-board leds
    TCCR0A |= (1<<WGM01);           //CTC mode
    TCCR0B |= (1<<CS01)|(1<<CS00);  //prescaler 64
    OCR0A = 249;                    //1ms
    TIMSK0 |= (1<<OCIE0A);          //compare match interrupt
}

void Board::ledFlashStartup(bool fwUpdated)
{
    //block interrupts here to avoid received midi traffic messing with indicator leds animation
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        for (int i=0; i<3; i++)
        {
            if (fwUpdated)
            {
                INT_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
                INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
                _delay_ms(200);
                INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
                INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
                _delay_ms(200);
            }
            else
            {
                INT_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
                INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
                _delay_ms(200);
                INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
                INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
                _delay_ms(200);
            }
        }
    }
}

Board board;