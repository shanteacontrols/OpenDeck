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
#include "Pins.h"
#include "board/common/Map.h"
#include "board/common/constants/LEDs.h"
#include "core/src/avr/PinManipulation.h"
#include "core/src/avr/ADC.h"

namespace Board
{
    namespace setup
    {
        void pins()
        {
            CORE_AVR_PIN_SET_INPUT(DI_1_PORT, DI_1_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_1_PORT, DI_1_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_2_PORT, DI_2_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_2_PORT, DI_2_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_3_PORT, DI_3_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_3_PORT, DI_3_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_4_PORT, DI_4_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_4_PORT, DI_4_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_5_PORT, DI_5_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_5_PORT, DI_5_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_6_PORT, DI_6_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_6_PORT, DI_6_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_7_PORT, DI_7_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_7_PORT, DI_7_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_8_PORT, DI_8_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_8_PORT, DI_8_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_9_PORT, DI_9_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_9_PORT, DI_9_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_10_PORT, DI_10_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_10_PORT, DI_10_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_11_PORT, DI_11_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_11_PORT, DI_11_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_12_PORT, DI_12_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_12_PORT, DI_12_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_13_PORT, DI_13_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_13_PORT, DI_13_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_14_PORT, DI_14_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_14_PORT, DI_14_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_15_PORT, DI_15_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_15_PORT, DI_15_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_16_PORT, DI_16_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_16_PORT, DI_16_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_1_PORT, DO_1_PIN);
            EXT_LED_OFF(DO_1_PORT, DO_1_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_2_PORT, DO_2_PIN);
            EXT_LED_OFF(DO_2_PORT, DO_2_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_3_PORT, DO_3_PIN);
            EXT_LED_OFF(DO_3_PORT, DO_3_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_4_PORT, DO_4_PIN);
            EXT_LED_OFF(DO_4_PORT, DO_4_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_5_PORT, DO_5_PIN);
            EXT_LED_OFF(DO_5_PORT, DO_5_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_6_PORT, DO_6_PIN);
            EXT_LED_OFF(DO_6_PORT, DO_6_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_7_PORT, DO_7_PIN);
            EXT_LED_OFF(DO_7_PORT, DO_7_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_8_PORT, DO_8_PIN);
            EXT_LED_OFF(DO_8_PORT, DO_8_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_9_PORT, DO_9_PIN);
            EXT_LED_OFF(DO_9_PORT, DO_9_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_10_PORT, DO_10_PIN);
            EXT_LED_OFF(DO_10_PORT, DO_10_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_11_PORT, DO_11_PIN);
            EXT_LED_OFF(DO_11_PORT, DO_11_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_12_PORT, DO_12_PIN);
            EXT_LED_OFF(DO_12_PORT, DO_12_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_13_PORT, DO_13_PIN);
            EXT_LED_OFF(DO_13_PORT, DO_13_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_14_PORT, DO_14_PIN);
            EXT_LED_OFF(DO_14_PORT, DO_14_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_15_PORT, DO_15_PIN);
            EXT_LED_OFF(DO_15_PORT, DO_15_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_16_PORT, DO_16_PIN);
            EXT_LED_OFF(DO_16_PORT, DO_16_PIN);

            CORE_AVR_PIN_SET_INPUT(AI_1_PORT, AI_1_PIN);
            CORE_AVR_PIN_SET_LOW(AI_1_PORT, AI_1_PIN);

            CORE_AVR_PIN_SET_INPUT(AI_2_PORT, AI_2_PIN);
            CORE_AVR_PIN_SET_LOW(AI_2_PORT, AI_2_PIN);

            CORE_AVR_PIN_SET_INPUT(AI_3_PORT, AI_3_PIN);
            CORE_AVR_PIN_SET_LOW(AI_3_PORT, AI_3_PIN);

            CORE_AVR_PIN_SET_INPUT(AI_4_PORT, AI_4_PIN);
            CORE_AVR_PIN_SET_LOW(AI_4_PORT, AI_4_PIN);

            CORE_AVR_PIN_SET_INPUT(AI_5_PORT, AI_5_PIN);
            CORE_AVR_PIN_SET_LOW(AI_5_PORT, AI_5_PIN);

            CORE_AVR_PIN_SET_INPUT(AI_6_PORT, AI_6_PIN);
            CORE_AVR_PIN_SET_LOW(AI_6_PORT, AI_6_PIN);

            CORE_AVR_PIN_SET_INPUT(AI_7_PORT, AI_7_PIN);
            CORE_AVR_PIN_SET_LOW(AI_7_PORT, AI_7_PIN);

            CORE_AVR_PIN_SET_INPUT(AI_8_PORT, AI_8_PIN);
            CORE_AVR_PIN_SET_LOW(AI_8_PORT, AI_8_PIN);
        }

        void adc()
        {
            using namespace core::avr;

            adc::conf_t adcConfiguration;

            adcConfiguration.prescaler = adc::prescaler_t::p128;
            adcConfiguration.vref = adc::vRef_t::avcc;

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

            //set timer0 to ctc, used for millis/led matrix
            TCCR0A |= (1 << WGM01);                 //CTC mode
            TCCR0B |= (1 << CS01) | (1 << CS00);    //prescaler 64
            OCR0A = 124;                            //500us
            TIMSK0 |= (1 << OCIE0A);                //compare match interrupt
        }
    }    // namespace setup

    void ledFlashStartup(bool fwUpdated)
    {
        //block interrupts here to avoid received midi traffic messing with indicator leds animation
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            for (int i = 0; i < 3; i++)
            {
                if (fwUpdated)
                {
                    INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
                    _delay_ms(500);
                    INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
                    _delay_ms(500);
                }
                else
                {
                    INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
                    _delay_ms(200);
                    INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
                    _delay_ms(200);
                }
            }
        }
    }
}    // namespace Board