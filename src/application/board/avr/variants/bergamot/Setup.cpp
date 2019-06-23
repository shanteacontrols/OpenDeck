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

#include "board/common/Map.h"
#include "Pins.h"
#include "core/src/avr/PinManipulation.h"
#include "core/src/avr/ADC.h"

namespace Board
{
    namespace setup
    {
        void pins()
        {
            CORE_AVR_PIN_SET_INPUT(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN);
            CORE_AVR_PIN_SET_OUTPUT(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
            CORE_AVR_PIN_SET_OUTPUT(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);

            CORE_AVR_PIN_SET_OUTPUT(MUX_S0_PORT, MUX_S0_PIN);
            CORE_AVR_PIN_SET_OUTPUT(MUX_S1_PORT, MUX_S1_PIN);
            CORE_AVR_PIN_SET_OUTPUT(MUX_S2_PORT, MUX_S2_PIN);
            CORE_AVR_PIN_SET_OUTPUT(MUX_S3_PORT, MUX_S3_PIN);

            CORE_AVR_PIN_SET_INPUT(MUX_1_IN_PORT, MUX_1_IN_PIN);

            CORE_AVR_PIN_SET_INPUT(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);

            //unused pins
            CORE_AVR_PIN_SET_OUTPUT(PORTB, 0);
            CORE_AVR_PIN_SET_OUTPUT(PORTB, 4);
            CORE_AVR_PIN_SET_OUTPUT(PORTB, 5);
            CORE_AVR_PIN_SET_OUTPUT(PORTB, 6);
            CORE_AVR_PIN_SET_OUTPUT(PORTB, 7);

            CORE_AVR_PIN_SET_OUTPUT(PORTC, 6);
            CORE_AVR_PIN_SET_OUTPUT(PORTC, 7);

            CORE_AVR_PIN_SET_OUTPUT(PORTD, 0);
            CORE_AVR_PIN_SET_OUTPUT(PORTD, 1);
            CORE_AVR_PIN_SET_OUTPUT(PORTD, 5);

            CORE_AVR_PIN_SET_OUTPUT(PORTE, 2);
            CORE_AVR_PIN_SET_OUTPUT(PORTE, 6);

            //make sure all unused pins are logic low
            CORE_AVR_PIN_SET_LOW(PORTB, 0);
            CORE_AVR_PIN_SET_LOW(PORTB, 4);
            CORE_AVR_PIN_SET_LOW(PORTB, 5);
            CORE_AVR_PIN_SET_LOW(PORTB, 6);
            CORE_AVR_PIN_SET_LOW(PORTB, 7);

            CORE_AVR_PIN_SET_LOW(PORTC, 6);
            CORE_AVR_PIN_SET_LOW(PORTC, 7);

            CORE_AVR_PIN_SET_LOW(PORTD, 0);
            CORE_AVR_PIN_SET_LOW(PORTD, 1);
            CORE_AVR_PIN_SET_LOW(PORTD, 5);

            CORE_AVR_PIN_SET_LOW(PORTE, 2);
            CORE_AVR_PIN_SET_LOW(PORTE, 6);
        }

        void adc()
        {
            using namespace core::avr;

            adc::disconnectDigitalIn(Board::map::adcChannel(0));

            adc::conf_t adcConfiguration;

            adcConfiguration.prescaler = adc::prescaler_t::p128;
            adcConfiguration.vref = adc::vRef_t::aref;

            adc::setup(adcConfiguration);
            adc::setChannel(Board::map::adcChannel(0));

            for (int i = 0; i < 3; i++)
                adc::read();    //few dummy reads to init ADC

            adc::enableInterrupt();
            adc::startConversion();
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

            //set timer0 to ctc
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