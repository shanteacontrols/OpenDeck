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

void Board::init()
{
    cli();
    //disable watchdog
    MCUSR &= ~(1 << WDRF);
    wdt_disable();
    initPins();
    initAnalog();
    initEncoders();

    _delay_ms(5);

    configureTimers();
}

void Board::initPins()
{
    //configure input matrix
    //shift register
    setInput(SR_DIN_PORT, SR_DIN_PIN);
    setOutput(SR_CLK_PORT, SR_CLK_PIN);
    setOutput(SR_LATCH_PORT, SR_LATCH_PIN);

    //decoder
    setOutput(DEC_DM_A0_PORT, DEC_DM_A0_PIN);
    setOutput(DEC_DM_A1_PORT, DEC_DM_A1_PIN);
    setOutput(DEC_DM_A1_PORT, DEC_DM_A2_PIN);

    //configure led matrix
    //rows

    setOutput(LED_ROW_1_PORT, LED_ROW_1_PIN);
    setOutput(LED_ROW_2_PORT, LED_ROW_2_PIN);
    setOutput(LED_ROW_3_PORT, LED_ROW_3_PIN);
    setOutput(LED_ROW_4_PORT, LED_ROW_4_PIN);
    setOutput(LED_ROW_5_PORT, LED_ROW_5_PIN);
    setOutput(LED_ROW_6_PORT, LED_ROW_6_PIN);

    //make sure to turn all rows off
    #ifdef LED_INVERT
    setHigh(LED_ROW_1_PORT, LED_ROW_1_PIN);
    setHigh(LED_ROW_2_PORT, LED_ROW_2_PIN);
    setHigh(LED_ROW_3_PORT, LED_ROW_3_PIN);
    setHigh(LED_ROW_4_PORT, LED_ROW_4_PIN);
    setHigh(LED_ROW_5_PORT, LED_ROW_5_PIN);
    setHigh(LED_ROW_6_PORT, LED_ROW_6_PIN);
    #else
    setLow(LED_ROW_1_PORT, LED_ROW_1_PIN);
    setLow(LED_ROW_2_PORT, LED_ROW_2_PIN);
    setLow(LED_ROW_3_PORT, LED_ROW_3_PIN);
    setLow(LED_ROW_4_PORT, LED_ROW_4_PIN);
    setLow(LED_ROW_5_PORT, LED_ROW_5_PIN);
    setLow(LED_ROW_6_PORT, LED_ROW_6_PIN);
    #endif

    //decoder
    setOutput(DEC_LM_A0_PORT, DEC_LM_A0_PIN);
    setOutput(DEC_LM_A1_PORT, DEC_LM_A1_PIN);
    setOutput(DEC_LM_A2_PORT, DEC_LM_A2_PIN);

    //configure analog
    //select pins
    setOutput(MUX_S0_PORT, MUX_S0_PIN);
    setOutput(MUX_S1_PORT, MUX_S1_PIN);
    setOutput(MUX_S2_PORT, MUX_S2_PIN);
    setOutput(MUX_S3_PORT, MUX_S3_PIN);

    //mux inputs
    setInput(MUX_1_IN_PORT, MUX_1_IN_PIN);
    setInput(MUX_2_IN_PORT, MUX_2_IN_PIN);

    //bootloader led
    setOutput(LED_OUT_PORT, LED_OUT_PIN);
    setOutput(LED_IN_PORT, LED_IN_PIN);

    setLow(LED_OUT_PORT, LED_OUT_PIN);
    setLow(LED_IN_PORT, LED_IN_PIN);
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

    //set timer1, timer3 and timer4 to phase correct pwm mode
    //timer 1
    TCCR1A |= (1<<WGM10);           //phase correct PWM
    TCCR1B |= (1<<CS10);            //prescaler 1
    //timer 3
    TCCR3A |= (1<<WGM30);           //phase correct PWM
    TCCR3B |= (1<<CS30);            //prescaler 1
    //timer 4
    TCCR4A |= (1<<PWM4A);           //Pulse Width Modulator A Enable
    TCCR4B |= (1<<CS40);            //prescaler 1
    TCCR4C |= (1<<PWM4D);           //Pulse Width Modulator D Enable
    TCCR4D |= (1<<WGM40);           //phase correct PWM

    //set timer0 to ctc, used for millis/led matrix
    TCCR0A |= (1<<WGM01);           //CTC mode
    TCCR0B |= (1<<CS01)|(1<<CS00);  //prescaler 64
    OCR0A = 124;                    //500uS
    TIMSK0 |= (1<<OCIE0A);          //compare match interrupt
}
