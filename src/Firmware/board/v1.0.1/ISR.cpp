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
#include "Variables.h"

volatile uint32_t rTime_ms;
uint8_t midiIn_timeout, midiOut_timeout;

const uint8_t ledRowPinArray[] =
{
    LED_ROW_1_PIN,
    LED_ROW_2_PIN,
    LED_ROW_3_PIN,
    LED_ROW_4_PIN,
    LED_ROW_5_PIN,
    LED_ROW_6_PIN
};

volatile uint8_t *ledRowPortArray[] =
{
    &LED_ROW_1_PORT,
    &LED_ROW_2_PORT,
    &LED_ROW_3_PORT,
    &LED_ROW_4_PORT,
    &LED_ROW_5_PORT,
    &LED_ROW_6_PORT
};

inline void activateInputColumn(uint8_t column)
{
    bitRead(dmColumnArray[column], 0) ? setHigh(DEC_DM_A0_PORT, DEC_DM_A0_PIN) : setLow(DEC_DM_A0_PORT, DEC_DM_A0_PIN);
    bitRead(dmColumnArray[column], 1) ? setHigh(DEC_DM_A1_PORT, DEC_DM_A1_PIN) : setLow(DEC_DM_A1_PORT, DEC_DM_A1_PIN);
    bitRead(dmColumnArray[column], 2) ? setHigh(DEC_DM_A2_PORT, DEC_DM_A2_PIN) : setLow(DEC_DM_A2_PORT, DEC_DM_A2_PIN);

    _NOP();
}

inline void activateOutputColumn(uint8_t column)
{
    bitRead(column, 0) ? setHigh(DEC_LM_A0_PORT, DEC_LM_A0_PIN) : setLow(DEC_LM_A0_PORT, DEC_LM_A0_PIN);
    bitRead(column, 1) ? setHigh(DEC_LM_A1_PORT, DEC_LM_A1_PIN) : setLow(DEC_LM_A1_PORT, DEC_LM_A1_PIN);
    bitRead(column, 2) ? setHigh(DEC_LM_A2_PORT, DEC_LM_A2_PIN) : setLow(DEC_LM_A2_PORT, DEC_LM_A2_PIN);

    _NOP();
}

inline void storeDigitalIn(uint8_t column, uint8_t bufferIndex)
{
    uint8_t data = 0;
    uint8_t dataReorder = 0;

    //make room for new data
    inputBuffer[bufferIndex] <<= 8;

    //pulse latch pin
    pulseLowToHigh(SR_LATCH_PORT, SR_LATCH_PIN);

    for (int i=0; i<8; i++)
    {
        data <<= 1;
        data |= readPin(SR_DIN_PORT, SR_DIN_PIN);
        //pulse clock pin
        pulseHightToLow(SR_CLK_PORT, SR_CLK_PIN);
    }

    //reorder data to match rows on PCB layout
    for (int i=0; i<8; i++)
        bitWrite(dataReorder, i, bitRead(data, dmRowBitArray[i]));

    inputBuffer[bufferIndex] |= (uint64_t)dataReorder;
}

inline void ledRowsOff()
{
    //turn off PWM
    TCCR1A &= ~(1<<COM1C1);
    TCCR4C &= ~(1<<COM4D1);
    TCCR1A &= ~(1<<COM1A1);
    TCCR4A &= ~(1<<COM4A1);
    TCCR3A &= ~(1<<COM3A1);
    TCCR1A &= ~(1<<COM1B1);

    for (int i=0; i<NUMBER_OF_LED_ROWS; i++)
    {
        #ifdef LED_INVERT
        setHigh(*(ledRowPortArray[i]), ledRowPinArray[i]);
        #else
        setLow(*(ledRowPortArray[i]), ledRowPinArray[i]);
        #endif
    }
}

