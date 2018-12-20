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

namespace digitalInputCommon
{
    namespace detail
    {
        ///
        /// \brief Used for buttonPCinc/buttonPCdec messages when each button press/encoder rotation sends incremented or decremented PC value.
        /// 16 entries in array are used for 16 MIDI channels.
        ///
        extern uint8_t     lastPCvalue[16];
    }
}