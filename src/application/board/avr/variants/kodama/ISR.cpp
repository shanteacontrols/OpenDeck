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
#include "pins/Map.h"
#include "board/common/analog/input/Variables.h"
#include "board/common/digital/input/Variables.h"
#include "board/common/digital/input/direct/Variables.h"
#include "board/common/digital/input/Variables.h"
#include "../../../../interface/digital/output/leds/Variables.h"
#include "../../../../interface/digital/output/leds/Helpers.h"
#include "board/common/indicators/Variables.h"
#include "board/common/constants/LEDs.h"

volatile uint32_t rTime_ms;

volatile bool MIDIreceived, MIDIsent;
static uint8_t lastLEDstate[MAX_NUMBER_OF_LEDS] = { 255 };

inline void storeDigitalIn()
{
    setLow(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
    setLow(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
    _NOP();
    _NOP();

    setHigh(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

    for (int i=0; i<NUMBER_OF_IN_SR; i++)
    {
        for (int j=0; j<8; j++)
        {
            setLow(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
            _NOP();
            BIT_WRITE(digitalInBuffer[i], j, !readPin(SR_IN_DATA_PORT, SR_IN_DATA_PIN));
            setHigh(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
        }
    }

    digitalInBufferCounter = DIGITAL_IN_BUFFER_SIZE;
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
            LED_ON(ledState[i]) ? setLow(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN) : setHigh(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
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
        checkLEDs();

        if (digitalInBufferCounter < DIGITAL_IN_BUFFER_SIZE)
            storeDigitalIn();

        updateStuff = 0;
    }
}

inline void setMuxInput()
{
    BIT_READ(activeMuxInput, 0) ? setHigh(MUX_S0_PORT, MUX_S0_PIN) : setLow(MUX_S0_PORT, MUX_S0_PIN);
    BIT_READ(activeMuxInput, 1) ? setHigh(MUX_S1_PORT, MUX_S1_PIN) : setLow(MUX_S1_PORT, MUX_S1_PIN);
    BIT_READ(activeMuxInput, 2) ? setHigh(MUX_S2_PORT, MUX_S2_PIN) : setLow(MUX_S2_PORT, MUX_S2_PIN);
    BIT_READ(activeMuxInput, 3) ? setHigh(MUX_S3_PORT, MUX_S3_PIN) : setLow(MUX_S3_PORT, MUX_S3_PIN);
}

ISR(ADC_vect)
{
    analogBuffer[analogIndex] += ADC;
    analogIndex++;
    activeMuxInput++;

    bool switchMux = (activeMuxInput == NUMBER_OF_MUX_INPUTS);

    if (switchMux)
    {
        activeMuxInput = 0;
        activeMux++;

        if (activeMux == NUMBER_OF_MUX)
        {
            activeMux = 0;
            analogIndex = 0;
            analogSampleCounter++;
        }

        setADCchannel(muxInPinArray[activeMux]);
    }

    //always set mux input
    setMuxInput();
}
