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

#ifdef __AVR__
#include <util/atomic.h>
#endif
#include "Variables.h"

namespace Board
{
    namespace detail
    {
        volatile uint8_t    digitalInBuffer[DIGITAL_IN_BUFFER_SIZE][DIGITAL_IN_ARRAY_SIZE];
        uint8_t             digitalInBufferReadOnly[DIGITAL_IN_ARRAY_SIZE];

        #ifdef IN_MATRIX
        volatile uint8_t    activeInColumn;
        #endif

        volatile uint8_t    dIn_head;
        volatile uint8_t    dIn_tail;
        volatile uint8_t    dIn_count;
    }

    bool digitalInputDataAvailable()
    {
        using namespace Board::detail;

        if (dIn_count)
        {
            #ifdef __AVR__
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            #endif
            {
                if (++dIn_tail == DIGITAL_IN_BUFFER_SIZE)
                    dIn_tail = 0;

                for (int i=0; i<DIGITAL_IN_ARRAY_SIZE; i++)
                    digitalInBufferReadOnly[i] = digitalInBuffer[dIn_tail][i];

                dIn_count--;
            }

            return true;
        }

        return false;
    }
}