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

void Board::init()
{
    cli();
    //disable watchdog
    MCUSR &= ~(1 << WDRF);
    wdt_disable();
    initPins();
    initAnalog();
    initEncoders();

    initUART_MIDI(MIDI_BAUD_RATE_STD);
    initUSB_MIDI();

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

    //bootloader/midi leds
    setOutput(LED_IN_PORT, LED_IN_PIN);
    setOutput(LED_OUT_PORT, LED_OUT_PIN);

    MIDI_LED_OFF(LED_IN_PORT, LED_IN_PIN);
    MIDI_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
}

