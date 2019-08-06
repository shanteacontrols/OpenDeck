/*

Copyright 2015-2019 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <util/delay.h>
#include "board/Board.h"
#include "board/avr/Setup.h"
#include "Pins.h"
#include "core/src/avr/PinManipulation.h"
#include "core/src/avr/ADC.h"
#include "board/common/constants/LEDs.h"
#include "board/common/Map.h"

namespace Board
{
    namespace setup
    {
        void pins()
        {
            CORE_AVR_PIN_SET_INPUT(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN);
            CORE_AVR_PIN_SET_OUTPUT(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
            CORE_AVR_PIN_SET_OUTPUT(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);

            CORE_AVR_PIN_SET_OUTPUT(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
            CORE_AVR_PIN_SET_OUTPUT(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
            CORE_AVR_PIN_SET_OUTPUT(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

            CORE_AVR_PIN_SET_OUTPUT(MUX_S0_PORT, MUX_S0_PIN);
            CORE_AVR_PIN_SET_OUTPUT(MUX_S1_PORT, MUX_S1_PIN);
            CORE_AVR_PIN_SET_OUTPUT(MUX_S2_PORT, MUX_S2_PIN);
            CORE_AVR_PIN_SET_OUTPUT(MUX_S3_PORT, MUX_S3_PIN);

            CORE_AVR_PIN_SET_INPUT(MUX_1_IN_PORT, MUX_1_IN_PIN);
            CORE_AVR_PIN_SET_INPUT(MUX_2_IN_PORT, MUX_2_IN_PIN);
            CORE_AVR_PIN_SET_INPUT(MUX_3_IN_PORT, MUX_3_IN_PIN);
            CORE_AVR_PIN_SET_INPUT(MUX_4_IN_PORT, MUX_4_IN_PIN);

            //unused pins
            CORE_AVR_PIN_SET_OUTPUT(PORTB, 0);
            CORE_AVR_PIN_SET_OUTPUT(PORTB, 4);
            CORE_AVR_PIN_SET_OUTPUT(PORTB, 5);
            CORE_AVR_PIN_SET_OUTPUT(PORTB, 6);
            CORE_AVR_PIN_SET_OUTPUT(PORTB, 7);

            CORE_AVR_PIN_SET_OUTPUT(PORTC, 6);
            CORE_AVR_PIN_SET_OUTPUT(PORTC, 7);

            CORE_AVR_PIN_SET_OUTPUT(PORTD, 6);
            CORE_AVR_PIN_SET_OUTPUT(PORTD, 7);

            CORE_AVR_PIN_SET_OUTPUT(PORTE, 2);

            CORE_AVR_PIN_SET_OUTPUT(PORTF, 0);
            CORE_AVR_PIN_SET_OUTPUT(PORTF, 1);

            //make sure all unused pins are logic low
            CORE_AVR_PIN_SET_LOW(PORTB, 0);
            CORE_AVR_PIN_SET_LOW(PORTB, 4);
            CORE_AVR_PIN_SET_LOW(PORTB, 5);
            CORE_AVR_PIN_SET_LOW(PORTB, 6);
            CORE_AVR_PIN_SET_LOW(PORTB, 7);

            CORE_AVR_PIN_SET_LOW(PORTC, 6);
            CORE_AVR_PIN_SET_LOW(PORTC, 7);

            CORE_AVR_PIN_SET_LOW(PORTD, 6);
            CORE_AVR_PIN_SET_LOW(PORTD, 7);

            CORE_AVR_PIN_SET_LOW(PORTE, 2);

            CORE_AVR_PIN_SET_LOW(PORTF, 0);
            CORE_AVR_PIN_SET_LOW(PORTF, 1);

            //init all outputs on shift register
            CORE_AVR_PIN_SET_LOW(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

            for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
            {
                EXT_LED_OFF(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
                CORE_AVR_PIN_SET_HIGH(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                _NOP();
                _NOP();
                CORE_AVR_PIN_SET_LOW(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
            }

            CORE_AVR_PIN_SET_HIGH(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);
        }

        void adc()
        {
            core::CORE_ARCH::adc::disconnectDigitalIn(Board::map::adcChannel(0));
            core::CORE_ARCH::adc::disconnectDigitalIn(Board::map::adcChannel(1));
            core::CORE_ARCH::adc::disconnectDigitalIn(Board::map::adcChannel(2));
            core::CORE_ARCH::adc::disconnectDigitalIn(Board::map::adcChannel(3));

            core::CORE_ARCH::adc::conf_t adcConfiguration;

            adcConfiguration.prescaler = core::CORE_ARCH::adc::prescaler_t::p128;
            adcConfiguration.vref = core::CORE_ARCH::adc::vRef_t::aref;

            core::CORE_ARCH::adc::setup(adcConfiguration);
            core::CORE_ARCH::adc::setChannel(Board::map::adcChannel(0));

            for (int i = 0; i < 3; i++)
                core::CORE_ARCH::adc::read();    //few dummy reads to init ADC

            core::CORE_ARCH::adc::enableInterrupt();
            core::CORE_ARCH::adc::startConversion();
        }

        void timers()
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
            TCCR0A |= (1 << WGM01);                 //CTC mode
            TCCR0B |= (1 << CS01) | (1 << CS00);    //prescaler 64
            OCR0A = 124;                            //500us
            TIMSK0 |= (1 << OCIE0A);                //compare match interrupt
        }
    }    // namespace setup

    void ledFlashStartup(bool fwUpdated)
    {
    }
}    // namespace Board