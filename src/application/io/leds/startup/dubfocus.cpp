/*

Copyright 2015-2020 Igor Petrovic

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

#include "io/leds/LEDs.h"
#include "core/src/general/Timing.h"

using namespace IO;

#define CONNECTED_LEDS 12

namespace
{
    const uint8_t ledMapArray[CONNECTED_LEDS] = {
        3,
        0,
        1,
        2,
        14,
        12,
        13,
        15,
        6,
        4,
        5,
        7
    };
}    // namespace

void LEDs::startUpAnimation()
{
    //turn all leds on first
    setAllOn();

    core::timing::waitMs(1000);

    for (int i = 0; i < CONNECTED_LEDS; i++)
    {
        updateState(ledMapArray[i], ledBit_t::active, false);
        updateState(ledMapArray[i], ledBit_t::state, false);

        core::timing::waitMs(35);
    }

    for (int i = 0; i < CONNECTED_LEDS; i++)
    {
        updateState(ledMapArray[CONNECTED_LEDS - 1 - i], ledBit_t::active, true);
        updateState(ledMapArray[CONNECTED_LEDS - 1 - i], ledBit_t::state, true);

        core::timing::waitMs(35);
    }

    for (int i = 0; i < CONNECTED_LEDS; i++)
    {
        updateState(ledMapArray[CONNECTED_LEDS - 1 - i], ledBit_t::active, false);
        updateState(ledMapArray[CONNECTED_LEDS - 1 - i], ledBit_t::state, false);

        core::timing::waitMs(35);
    }

    //turn all off again
    setAllOff();
}