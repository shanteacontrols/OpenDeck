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

    initUART_MIDI(MIDI_BAUD_RATE);
    initUSB_MIDI();

    configureTimers();
}

void Board::initPins()
{
    setInput(DI_1_PORT, DI_1_PIN);
    setHigh(DI_1_PORT, DI_1_PIN);

    setInput(DI_2_PORT, DI_2_PIN);
    setHigh(DI_2_PORT, DI_2_PIN);

    setInput(DI_3_PORT, DI_3_PIN);
    setHigh(DI_3_PORT, DI_3_PIN);

    setInput(DI_4_PORT, DI_4_PIN);
    setHigh(DI_4_PORT, DI_4_PIN);

    setInput(DI_5_PORT, DI_5_PIN);
    setHigh(DI_5_PORT, DI_5_PIN);

    setInput(DI_6_PORT, DI_6_PIN);
    setHigh(DI_6_PORT, DI_6_PIN);

    setInput(DI_7_PORT, DI_7_PIN);
    setHigh(DI_7_PORT, DI_7_PIN);

    setInput(DI_8_PORT, DI_8_PIN);
    setHigh(DI_8_PORT, DI_8_PIN);

    setInput(DI_9_PORT, DI_9_PIN);
    setHigh(DI_9_PORT, DI_9_PIN);

    setInput(DI_10_PORT, DI_10_PIN);
    setHigh(DI_10_PORT, DI_10_PIN);

    setInput(DI_11_PORT, DI_11_PIN);
    setHigh(DI_11_PORT, DI_11_PIN);

    setInput(DI_12_PORT, DI_12_PIN);
    setHigh(DI_12_PORT, DI_12_PIN);

    setInput(DI_13_PORT, DI_13_PIN);
    setHigh(DI_13_PORT, DI_13_PIN);

    setInput(DI_14_PORT, DI_14_PIN);
    setHigh(DI_14_PORT, DI_14_PIN);

    setInput(DI_15_PORT, DI_15_PIN);
    setHigh(DI_15_PORT, DI_15_PIN);

    setInput(DI_16_PORT, DI_16_PIN);
    setHigh(DI_16_PORT, DI_16_PIN);


    setOutput(DO_1_PORT, DO_1_PIN);
    setLow(DO_1_PORT, DO_1_PIN);

    setOutput(DO_2_PORT, DO_2_PIN);
    setLow(DO_2_PORT, DO_2_PIN);

    setOutput(DO_3_PORT, DO_3_PIN);
    setLow(DO_3_PORT, DO_3_PIN);

    setOutput(DO_4_PORT, DO_4_PIN);
    setLow(DO_4_PORT, DO_4_PIN);

    setOutput(DO_5_PORT, DO_5_PIN);
    setLow(DO_5_PORT, DO_5_PIN);

    setOutput(DO_6_PORT, DO_6_PIN);
    setLow(DO_6_PORT, DO_6_PIN);

    setOutput(DO_7_PORT, DO_7_PIN);
    setLow(DO_7_PORT, DO_7_PIN);

    setOutput(DO_8_PORT, DO_8_PIN);
    setLow(DO_8_PORT, DO_8_PIN);

    setOutput(DO_9_PORT, DO_9_PIN);
    setLow(DO_9_PORT, DO_9_PIN);

    setOutput(DO_10_PORT, DO_10_PIN);
    setLow(DO_10_PORT, DO_10_PIN);

    setOutput(DO_11_PORT, DO_11_PIN);
    setLow(DO_11_PORT, DO_11_PIN);

    setOutput(DO_12_PORT, DO_12_PIN);
    setLow(DO_12_PORT, DO_12_PIN);

    setOutput(DO_13_PORT, DO_13_PIN);
    setLow(DO_13_PORT, DO_13_PIN);

    setOutput(DO_14_PORT, DO_14_PIN);
    setLow(DO_14_PORT, DO_14_PIN);

    setOutput(DO_15_PORT, DO_15_PIN);
    setLow(DO_15_PORT, DO_15_PIN);

    setOutput(DO_16_PORT, DO_16_PIN);
    setLow(DO_16_PORT, DO_16_PIN);


    setInput(AI_1_PORT, AI_1_PIN);
    setLow(AI_1_PORT, AI_1_PIN);

    setInput(AI_2_PORT, AI_2_PIN);
    setLow(AI_2_PORT, AI_2_PIN);

    setInput(AI_3_PORT, AI_3_PIN);
    setLow(AI_3_PORT, AI_3_PIN);

    setInput(AI_4_PORT, AI_4_PIN);
    setLow(AI_4_PORT, AI_4_PIN);

    setInput(AI_5_PORT, AI_5_PIN);
    setLow(AI_5_PORT, AI_5_PIN);

    setInput(AI_6_PORT, AI_6_PIN);
    setLow(AI_6_PORT, AI_6_PIN);

    setInput(AI_7_PORT, AI_7_PIN);
    setLow(AI_7_PORT, AI_7_PIN);

    setInput(AI_8_PORT, AI_8_PIN);
    setLow(AI_8_PORT, AI_8_PIN);
}

void Board::configureTimers()
{
    //clear timer0 conf
    TCCR0A = 0;
    TCCR0B = 0;
    TIMSK0 = 0;

    //set timer0 to ctc, used for millis/led matrix
    TCCR0A |= (1<<WGM01);           //CTC mode
    TCCR0B |= (1<<CS01)|(1<<CS00);  //prescaler 64
    OCR0A = 62;                     //250us
    TIMSK0 |= (1<<OCIE0A);          //compare match interrupt
}
