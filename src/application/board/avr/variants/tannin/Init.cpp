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
#include "pins/Pins.h"
#include "board/common/constants/LEDs.h"
#include "interface/digital/output/leds/Constants.h"
#include "../../../../interface/digital/output/leds/Variables.h"

#define START_UP_ANIM_DELAY 120

void tanninStartUpAnimation()
{
    //on sequence
    BIT_SET(ledState[3], LED_ACTIVE_BIT);
    BIT_SET(ledState[3], LED_STATE_BIT);
    BIT_SET(ledState[19], LED_ACTIVE_BIT);
    BIT_SET(ledState[19], LED_STATE_BIT);
    wait_ms(START_UP_ANIM_DELAY);

    BIT_SET(ledState[2], LED_ACTIVE_BIT);
    BIT_SET(ledState[2], LED_STATE_BIT);
    BIT_SET(ledState[18], LED_ACTIVE_BIT);
    BIT_SET(ledState[18], LED_STATE_BIT);
    wait_ms(START_UP_ANIM_DELAY);

    BIT_SET(ledState[1], LED_ACTIVE_BIT);
    BIT_SET(ledState[1], LED_STATE_BIT);
    BIT_SET(ledState[17], LED_ACTIVE_BIT);
    BIT_SET(ledState[17], LED_STATE_BIT);
    BIT_SET(ledState[11], LED_ACTIVE_BIT);
    BIT_SET(ledState[11], LED_STATE_BIT);
    BIT_SET(ledState[27], LED_ACTIVE_BIT);
    BIT_SET(ledState[27], LED_STATE_BIT);
    wait_ms(START_UP_ANIM_DELAY);

    BIT_SET(ledState[0], LED_ACTIVE_BIT);
    BIT_SET(ledState[0], LED_STATE_BIT);
    BIT_SET(ledState[16], LED_ACTIVE_BIT);
    BIT_SET(ledState[16], LED_STATE_BIT);
    BIT_SET(ledState[10], LED_ACTIVE_BIT);
    BIT_SET(ledState[10], LED_STATE_BIT);
    BIT_SET(ledState[26], LED_ACTIVE_BIT);
    BIT_SET(ledState[26], LED_STATE_BIT);
    wait_ms(START_UP_ANIM_DELAY);

    BIT_SET(ledState[9], LED_ACTIVE_BIT);
    BIT_SET(ledState[9], LED_STATE_BIT);
    BIT_SET(ledState[25], LED_ACTIVE_BIT);
    BIT_SET(ledState[25], LED_STATE_BIT);
    wait_ms(START_UP_ANIM_DELAY);

    BIT_SET(ledState[8], LED_ACTIVE_BIT);
    BIT_SET(ledState[8], LED_STATE_BIT);
    BIT_SET(ledState[24], LED_ACTIVE_BIT);
    BIT_SET(ledState[24], LED_STATE_BIT);
    wait_ms(2000);

    //off sequence
    BIT_CLEAR(ledState[0], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[0], LED_STATE_BIT);
    BIT_CLEAR(ledState[16], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[16], LED_STATE_BIT);
    wait_ms(START_UP_ANIM_DELAY);

    BIT_CLEAR(ledState[1], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[1], LED_STATE_BIT);
    BIT_CLEAR(ledState[17], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[17], LED_STATE_BIT);
    wait_ms(START_UP_ANIM_DELAY);

    BIT_CLEAR(ledState[2], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[2], LED_STATE_BIT);
    BIT_CLEAR(ledState[18], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[18], LED_STATE_BIT);
    BIT_CLEAR(ledState[8], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[8], LED_STATE_BIT);
    BIT_CLEAR(ledState[24], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[24], LED_STATE_BIT);
    wait_ms(START_UP_ANIM_DELAY);

    BIT_CLEAR(ledState[3], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[3], LED_STATE_BIT);
    BIT_CLEAR(ledState[19], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[19], LED_STATE_BIT);
    BIT_CLEAR(ledState[9], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[9], LED_STATE_BIT);
    BIT_CLEAR(ledState[25], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[25], LED_STATE_BIT);
    wait_ms(START_UP_ANIM_DELAY);

    BIT_CLEAR(ledState[10], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[10], LED_STATE_BIT);
    BIT_CLEAR(ledState[26], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[26], LED_STATE_BIT);
    wait_ms(START_UP_ANIM_DELAY);

    BIT_CLEAR(ledState[11], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[11], LED_STATE_BIT);
    BIT_CLEAR(ledState[27], LED_ACTIVE_BIT);
    BIT_CLEAR(ledState[27], LED_STATE_BIT);
    wait_ms(2000);
}

void Board::initPins()
{
    //configure input matrix
    //shift register
    setInput(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN);
    setOutput(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
    setOutput(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);

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

    //make sure to turn all rows off
    EXT_LED_OFF(LED_ROW_1_PORT, LED_ROW_1_PIN);
    EXT_LED_OFF(LED_ROW_2_PORT, LED_ROW_2_PIN);
    EXT_LED_OFF(LED_ROW_3_PORT, LED_ROW_3_PIN);
    EXT_LED_OFF(LED_ROW_4_PORT, LED_ROW_4_PIN);

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

    setLow(MUX_S0_PORT, MUX_S0_PIN);
    setLow(MUX_S1_PORT, MUX_S1_PIN);
    setLow(MUX_S2_PORT, MUX_S2_PIN);
    setLow(MUX_S3_PORT, MUX_S3_PIN);

    //mux inputs
    setInput(MUX_1_IN_PORT, MUX_1_IN_PIN);
}

void Board::initAnalog()
{
    disconnectDigitalInADC(MUX_1_IN_PIN);

    adcConf adcConfiguration;

    adcConfiguration.prescaler = ADC_PRESCALER_128;
    adcConfiguration.vref = ADC_VREF_AREF;

    setUpADC(adcConfiguration);
    setADCchannel(MUX_1_IN_PIN);

    for (int i=0; i<3; i++)
        getADCvalue();  //few dummy reads to init ADC

    adcInterruptEnable();
    startADCconversion();
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
    OCR0A = 124;                    //500us
    TIMSK0 |= (1<<OCIE0A);          //compare match interrupt
}

void Board::ledFlashStartup(bool fwUpdated)
{
    
}

void Board::initCustom()
{
    startUpAnimation = tanninStartUpAnimation;
}

Board board;
