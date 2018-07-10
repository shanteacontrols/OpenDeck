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
#include "interface/digital/output/leds/Constants.h"
#include "../../../../interface/digital/output/leds/Variables.h"
#include "board/common/constants/LEDs.h"

void kodamaStartUpAnimation()
{
    //turn all leds on first
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        BIT_SET(ledState[i], LED_ACTIVE_BIT);
        BIT_SET(ledState[i], LED_STATE_BIT);
    }

    wait_ms(1000);

    for (int i=0; i<12; i++)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            BIT_CLEAR(ledState[ledMapArray[i]], LED_ACTIVE_BIT);
            BIT_CLEAR(ledState[ledMapArray[i]], LED_STATE_BIT);
        }

        wait_ms(35);
    }

    for (int i=0; i<12; i++)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            BIT_SET(ledState[ledMapArray[11-i]], LED_ACTIVE_BIT);
            BIT_SET(ledState[ledMapArray[11-i]], LED_STATE_BIT);
        }

        wait_ms(35);
    }

    for (int i=0; i<12; i++)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            BIT_CLEAR(ledState[ledMapArray[11-i]], LED_ACTIVE_BIT);
            BIT_CLEAR(ledState[ledMapArray[11-i]], LED_STATE_BIT);
        }

        wait_ms(35);
    }

    //turn all off again
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        BIT_CLEAR(ledState[i], LED_ACTIVE_BIT);
        BIT_CLEAR(ledState[i], LED_STATE_BIT);
    }
}

void Board::initPins()
{
    setInput(SR_IN_DATA_PORT, SR_IN_DATA_PIN);
    setOutput(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
    setOutput(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

    setOutput(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
    setOutput(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
    setOutput(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

    setOutput(MUX_S0_PORT, MUX_S0_PIN);
    setOutput(MUX_S1_PORT, MUX_S1_PIN);
    setOutput(MUX_S2_PORT, MUX_S2_PIN);
    setOutput(MUX_S3_PORT, MUX_S3_PIN);

    setInput(MUX_1_IN_PORT, MUX_1_IN_PIN);
    setInput(MUX_2_IN_PORT, MUX_2_IN_PIN);
    setInput(MUX_3_IN_PORT, MUX_3_IN_PIN);
    setInput(MUX_4_IN_PORT, MUX_4_IN_PIN);

    //unused pins
    setOutput(PORTB, 0);
    setOutput(PORTB, 4);
    setOutput(PORTB, 5);
    setOutput(PORTB, 7);

    setOutput(PORTC, 6);
    setOutput(PORTC, 7);

    setOutput(PORTD, 6);
    setOutput(PORTD, 7);

    setOutput(PORTE, 2);

    setOutput(PORTF, 0);
    setOutput(PORTF, 1);
    setOutput(PORTF, 4);

    //make sure all unused pins are logic low
    setLow(PORTB, 0);
    setLow(PORTB, 4);
    setLow(PORTB, 5);
    setLow(PORTB, 7);

    setLow(PORTC, 6);
    setLow(PORTC, 7);

    setLow(PORTD, 6);
    setLow(PORTD, 7);

    setLow(PORTE, 2);

    setLow(PORTF, 0);
    setLow(PORTF, 1);
    setLow(PORTF, 4);
}

void Board::initAnalog()
{
    disconnectDigitalInADC(muxInPinArray[0]);
    disconnectDigitalInADC(muxInPinArray[1]);
    disconnectDigitalInADC(muxInPinArray[2]);
    disconnectDigitalInADC(muxInPinArray[3]);

    adcConf adcConfiguration;

    adcConfiguration.prescaler = ADC_PRESCALER_128;
    adcConfiguration.vref = ADC_VREF_AREF;

    setUpADC(adcConfiguration);
    setADCchannel(muxInPinArray[0]);

    for (int i=0; i<3; i++)
        getADCvalue();  //few dummy reads to init ADC

    adcInterruptEnable();
}

void Board::configureTimers()
{
    //clear timer0 conf
    TCCR0A = 0;
    TCCR0B = 0;
    TIMSK0 = 0;

    //clear timer1 conf
    TCCR1A = 0;
    TCCR1B = 0;

    //clear timer3 conf
    TCCR3A = 0;
    TCCR3B = 0;

    //clear timer4 conf
    TCCR4A = 0;
    TCCR4B = 0;
    TCCR4C = 0;
    TCCR4D = 0;
    TCCR4E = 0;

    //set timer0 to ctc, used for millis/led matrix
    TCCR0A |= (1<<WGM01);           //CTC mode
    TCCR0B |= (1<<CS01)|(1<<CS00);  //prescaler 64
    OCR0A = 62;                     //250us
    TIMSK0 |= (1<<OCIE0A);          //compare match interrupt
}

void Board::ledFlashStartup(bool fwUpdated)
{
    
}

void Board::initCustom()
{
    //init all outputs on shift register
    setLow(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        EXT_LED_OFF(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
        pulseHighToLow(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
    }

    setHigh(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

    startUpAnimation = kodamaStartUpAnimation;
}

Board board;