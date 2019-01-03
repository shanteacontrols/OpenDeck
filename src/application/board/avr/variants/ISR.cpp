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

#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include "board/Board.h"
#include "Common.h"
#include "HardwareControl.cpp"
#include "board/common/analog/input/Variables.h"
#include "board/common/indicators/Variables.h"
#include "core/src/HAL/avr/PinManipulation.h"
#include "core/src/HAL/avr/adc/ADC.h"

///
/// \brief Implementation of core variable used to keep track of run time in milliseconds.
///
volatile uint32_t rTime_ms;

namespace Board
{
    namespace detail
    {
        #ifdef LED_INDICATORS
        volatile uint8_t    midiIn_timeout;
        volatile uint8_t    midiOut_timeout;
        #endif

        volatile bool       USBreceived;
        volatile bool       USBsent;
        volatile bool       UARTreceived;
        volatile bool       UARTsent;
    }
}

///
/// \brief Main interrupt service routine.
/// Used to control I/O on board and to update current run time.
///
ISR(TIMER0_COMPA_vect)
{
    using namespace Board;
    using namespace Board::detail;

    static bool _1ms = true;
    _1ms = !_1ms;

    if (_1ms)
    {
        #ifdef LEDS_SUPPORTED
        checkLEDs();
        #endif

        rTime_ms++;

        #ifdef LED_INDICATORS
        if (USBreceived || UARTreceived)
        {
            INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
            USBreceived = false;
            UARTreceived = false;
            midiIn_timeout = MIDI_INDICATOR_TIMEOUT;
        }

        if (midiIn_timeout)
            midiIn_timeout--;
        else
            INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);

        if (USBsent || UARTsent)
        {
            INT_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
            USBsent = false;
            UARTsent = false;
            midiOut_timeout = MIDI_INDICATOR_TIMEOUT;
        }

        if (midiOut_timeout)
            midiOut_timeout--;
        else
            INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
        #endif
    }

    if (dIn_count < DIGITAL_IN_BUFFER_SIZE)
    {
        if (++dIn_head == DIGITAL_IN_BUFFER_SIZE)
            dIn_head = 0;

        storeDigitalIn();

        dIn_count++;
    }
}

///
/// \brief ADC ISR used to read values from multiplexers.
///
ISR(ADC_vect)
{
    //always ignore first reading
    static bool ignoreFirst = true;

    using namespace Board::detail;

    if (!ignoreFirst)
    {
        analogBuffer[analogIndex] += ADC;
        analogIndex++;
        #ifdef USE_MUX
        activeMuxInput++;

        bool switchMux = (activeMuxInput == NUMBER_OF_MUX_INPUTS);

        if (switchMux)
        #else
        if (analogIndex == MAX_NUMBER_OF_ANALOG)
        #endif
        {
            #ifdef USE_MUX
            activeMuxInput = 0;
            activeMux++;

            if (activeMux == NUMBER_OF_MUX)
            {
                activeMux = 0;
            #endif
                analogIndex = 0;
                analogSampleCounter++;
            #ifdef USE_MUX
            }
            #endif

            #ifdef USE_MUX
            //switch to next mux once all mux inputs are read
            setADCchannel(adcChannelArray[activeMux]);
            #endif
        }

        //always switch to next read pin
        #ifdef USE_MUX
        setMuxInput();
        #else
        setADCchannel(adcChannelArray[analogIndex]);
        #endif
    }

    ignoreFirst = !ignoreFirst;

    if (analogSampleCounter != NUMBER_OF_ANALOG_SAMPLES)
        startADCconversion();
}
