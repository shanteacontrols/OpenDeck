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

                protected:
                ///
                /// \brief Used for Buttons::messageType_t::programChangeInc/Buttons::messageType_t::programChangeDec messages when each button press/encoder rotation sends incremented or decremented PC value.
                /// 16 entries in array are used for 16 MIDI channels.
                ///
                static int8_t lastPCvalue[16];
            };
        }    // namespace input
    }        // namespace digital
}    // namespace Interface