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

#include "core/src/general/RingBuffer.h"

namespace Board
{
    namespace detail
    {
        ///
        /// \brief Buffer in which outgoing UART data is stored.
        ///
        extern RingBuff_t   txBuffer[UART_INTERFACES];

        ///
        /// \brief Buffer in which incoming UART data is stored.
        ///
        extern RingBuff_t   rxBuffer[UART_INTERFACES];
    }
}