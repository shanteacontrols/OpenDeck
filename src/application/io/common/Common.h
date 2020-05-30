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

#pragma once

#include <inttypes.h>
#include <stddef.h>

namespace IO
{
    class Common
    {
        public:
        Common() = default;

        enum class incDecType_t : uint8_t
        {
            toEdge,
            reset,
        };

        bool    pcIncrement(uint8_t channel);
        bool    pcDecrement(uint8_t channel);
        uint8_t program(uint8_t channel);
        bool    setProgram(uint8_t channel, uint8_t program);
        uint8_t valueInc(size_t index, uint8_t step, incDecType_t type);
        uint8_t valueIncDec(size_t index, uint8_t step);
        uint8_t currentValue(size_t index);
        void    resetValue(size_t index);

        protected:
        ///
        /// \brief Holds current program change value for all 16 MIDI channels.
        ///
        static uint8_t pcValue[16];

        ///
        /// \brief Holds current MIDI value (velocity or control value) for all buttons.
        /// Used when button is configured in any of the multi-value modes.
        ///
        static uint8_t midiValue[MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS];
    };
}    // namespace IO