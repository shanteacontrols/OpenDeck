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

#ifdef BOARD_A_LEO

#include "Board.h"
#include "Variables.h"

volatile uint32_t rTime_ms;
uint8_t midiIn_timeout, midiOut_timeout;

uint8_t lastLEDstate[MAX_NUMBER_OF_LEDS];

volatile uint8_t *dInPortArray[] =
{
    &DI_1_PORT,
    &DI_2_PORT,
    &DI_3_PORT,
    &DI_4_PORT,
    &DI_5_PORT,
    &DI_6_PORT,
    &DI_7_PORT,
    &DI_8_PORT
};

const uint8_t dInPinArray[] =
{
    DI_1_PIN,
    DI_2_PIN,
    DI_3_PIN,
    DI_4_PIN,
    DI_5_PIN,
    DI_6_PIN,
    DI_7_PIN,
    DI_8_PIN
};

volatile uint8_t *dOutPortArray[] =
{
    &DO_1_PORT,
    &DO_2_PORT,
    &DO_3_PORT,
    &DO_4_PORT,
    &DO_5_PORT,
    &DO_6_PORT,
    &DO_7_PORT,
    &DO_8_PORT
};

const uint8_t dOutPinArray[] =
{
    DO_1_PIN,
    DO_2_PIN,
    DO_3_PIN,
    DO_4_PIN,
    DO_5_PIN,
    DO_6_PIN,
    DO_7_PIN,
    DO_8_PIN
};

inline void storeDigitalIn(uint8_t bufferIndex)
{
    uint8_t data = 0;

    //clear old data
    inputBuffer[bufferIndex] = 0;

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
    {
        data <<= 1;
        data |= readPin(*(dInPortArray[i]), dInPinArray[i]);
    }

    inputBuffer[bufferIndex] |= data;
}

inline void checkLEDs()
{
    if (blinkEnabled)
    {
        if (!blinkTimerCounter)
        {
            //change blinkBit state and write it into ledState variable if LED is in blink state
            for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            {
                if (BIT_READ(ledState[i], LED_BLINK_ON_BIT))
                {
                    if (blinkState)
                        BIT_SET(ledState[i], LED_BLINK_STATE_BIT);
                    else
                        BIT_CLEAR(ledState[i], LED_BLINK_STATE_BIT);
                }
            }

            blinkState = !blinkState;
        }
    }

    //if there is an active LED in current column, turn on LED row
    //do fancy transitions here
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        uint8_t ledStateSingle = BIT_READ(ledState[i], LED_ACTIVE_BIT) && (BIT_READ(ledState[i], LED_BLINK_ON_BIT) == BIT_READ(ledState[i], LED_BLINK_STATE_BIT));

        if (ledStateSingle != lastLEDstate[i])
        {
            if (ledStateSingle)
            {
                setHigh(*(dOutPortArray[i]), dOutPinArray[i]);
            }
            else
            {
                setLow(*(dOutPortArray[i]), dOutPinArray[i]);
            }

            lastLEDstate[i] = ledStateSingle;
        }
    }
}

ISR(TIMER0_COMPA_vect)
{
    //update buttons and leds every 1ms
    //update run timer every 1ms
    //use this timer to start adc conversion every 250us

    static uint8_t updateStuff = 0;
    updateStuff++;

    //startADCconversion();

    if (updateStuff == 4)
    {
        checkLEDs();
        blinkTimerCounter++;
        rTime_ms++;
        updateStuff = 0;

        if (blinkTimerCounter >= ledBlinkTime)
            blinkTimerCounter = 0;

        if (MIDIevent_in)
        {
            setLow(LED_IN_PORT, LED_IN_PIN);
            MIDIevent_in = false;
            midiIn_timeout = MIDI_INDICATOR_TIMEOUT;
        }

        if (MIDIevent_out)
        {
            setLow(LED_OUT_PORT, LED_OUT_PIN);
            MIDIevent_out = false;
            midiOut_timeout = MIDI_INDICATOR_TIMEOUT;
        }

        if (midiIn_timeout)
            midiIn_timeout--;
        else
            setHigh(LED_IN_PORT, LED_IN_PIN);

        if (midiOut_timeout)
            midiOut_timeout--;
        else
            setHigh(LED_OUT_PORT, LED_OUT_PIN);

        //read input matrix
        uint8_t bufferIndex = digital_buffer_head + 1;

        if (bufferIndex >= DIGITAL_BUFFER_SIZE)
        {
            bufferIndex = 0;
        }

        if (digital_buffer_tail == bufferIndex)
        {
            return; //buffer full, exit
        }

        inputBuffer[bufferIndex] = 0;
        digital_buffer_head = bufferIndex;

        storeDigitalIn(bufferIndex);
    }
}

#endif