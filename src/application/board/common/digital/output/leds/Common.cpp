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
#include "board/common/digital/output/Variables.h"
#include "board/common/constants/LEDs.h"

namespace Board
{
    namespace detail
    {
        volatile uint8_t    pwmSteps;
        volatile int8_t     transitionCounter[MAX_NUMBER_OF_LEDS];
    }

    ///
    /// \brief Array holding current LED status for all LEDs.
    ///
    uint8_t             ledState[MAX_NUMBER_OF_LEDS];

    bool setLEDfadeSpeed(uint8_t transitionSpeed)
    {
        using namespace Board::detail;

        if (transitionSpeed > FADE_TIME_MAX)
        {
            return false;
        }

        //reset transition counter
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
                transitionCounter[i] = 0;

            pwmSteps = transitionSpeed;
        }

        return true;
    }
}