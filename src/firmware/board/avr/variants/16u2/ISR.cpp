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
#include "Config.h"

volatile uint32_t rTime_ms;
static uint8_t midiIn_timeout, midiOut_timeout;

bool MIDIreceived, MIDIsent;

ISR(TIMER0_COMPA_vect)
{
    if (MIDIreceived)
    {
        setLow(LED_IN_PORT, LED_IN_PIN);
        MIDIreceived = false;
        midiIn_timeout = MIDI_INDICATOR_TIMEOUT;
    }

    if (MIDIsent)
    {
        setLow(LED_OUT_PORT, LED_OUT_PIN);
        MIDIsent = false;
        midiOut_timeout = MIDI_INDICATOR_TIMEOUT;
    }

    if (midiIn_timeout)
    {
        midiIn_timeout--;
    }
    else
    {
        setHigh(LED_IN_PORT, LED_IN_PIN);
    }

    if (midiOut_timeout)
    {
        midiOut_timeout--;
    }
    else
    {
        setHigh(LED_OUT_PORT, LED_OUT_PIN);
    }

    rTime_ms++;
}