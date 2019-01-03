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

namespace Board
{
    namespace detail
    {
        void initPins()
        {
            //bootloader/midi leds
            setOutput(LED_IN_PORT, LED_IN_PIN);
            setOutput(LED_OUT_PORT, LED_OUT_PIN);

            INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);
            INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
        }

        void configureTimers()
        {
            //set timer0 to ctc, used to show midi tx/rx status using on-board leds
            TCCR0A |= (1<<WGM01);           //CTC mode
            TCCR0B |= (1<<CS01)|(1<<CS00);  //prescaler 64
            OCR0A = 249;                    //1ms
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
}