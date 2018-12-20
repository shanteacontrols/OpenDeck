/*

Copyright 2015-2018 Igor Petrovic

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

#pragma once

#include <inttypes.h>
#include "board/common/constants/DigitalIn.h"

namespace Board
{
    namespace detail
    {
        ///
        /// \brief Array used to store readings from digital input matrix.
        ///
        extern volatile uint8_t     digitalInBuffer[DIGITAL_IN_BUFFER_SIZE][DIGITAL_IN_ARRAY_SIZE];

        ///
        /// \brief Read only copy of digital input buffer.
        /// Used to avoid data overwrite from ISR.
        ///
        extern uint8_t              digitalInBufferReadOnly[DIGITAL_IN_ARRAY_SIZE];

        #ifdef IN_MATRIX
        ///
        /// \brief Holds value of currently active input matrix column.
        ///
        extern volatile uint8_t     activeInColumn;
        #endif

        ///
        /// \brief Holds "head" index position in ring buffer.
        ///
        extern volatile uint8_t     dIn_head;

        ///
        /// \brief Holds "tail" index position in ring buffer.
        ///
        extern volatile uint8_t     dIn_tail;

        ///
        /// \brief Holds current number of elements stored in ring buffer.
        ///
        extern volatile uint8_t     dIn_count;
    }
}