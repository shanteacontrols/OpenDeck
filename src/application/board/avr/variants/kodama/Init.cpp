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
#include "board/Board.h"
#include "pins/Map.h"
#include "interface/digital/output/leds/Constants.h"
#include "board/common/digital/output/Variables.h"
#include "board/common/constants/LEDs.h"
#include "core/src/general/BitManipulation.h"
#include "core/src/general/Timing.h"
#include "core/src/HAL/avr/PinManipulation.h"
#include "core/src/HAL/avr/adc/ADC.h"

namespace Board
{
    namespace detail
    {
        void initPins()
        {
            setInput(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN);
            setOutput(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
            setOutput(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);

            setOutput(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
            setOutput(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
            setOutput(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

            setOutput(MUX_S0_PORT, MUX_S0_PIN);
            setOutput(MUX_S1_PORT, MUX_S1_PIN);
            setOutput(MUX_S2_PORT, MUX_S2_PIN);
            setOutput(MUX_S3_PORT, MUX_S3_PIN);

            setInput(MUX_1_IN_PORT, MUX_1_IN_PIN);
            setInput(MUX_2_IN_PORT, MUX_2_IN_PIN);
            setInput(MUX_3_IN_PORT, MUX_3_IN_PIN);
            setInput(MUX_4_IN_PORT, MUX_4_IN_PIN);

            //unused pins
            setOutput(PORTB, 0);
            setOutput(PORTB, 4);
            setOutput(PORTB, 5);
            setOutput(PORTB, 7);

            setOutput(PORTC, 6);
            setOutput(PORTC, 7);

            setOutput(PORTD, 6);
            setOutput(PORTD, 7);

            setOutput(PORTE, 2);

            setOutput(PORTF, 0);
            setOutput(PORTF, 1);
            setOutput(PORTF, 4);

            //make sure all unused pins are logic low
            setLow(PORTB, 0);
            setLow(PORTB, 4);
            setLow(PORTB, 5);
            setLow(PORTB, 7);

            setLow(PORTC, 6);
            setLow(PORTC, 7);

            setLow(PORTD, 6);
            setLow(PORTD, 7);

            setLow(PORTE, 2);

            setLow(PORTF, 0);
            setLow(PORTF, 1);
            setLow(PORTF, 4);

            //init all outputs on shift register
            setLow(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

            for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            {
                EXT_LED_OFF(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
                pulseHighToLow(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
            }

            setHigh(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);
        }

        void initAnalog()
        {
            disconnectDigitalInADC(adcChannelArray[0]);
            disconnectDigitalInADC(adcChannelArray[1]);
            disconnectDigitalInADC(adcChannelArray[2]);
            disconnectDigitalInADC(adcChannelArray[3]);

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
        
    }

    bool startUpAnimation()
    {
        using namespace Board::detail;

        //turn all leds on first
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        {
            BIT_SET(ledState[i], LED_ACTIVE_BIT);
            BIT_SET(ledState[i], LED_STATE_BIT);
        }

        wait_ms(1000);

        for (int i=0; i<12; i++)
        {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                BIT_CLEAR(ledState[ledMapArray[i]], LED_ACTIVE_BIT);
                BIT_CLEAR(ledState[ledMapArray[i]], LED_STATE_BIT);
            }

            wait_ms(35);
        }

        for (int i=0; i<12; i++)
        {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                BIT_SET(ledState[ledMapArray[11-i]], LED_ACTIVE_BIT);
                BIT_SET(ledState[ledMapArray[11-i]], LED_STATE_BIT);
            }

            wait_ms(35);
        }

        for (int i=0; i<12; i++)
        {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                BIT_CLEAR(ledState[ledMapArray[11-i]], LED_ACTIVE_BIT);
                BIT_CLEAR(ledState[ledMapArray[11-i]], LED_STATE_BIT);
            }

            wait_ms(35);
        }

        //turn all off again
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        {
            BIT_CLEAR(ledState[i], LED_ACTIVE_BIT);
            BIT_CLEAR(ledState[i], LED_STATE_BIT);
        }

        return true;
    }
}