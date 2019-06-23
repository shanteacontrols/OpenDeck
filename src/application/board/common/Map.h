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

#pragma once

#include "board/Board.h"

#ifdef __AVR__
#include "core/src/avr/PinManipulation.h"
#endif

namespace Board
{
    namespace map
    {
        ///
        /// \brief Used to retrieve real ADC channel for an given index.
        ///
        uint8_t adcChannel(uint8_t index);

        ///
        /// \brief Used to retrieve real analog multiplexer channel for an given index.
        ///
        uint8_t muxChannel(uint8_t index);

        ///
        /// \brief Used to retrieve real row address in an input matrix for an given index.
        ///
        uint8_t inMatrixRow(uint8_t index);

        ///
        /// \brief Used to retrieve real column address in an input matrix for an given index.
        ///
        uint8_t inMatrixColumn(uint8_t index);

        ///
        /// \brief Used to retrieve button port and pin channel for an given index.
        ///
        core::CORE_ARCH::pins::mcuPin_t button(uint8_t index);

        ///
        /// \brief Used to retrieve LED port and pin channel for an given index.
        ///
        core::CORE_ARCH::pins::mcuPin_t led(uint8_t index);

        ///
        /// \brief Used to retrieve all the registers needed to control PWM channel for an given index.
        ///
        core::CORE_ARCH::pins::pwmChannel_t pwmChannel(uint8_t index);
    }    // namespace map
}    // namespace Board