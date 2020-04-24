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

namespace Interface
{
    namespace digital
    {
        namespace input
        {
            class Common
            {
                public:
                Common() = default;

                bool    pcIncrement(uint8_t channel);
                bool    pcDecrement(uint8_t channel);
                uint8_t program(uint8_t channel);
                bool    setProgram(uint8_t channel, uint8_t program);

                protected:
                ///
                /// \brief Holds current program change value for all 16 MIDI channels.
                ///
                static uint8_t pcValue[16];
            };
        }    // namespace input
    }        // namespace digital
}    // namespace Interface