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

void Board::ledFlashStartup(bool fwUpdated)
{
    
}

void Board::initCustom()
{
    startUpAnimation = tanninStartUpAnimation;
}

Board board;
