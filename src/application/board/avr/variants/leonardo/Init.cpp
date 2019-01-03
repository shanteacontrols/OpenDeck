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

#include <util/atomic.h>
#include <util/delay.h>
#include "board/Board.h"
#include "pins/Pins.h"
#include "board/common/constants/LEDs.h"
#include "core/src/HAL/avr/PinManipulation.h"
#include "core/src/HAL/avr/adc/ADC.h"

namespace Board
{
    namespace detail
    {
        void initPins()
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

            #ifdef BOARD_A_LEO
            setInput(DI_7_PORT, DI_7_PIN);
            setHigh(DI_7_PORT, DI_7_PIN);

            setInput(DI_8_PORT, DI_8_PIN);
            setHigh(DI_8_PORT, DI_8_PIN);
            #endif

            setOutput(DO_1_PORT, DO_1_PIN);
            EXT_LED_OFF(DO_1_PORT, DO_1_PIN);

            setOutput(DO_2_PORT, DO_2_PIN);
            EXT_LED_OFF(DO_2_PORT, DO_2_PIN);

            setOutput(DO_3_PORT, DO_3_PIN);
            EXT_LED_OFF(DO_3_PORT, DO_3_PIN);

            setOutput(DO_4_PORT, DO_4_PIN);
            EXT_LED_OFF(DO_4_PORT, DO_4_PIN);

            setOutput(DO_5_PORT, DO_5_PIN);
            EXT_LED_OFF(DO_5_PORT, DO_5_PIN);

            setOutput(DO_6_PORT, DO_6_PIN);
            EXT_LED_OFF(DO_6_PORT, DO_6_PIN);


            setInput(AI_1_PORT, AI_1_PIN);
            setLow(AI_1_PORT, AI_1_PIN);

            setInput(AI_2_PORT, AI_2_PIN);
            setLow(AI_2_PORT, AI_2_PIN);

            setInput(AI_3_PORT, AI_3_PIN);
            setLow(AI_3_PORT, AI_3_PIN);

            setInput(AI_4_PORT, AI_4_PIN);
            setLow(AI_4_PORT, AI_4_PIN);

            #ifdef BOARD_A_LEO
            setInput(AI_5_PORT, AI_5_PIN);
            setLow(AI_5_PORT, AI_5_PIN);

            setInput(AI_6_PORT, AI_6_PIN);
            setLow(AI_6_PORT, AI_6_PIN);
            #endif

            //bootloader/midi leds
            setOutput(LED_IN_PORT, LED_IN_PIN);
            setOutput(LED_OUT_PORT, LED_OUT_PIN);

            INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
            INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
        }

        void initAnalog()
        {
            adcConf adcConfiguration;

            adcConfiguration.prescaler = ADC_PRESCALER_128;
            adcConfiguration.vref = ADC_VREF_AVCC;

            setUpADC(adcConfiguration);
            setADCchannel(AI_1_PIN);

            for (int i=0; i<3; i++)
                getADCvalue();  //few dummy reads to init ADC

            adcInterruptEnable();
            startADCconversion();
        }

        void configureTimers()
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
    }

    bool startUpAnimation()
    {
        return false;
    }
}