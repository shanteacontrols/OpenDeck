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

#include "board/Internal.h"
#include "board/common/io/Helpers.h"
#include "Pins.h"
#include "core/src/general/IO.h"
#include "core/src/general/ADC.h"

namespace Board
{
    namespace detail
    {
        namespace setup
        {
            void io()
            {
                CORE_IO_CONFIG(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN, core::io::pinMode_t::input);
                CORE_IO_CONFIG(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN, core::io::pinMode_t::output);

                CORE_IO_CONFIG(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN, core::io::pinMode_t::output);

                CORE_IO_CONFIG(MUX_S0_PORT, MUX_S0_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(MUX_S1_PORT, MUX_S1_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(MUX_S2_PORT, MUX_S2_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(MUX_S3_PORT, MUX_S3_PIN, core::io::pinMode_t::output);

                CORE_IO_CONFIG(MUX_1_IN_PORT, MUX_1_IN_PIN, core::io::pinMode_t::input);
                CORE_IO_CONFIG(MUX_2_IN_PORT, MUX_2_IN_PIN, core::io::pinMode_t::input);
                CORE_IO_CONFIG(MUX_3_IN_PORT, MUX_3_IN_PIN, core::io::pinMode_t::input);
                CORE_IO_CONFIG(MUX_4_IN_PORT, MUX_4_IN_PIN, core::io::pinMode_t::input);

                //unused pins
                CORE_IO_CONFIG(PORTB, 0, core::io::pinMode_t::output);
                CORE_IO_CONFIG(PORTB, 4, core::io::pinMode_t::output);
                CORE_IO_CONFIG(PORTB, 5, core::io::pinMode_t::output);
                CORE_IO_CONFIG(PORTB, 6, core::io::pinMode_t::output);
                CORE_IO_CONFIG(PORTB, 7, core::io::pinMode_t::output);

                CORE_IO_CONFIG(PORTC, 6, core::io::pinMode_t::output);
                CORE_IO_CONFIG(PORTC, 7, core::io::pinMode_t::output);

                CORE_IO_CONFIG(PORTD, 6, core::io::pinMode_t::output);
                CORE_IO_CONFIG(PORTD, 7, core::io::pinMode_t::output);

                CORE_IO_CONFIG(PORTE, 2, core::io::pinMode_t::output);

                CORE_IO_CONFIG(PORTF, 0, core::io::pinMode_t::output);
                CORE_IO_CONFIG(PORTF, 1, core::io::pinMode_t::output);

                //make sure all unused pins are logic low
                CORE_IO_SET_LOW(PORTB, 0);
                CORE_IO_SET_LOW(PORTB, 4);
                CORE_IO_SET_LOW(PORTB, 5);
                CORE_IO_SET_LOW(PORTB, 6);
                CORE_IO_SET_LOW(PORTB, 7);

                CORE_IO_SET_LOW(PORTC, 6);
                CORE_IO_SET_LOW(PORTC, 7);

                CORE_IO_SET_LOW(PORTD, 6);
                CORE_IO_SET_LOW(PORTD, 7);

                CORE_IO_SET_LOW(PORTE, 2);

                CORE_IO_SET_LOW(PORTF, 0);
                CORE_IO_SET_LOW(PORTF, 1);

                //init all outputs on shift register
                CORE_IO_SET_LOW(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

                for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
                {
                    EXT_LED_OFF(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
                    CORE_IO_SET_HIGH(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                    _NOP();
                    _NOP();
                    CORE_IO_SET_LOW(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                }

                CORE_IO_SET_HIGH(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);
            }

            void adc()
            {
                core::adc::disconnectDigitalIn(Board::detail::map::adcChannel(0));
                core::adc::disconnectDigitalIn(Board::detail::map::adcChannel(1));
                core::adc::disconnectDigitalIn(Board::detail::map::adcChannel(2));
                core::adc::disconnectDigitalIn(Board::detail::map::adcChannel(3));

                core::adc::conf_t adcConfiguration;

                adcConfiguration.prescaler = core::adc::prescaler_t::p128;
                adcConfiguration.vref      = core::adc::vRef_t::aref;

                core::adc::setup(adcConfiguration);
                core::adc::setChannel(Board::detail::map::adcChannel(0));

                for (int i = 0; i < 3; i++)
                    core::adc::read();    //few dummy reads to init ADC

                core::adc::enableInterrupt();
                core::adc::startConversion();
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
    }        // namespace detail
}    // namespace Board