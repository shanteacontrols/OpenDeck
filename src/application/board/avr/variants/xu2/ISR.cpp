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
#include "../Common.h"
#include "pins/Pins.h"
#include "board/common/constants/LEDs.h"
#include "board/common/indicators/Variables.h"

volatile uint32_t rTime_ms;

namespace Board
{
    namespace detail
    {
        volatile uint8_t midiIn_timeout, midiOut_timeout;
        volatile bool USBreceived, USBsent, UARTreceived, UARTsent;
    }
}

ISR(TIMER0_COMPA_vect)
{
    using namespace Board::detail;

    if (USBreceived)
    {
        INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
        USBreceived = false;
        midiIn_timeout = MIDI_INDICATOR_TIMEOUT;
    }

    if (USBsent)
    {
        INT_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
        USBsent = false;
        midiOut_timeout = MIDI_INDICATOR_TIMEOUT;
    }

    if (midiIn_timeout)
        midiIn_timeout--;
    else
        INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);

    if (midiOut_timeout)
        midiOut_timeout--;
    else
        INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);

    rTime_ms++;
}