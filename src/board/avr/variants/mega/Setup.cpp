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
                CORE_IO_CONFIG(DI_1_PORT, DI_1_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_1_PORT, DI_1_PIN);

                CORE_IO_CONFIG(DI_2_PORT, DI_2_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_2_PORT, DI_2_PIN);

                CORE_IO_CONFIG(DI_3_PORT, DI_3_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_3_PORT, DI_3_PIN);

                CORE_IO_CONFIG(DI_4_PORT, DI_4_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_4_PORT, DI_4_PIN);

                CORE_IO_CONFIG(DI_5_PORT, DI_5_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_5_PORT, DI_5_PIN);

                CORE_IO_CONFIG(DI_6_PORT, DI_6_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_6_PORT, DI_6_PIN);

                CORE_IO_CONFIG(DI_7_PORT, DI_7_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_7_PORT, DI_7_PIN);

                CORE_IO_CONFIG(DI_8_PORT, DI_8_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_8_PORT, DI_8_PIN);

                CORE_IO_CONFIG(DI_9_PORT, DI_9_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_9_PORT, DI_9_PIN);

                CORE_IO_CONFIG(DI_10_PORT, DI_10_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_10_PORT, DI_10_PIN);

                CORE_IO_CONFIG(DI_11_PORT, DI_11_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_11_PORT, DI_11_PIN);

                CORE_IO_CONFIG(DI_12_PORT, DI_12_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_12_PORT, DI_12_PIN);

                CORE_IO_CONFIG(DI_13_PORT, DI_13_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_13_PORT, DI_13_PIN);

                CORE_IO_CONFIG(DI_14_PORT, DI_14_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_14_PORT, DI_14_PIN);

                CORE_IO_CONFIG(DI_15_PORT, DI_15_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_15_PORT, DI_15_PIN);

                CORE_IO_CONFIG(DI_16_PORT, DI_16_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_16_PORT, DI_16_PIN);

                CORE_IO_CONFIG(DI_17_PORT, DI_17_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_17_PORT, DI_17_PIN);

                CORE_IO_CONFIG(DI_18_PORT, DI_18_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_18_PORT, DI_18_PIN);

                CORE_IO_CONFIG(DI_19_PORT, DI_19_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_19_PORT, DI_19_PIN);

                CORE_IO_CONFIG(DI_20_PORT, DI_20_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_20_PORT, DI_20_PIN);

                CORE_IO_CONFIG(DI_21_PORT, DI_21_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_21_PORT, DI_21_PIN);

                CORE_IO_CONFIG(DI_22_PORT, DI_22_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_22_PORT, DI_22_PIN);

                CORE_IO_CONFIG(DI_23_PORT, DI_23_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_23_PORT, DI_23_PIN);

                CORE_IO_CONFIG(DI_24_PORT, DI_24_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_24_PORT, DI_24_PIN);

                CORE_IO_CONFIG(DI_25_PORT, DI_25_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_25_PORT, DI_25_PIN);

                CORE_IO_CONFIG(DI_26_PORT, DI_26_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_26_PORT, DI_26_PIN);

                CORE_IO_CONFIG(DI_27_PORT, DI_27_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_27_PORT, DI_27_PIN);

                CORE_IO_CONFIG(DI_28_PORT, DI_28_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_28_PORT, DI_28_PIN);

                CORE_IO_CONFIG(DI_29_PORT, DI_29_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_29_PORT, DI_29_PIN);

                CORE_IO_CONFIG(DI_30_PORT, DI_30_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_30_PORT, DI_30_PIN);

                CORE_IO_CONFIG(DI_31_PORT, DI_31_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_31_PORT, DI_31_PIN);

                CORE_IO_CONFIG(DI_32_PORT, DI_32_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(DI_32_PORT, DI_32_PIN);

                CORE_IO_CONFIG(DO_1_PORT, DO_1_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_1_PORT, DO_1_PIN);

                CORE_IO_CONFIG(DO_2_PORT, DO_2_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_2_PORT, DO_2_PIN);

                CORE_IO_CONFIG(DO_3_PORT, DO_3_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_3_PORT, DO_3_PIN);

                CORE_IO_CONFIG(DO_4_PORT, DO_4_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_4_PORT, DO_4_PIN);

                CORE_IO_CONFIG(DO_5_PORT, DO_5_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_5_PORT, DO_5_PIN);

                CORE_IO_CONFIG(DO_6_PORT, DO_6_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_6_PORT, DO_6_PIN);

                CORE_IO_CONFIG(DO_7_PORT, DO_7_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_7_PORT, DO_7_PIN);

                CORE_IO_CONFIG(DO_8_PORT, DO_8_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_8_PORT, DO_8_PIN);

                CORE_IO_CONFIG(DO_9_PORT, DO_9_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_9_PORT, DO_9_PIN);

                CORE_IO_CONFIG(DO_10_PORT, DO_10_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_10_PORT, DO_10_PIN);

                CORE_IO_CONFIG(DO_11_PORT, DO_11_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_11_PORT, DO_11_PIN);

                CORE_IO_CONFIG(DO_12_PORT, DO_12_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_12_PORT, DO_12_PIN);

                CORE_IO_CONFIG(DO_13_PORT, DO_13_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_13_PORT, DO_13_PIN);

                CORE_IO_CONFIG(DO_14_PORT, DO_14_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_14_PORT, DO_14_PIN);

                CORE_IO_CONFIG(DO_15_PORT, DO_15_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_15_PORT, DO_15_PIN);

                CORE_IO_CONFIG(DO_16_PORT, DO_16_PIN, core::io::pinMode_t::output);
                EXT_LED_OFF(DO_16_PORT, DO_16_PIN);

                CORE_IO_CONFIG(AI_1_PORT, AI_1_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_1_PORT, AI_1_PIN);

                CORE_IO_CONFIG(AI_2_PORT, AI_2_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_2_PORT, AI_2_PIN);

                CORE_IO_CONFIG(AI_3_PORT, AI_3_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_3_PORT, AI_3_PIN);

                CORE_IO_CONFIG(AI_4_PORT, AI_4_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_4_PORT, AI_4_PIN);

                CORE_IO_CONFIG(AI_5_PORT, AI_5_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_5_PORT, AI_5_PIN);

                CORE_IO_CONFIG(AI_6_PORT, AI_6_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_6_PORT, AI_6_PIN);

                CORE_IO_CONFIG(AI_7_PORT, AI_7_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_7_PORT, AI_7_PIN);

                CORE_IO_CONFIG(AI_8_PORT, AI_8_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_8_PORT, AI_8_PIN);

                CORE_IO_CONFIG(AI_9_PORT, AI_9_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_9_PORT, AI_9_PIN);

                CORE_IO_CONFIG(AI_10_PORT, AI_10_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_10_PORT, AI_10_PIN);

                CORE_IO_CONFIG(AI_11_PORT, AI_11_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_11_PORT, AI_11_PIN);

                CORE_IO_CONFIG(AI_12_PORT, AI_12_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_12_PORT, AI_12_PIN);

                CORE_IO_CONFIG(AI_13_PORT, AI_13_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_13_PORT, AI_13_PIN);

                CORE_IO_CONFIG(AI_14_PORT, AI_14_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_14_PORT, AI_14_PIN);

                CORE_IO_CONFIG(AI_15_PORT, AI_15_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_15_PORT, AI_15_PIN);

                CORE_IO_CONFIG(AI_16_PORT, AI_16_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_LOW(AI_16_PORT, AI_16_PIN);

                CORE_IO_CONFIG(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN, core::io::pinMode_t::input);
                CORE_IO_SET_HIGH(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
            }

            void adc()
            {
                core::adc::conf_t adcConfiguration;

                adcConfiguration.prescaler = core::adc::prescaler_t::p128;
                adcConfiguration.vref = core::adc::vRef_t::avcc;

                core::adc::setup(adcConfiguration);
                core::adc::setChannel(Board::detail::map::adcChannel(0).index);

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

                //set timer0 to ctc, used for millis/led matrix
                TCCR0A |= (1 << WGM01);                 //CTC mode
                TCCR0B |= (1 << CS01) | (1 << CS00);    //prescaler 64
                OCR0A = 124;                            //500us
                TIMSK0 |= (1 << OCIE0A);                //compare match interrupt
            }
        }    // namespace setup
    }        // namespace detail
}    // namespace Board