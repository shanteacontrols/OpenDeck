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
#include "core/src/avr/PinManipulation.h"
#include "core/src/avr/ADC.h"
#include "board/common/constants/LEDs.h"

namespace Board
{
    namespace setup
    {
        void pins()
        {
            //configure input matrix
            //shift register
            setInput(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN);
            setOutput(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
            setOutput(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);

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
            EXT_LED_OFF(LED_ROW_1_PORT, LED_ROW_1_PIN);
            EXT_LED_OFF(LED_ROW_2_PORT, LED_ROW_2_PIN);
            EXT_LED_OFF(LED_ROW_3_PORT, LED_ROW_3_PIN);
            EXT_LED_OFF(LED_ROW_4_PORT, LED_ROW_4_PIN);
            EXT_LED_OFF(LED_ROW_5_PORT, LED_ROW_5_PIN);
            EXT_LED_OFF(LED_ROW_6_PORT, LED_ROW_6_PIN);

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
            setInput(MUX_2_IN_PORT, MUX_2_IN_PIN);

            //bootloader/midi leds
            setOutput(LED_IN_PORT, LED_IN_PIN);
            setOutput(LED_OUT_PORT, LED_OUT_PIN);

            INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
            INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
        }

        void adc()
        {
            using namespace core::avr;

            adc::disconnectDigitalIn(MUX_1_IN_PIN);
            adc::disconnectDigitalIn(MUX_2_IN_PIN);

            adc::conf_t adcConfiguration;

            adcConfiguration.prescaler = adc::prescaler_t::p128;
            adcConfiguration.vref = adc::vRef_t::aref;

            adc::setup(adcConfiguration);
            adc::setChannel(MUX_1_IN_PIN);

            for (int i=0; i<3; i++)
                adc::read();  //few dummy reads to init ADC

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

            //set timer1, timer3 and timer4 to phase correct pwm mode
            //timer 1
            TCCR1A |= (1<<WGM10);           //phase correct PWM
            TCCR1B |= (1<<CS10);            //prescaler 1
            //timer 3
            TCCR3A |= (1<<WGM30);           //phase correct PWM
            TCCR3B |= (1<<CS30);            //prescaler 1
            //timer 4
            TCCR4A |= (1<<PWM4A);           //Pulse Width Modulator A Enable
            TCCR4B |= (1<<CS40);            //prescaler 1
            TCCR4C |= (1<<PWM4D);           //Pulse Width Modulator D Enable
            TCCR4D |= (1<<WGM40);           //phase correct PWM

            //set timer0 to ctc, used for millis/led matrix
            TCCR0A |= (1<<WGM01);           //CTC mode
            TCCR0B |= (1<<CS01)|(1<<CS00);  //prescaler 64
            OCR0A = 124;                    //500us
            TIMSK0 |= (1<<OCIE0A);          //compare match interrupt
        }
    }

    void ledFlashStartup(bool fwUpdated)
    {
        //block interrupts here to avoid received midi traffic messing with indicator leds animation
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            for (int i=0; i<3; i++)
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
}