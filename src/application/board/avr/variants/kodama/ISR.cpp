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
#include "Variables.h"
#include "../../../../interface/digital/output/leds/Variables.h"
#include "../../../../interface/digital/output/leds/Helpers.h"

volatile uint32_t rTime_ms;

volatile bool MIDIreceived, MIDIsent;
static uint8_t lastLEDstate[MAX_NUMBER_OF_LEDS];

inline void storeDigitalIn()
{
    setLow(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
    setLow(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
    _NOP();
    _NOP();

    setHigh(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
    {
        setLow(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
        _NOP();
        BIT_WRITE(digitalInBuffer[i], i, !readPin(SR_IN_DATA_PORT, SR_IN_DATA_PIN));
        setHigh(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
    }
}

inline void checkLEDs()
{
    bool updateSR = false;

    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        uint8_t ledStateSingle = LED_ON(ledState[i]);

        if (ledStateSingle != lastLEDstate[i])
        {
            lastLEDstate[i] = ledStateSingle;
            updateSR = true;
        }
    }

    if (updateSR)
    {
        setLow(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        {
            LED_ON(ledState[i]) ? setHigh(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN) : setLow(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
            pulseHighToLow(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
        }

        setHigh(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);
    }
}

ISR(TIMER0_COMPA_vect)
{
    static uint8_t updateStuff = 0;
    updateStuff++;

    if (analogSampleCounter != NUMBER_OF_ANALOG_SAMPLES)
        startADCconversion();

    if (updateStuff == 4)
    {
        rTime_ms++;

        if (digitalInBufferCounter < DIGITAL_BUFFER_SIZE)
            storeDigitalIn();

        updateStuff = 0;
    }
}
