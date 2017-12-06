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

Board board;
MIDI midi;

int main(void)
{
    board.init();

    midi.setUSBMIDIstate(true);
    midi.setDINMIDIstate(true);
    midi.setOneByteParseDINstate(true);

    setLow(LED_OUT_PORT, LED_OUT_PIN);
    setLow(LED_IN_PORT, LED_IN_PIN);
    wait_ms(200);
    setHigh(LED_OUT_PORT, LED_OUT_PIN);
    setHigh(LED_IN_PORT, LED_IN_PIN);

    sei();

    while (1)
    {
        //route data from uart to usb
        if (midi.read(dinInterface, THRU_FULL_USB))
        {
            MIDIsent = true;
        }

        //route data from usb to uart
        if (midi.read(usbInterface, THRU_FULL_DIN))
        {
            MIDIreceived = true;
        }
    }
}

