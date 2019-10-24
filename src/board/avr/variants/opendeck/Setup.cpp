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
#include "board/common/Map.h"
#include "core/src/general/IO.h"
#include "core/src/general/Atomic.h"
#include "core/src/general/ADC.h"
#include "board/common/constants/LEDs.h"

namespace Board
{
    namespace setup
    {
        void pins()
        {
            //configure input matrix
            //shift register
            CORE_IO_CONFIG(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN, core::io::pinMode_t::input);
            CORE_IO_CONFIG(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN, core::io::pinMode_t::output);

            //decoder
            CORE_IO_CONFIG(DEC_DM_A0_PORT, DEC_DM_A0_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(DEC_DM_A1_PORT, DEC_DM_A1_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(DEC_DM_A1_PORT, DEC_DM_A2_PIN, core::io::pinMode_t::output);

            //configure led matrix
            //rows

            CORE_IO_CONFIG(LED_ROW_1_PORT, LED_ROW_1_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(LED_ROW_2_PORT, LED_ROW_2_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(LED_ROW_3_PORT, LED_ROW_3_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(LED_ROW_4_PORT, LED_ROW_4_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(LED_ROW_5_PORT, LED_ROW_5_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(LED_ROW_6_PORT, LED_ROW_6_PIN, core::io::pinMode_t::output);

            //make sure to turn all rows off
            EXT_LED_OFF(LED_ROW_1_PORT, LED_ROW_1_PIN);
            EXT_LED_OFF(LED_ROW_2_PORT, LED_ROW_2_PIN);
            EXT_LED_OFF(LED_ROW_3_PORT, LED_ROW_3_PIN);
            EXT_LED_OFF(LED_ROW_4_PORT, LED_ROW_4_PIN);
            EXT_LED_OFF(LED_ROW_5_PORT, LED_ROW_5_PIN);
            EXT_LED_OFF(LED_ROW_6_PORT, LED_ROW_6_PIN);

            //decoder
            CORE_IO_CONFIG(DEC_LM_A0_PORT, DEC_LM_A0_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(DEC_LM_A1_PORT, DEC_LM_A1_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(DEC_LM_A2_PORT, DEC_LM_A2_PIN, core::io::pinMode_t::output);

            //configure analog
            //select pins
            CORE_IO_CONFIG(MUX_S0_PORT, MUX_S0_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(MUX_S1_PORT, MUX_S1_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(MUX_S2_PORT, MUX_S2_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(MUX_S3_PORT, MUX_S3_PIN, core::io::pinMode_t::output);

            CORE_IO_SET_LOW(MUX_S0_PORT, MUX_S0_PIN);
            CORE_IO_SET_LOW(MUX_S1_PORT, MUX_S1_PIN);
            CORE_IO_SET_LOW(MUX_S2_PORT, MUX_S2_PIN);
            CORE_IO_SET_LOW(MUX_S3_PORT, MUX_S3_PIN);

            //mux inputs
            CORE_IO_CONFIG(MUX_1_IN_PORT, MUX_1_IN_PIN, core::io::pinMode_t::input);
            CORE_IO_CONFIG(MUX_2_IN_PORT, MUX_2_IN_PIN, core::io::pinMode_t::input);

            //bootloader/midi leds
            CORE_IO_CONFIG(LED_IN_PORT, LED_IN_PIN, core::io::pinMode_t::output);
            CORE_IO_CONFIG(LED_OUT_PORT, LED_OUT_PIN, core::io::pinMode_t::output);

            INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
            INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
        }

        void adc()
        {
            core::adc::disconnectDigitalIn(MUX_1_IN_PIN);
            core::adc::disconnectDigitalIn(MUX_2_IN_PIN);

            core::adc::conf_t adcConfiguration;

            adcConfiguration.prescaler = core::adc::prescaler_t::p128;
            adcConfiguration.vref = core::adc::vRef_t::aref;

            core::adc::setup(adcConfiguration);
            core::adc::setChannel(MUX_1_IN_PIN);

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

            //set timer1, timer3 and timer4 to phase correct pwm mode
            //timer 1
            TCCR1A |= (1 << WGM10);    //phase correct PWM
            TCCR1B |= (1 << CS10);     //prescaler 1
            //timer 3
            TCCR3A |= (1 << WGM30);    //phase correct PWM
            TCCR3B |= (1 << CS30);     //prescaler 1
            //timer 4
            TCCR4A |= (1 << PWM4A);    //Pulse Width Modulator A Enable
            TCCR4B |= (1 << CS40);     //prescaler 1
            TCCR4C |= (1 << PWM4D);    //Pulse Width Modulator D Enable
            TCCR4D |= (1 << WGM40);    //phase correct PWM

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
        ATOMIC_SECTION
        {
            for (int i = 0; i < 3; i++)
            {
                if (fwUpdated)
                {
                    INT_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
                    INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
                    _delay_ms(LED_INDICATOR_STARTUP_DELAY);
                    INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
                    INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
                    _delay_ms(LED_INDICATOR_STARTUP_DELAY);
                }
                else
                {
                    INT_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
                    INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
                    _delay_ms(LED_INDICATOR_STARTUP_DELAY);
                    INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
                    INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
                    _delay_ms(LED_INDICATOR_STARTUP_DELAY);
                }
            }
        }

        INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
        INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
    }
}    // namespace Board