inline void ledRowOn(uint8_t rowNumber, uint8_t intensity)
{
    if (intensity == 255)
    {
        //max value, don't use pwm
        #ifdef LED_INVERT
        setLow(*(ledRowPortArray[rowNumber]), ledRowPinArray[rowNumber]);
        #else
        setHigh(*(ledRowPortArray[rowNumber]), ledRowPinArray[rowNumber]);
        #endif
        return;
    }

    #ifdef LED_INVERT
    intensity = 255 - intensity;
    #endif

    switch (rowNumber)
    {
        //turn off pwm if intensity is max
        case 0:
        OCR1C = intensity;
        TCCR1A |= (1<<COM1C1);
        break;

        case 1:
        OCR4D = intensity;
        TCCR4C |= (1<<COM4D1);
        break;

        case 2:
        OCR1A = intensity;
        TCCR1A |= (1<<COM1A1);
        break;

        case 3:
        OCR4A = intensity;
        TCCR4A |= (1<<COM4A1);
        break;

        case 4:
        OCR3A = intensity;
        TCCR3A |= (1<<COM3A1);
        break;

        case 5:
        OCR1B = intensity;
        TCCR1A |= (1<<COM1B1);
        break;

        default:
        break;
    }
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
                if (bitRead(ledState[i], LED_BLINK_ON_BIT))
                {
                    if (blinkState)
                        bitSet(ledState[i], LED_BLINK_STATE_BIT);
                    else
                        bitClear(ledState[i], LED_BLINK_STATE_BIT);
                }
            }

            blinkState = !blinkState;
        }
    }

    //if there is an active LED in current column, turn on LED row
    //do fancy transitions here
    for (int i=0; i<NUMBER_OF_LED_ROWS; i++)
    {
        uint8_t ledNumber = activeLEDcolumn+i*NUMBER_OF_LED_COLUMNS;
        uint8_t ledStateSingle = bitRead(ledState[ledNumber], LED_ACTIVE_BIT) && (bitRead(ledState[ledNumber], LED_BLINK_ON_BIT) == bitRead(ledState[ledNumber], LED_BLINK_STATE_BIT));

        ledStateSingle *= (NUMBER_OF_LED_TRANSITIONS-1);

        //don't bother with pwm if it's disabled
        if (!pwmSteps && ledStateSingle)
        {
            ledRowOn(i, 255);
        }
        else
        {
            if (ledTransitionScale[transitionCounter[ledNumber]])
                ledRowOn(i, ledTransitionScale[transitionCounter[ledNumber]]);

            if (transitionCounter[ledNumber] != ledStateSingle)
            {
                //fade up
                if (transitionCounter[ledNumber] < ledStateSingle)
                {
                    transitionCounter[ledNumber] += pwmSteps;

                    if (transitionCounter[ledNumber] > ledStateSingle)
                        transitionCounter[ledNumber] = ledStateSingle;
                }
                else
                {
                    //fade down
                    transitionCounter[ledNumber] -= pwmSteps;

                    if (transitionCounter[ledNumber] < 0)
                        transitionCounter[ledNumber] = 0;
                }
            }
        }
    }
}

ISR(TIMER0_COMPA_vect)
{
    //update blink every 1ms
    //update led matrix every 1ms
    //update button matrix each time

    static bool updateStuff = false;

    if (updateStuff)
    {
        ledRowsOff();

        if (activeLEDcolumn == NUMBER_OF_LED_COLUMNS)
            activeLEDcolumn = 0;

        activateOutputColumn(activeLEDcolumn);
        checkLEDs();

        activeLEDcolumn++;
        blinkTimerCounter++;
        rTime_ms++;

        if (blinkTimerCounter >= ledBlinkTime)
            blinkTimerCounter = 0;

        if (MIDIevent_in)
        {
            setHigh(LED_IN_PORT, LED_IN_PIN);
            MIDIevent_in = false;
            midiIn_timeout = MIDI_INDICATOR_TIMEOUT;
        }

        if (MIDIevent_out)
        {
            setHigh(LED_OUT_PORT, LED_OUT_PIN);
            MIDIevent_out = false;
            midiOut_timeout = MIDI_INDICATOR_TIMEOUT;
        }

        if (midiIn_timeout)
            midiIn_timeout--;
        else
            setLow(LED_IN_PORT, LED_IN_PIN);

        if (midiOut_timeout)
            midiOut_timeout--;
        else
            setLow(LED_OUT_PORT, LED_OUT_PIN);
    }

    updateStuff = !updateStuff;

    //read input matrix
    uint8_t bufferIndex = digital_buffer_head + 1;

    if (bufferIndex >= DIGITAL_BUFFER_SIZE)
        bufferIndex = 0;

    if (digital_buffer_tail == bufferIndex)
        return; //buffer full, exit

    inputBuffer[bufferIndex] = 0;
    digital_buffer_head = bufferIndex;

    for (int i=0; i<NUMBER_OF_BUTTON_COLUMNS; i++)
    {
        activateInputColumn(i);
        storeDigitalIn(i, bufferIndex);
    }
}
