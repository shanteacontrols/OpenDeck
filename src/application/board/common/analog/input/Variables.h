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

#include <inttypes.h>

namespace Board
{
    namespace detail
    {
        #ifdef USE_MUX
        ///
        /// \brief Holds currently active multiplexer.
        ///
        extern uint8_t              activeMux;

        ///
        /// \brief Holds currently active multiplexer input.
        ///
        extern uint8_t              activeMuxInput;
        #endif

        ///
        /// \brief Holds currently active analog index which is being read.
        /// Once all analog inputs are read, analog index is reset to 0.
        ///
        extern uint8_t              analogIndex;

        ///
        /// brief Holds currently active sample count.
        /// Once all analog inputs are read, sample count is increased.
        ///
        extern volatile uint8_t     analogSampleCounter;

        ///
        /// \brief Array in which analog samples are stored.
        ///
        extern volatile int16_t     analogBuffer[MAX_NUMBER_OF_ANALOG];
    }
}