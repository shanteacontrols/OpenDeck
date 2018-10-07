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
#include "board/common/constants/LEDs.h"
#include "core/src/HAL/avr/PinManipulation.h"
#include "core/src/HAL/avr/adc/ADC.h"

void Board::initPins()
{
    setInput(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN);
    setOutput(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
    setOutput(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);

    setOutput(MUX_S0_PORT, MUX_S0_PIN);
    setOutput(MUX_S1_PORT, MUX_S1_PIN);
    setOutput(MUX_S2_PORT, MUX_S2_PIN);
    setOutput(MUX_S3_PORT, MUX_S3_PIN);

    setInput(MUX_1_IN_PORT, MUX_1_IN_PIN);

    setInput(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);

    //unused pins
    setOutput(PORTB, 0);
    setOutput(PORTB, 4);
    setOutput(PORTB, 5);
    setOutput(PORTB, 6);
    setOutput(PORTB, 7);

    setOutput(PORTC, 6);
    setOutput(PORTC, 7);

    setOutput(PORTD, 0);
    setOutput(PORTD, 1);
    setOutput(PORTD, 5);

    setOutput(PORTE, 2);
    setOutput(PORTE, 6);

    //make sure all unused pins are logic low
    setLow(PORTB, 0);
    setLow(PORTB, 4);
    setLow(PORTB, 5);
    setLow(PORTB, 6);
    setLow(PORTB, 7);

    setLow(PORTC, 6);
    setLow(PORTC, 7);

    setLow(PORTD, 0);
    setLow(PORTD, 1);
    setLow(PORTD, 5);

    setLow(PORTE, 2);
    setLow(PORTE, 6);
}

void Board::initAnalog()
{
    disconnectDigitalInADC(adcChannelArray[0]);

    adcConf adcConfiguration;

    adcConfiguration.prescaler = ADC_PRESCALER_128;
    adcConfiguration.vref = ADC_VREF_AREF;

    setUpADC(adcConfiguration);
    setADCchannel(adcChannelArray[0]);

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

    //set timer0 to ctc
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

}