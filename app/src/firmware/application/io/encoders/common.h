/*

Copyright Igor Petrovic

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

#include "application/io/common/common.h"

namespace io::encoders
{
    class Collection : public io::common::BaseCollection<PROJECT_TARGET_SUPPORTED_NR_OF_ENCODERS>
    {
        public:
        Collection() = delete;
    };

    enum class type_t : uint8_t
    {
        CONTROL_CHANGE_7FH01H,
        CONTROL_CHANGE_3FH41H,
        PROGRAM_CHANGE,
        CONTROL_CHANGE,
        PRESET_CHANGE,
        PITCH_BEND,
        NRPN_7BIT,
        NRPN_14BIT,
        CONTROL_CHANGE_14BIT,
        CONTROL_CHANGE_41H01H,
        BPM_CHANGE,
        SINGLE_NOTE_VARIABLE_VAL,
        SINGLE_NOTE_FIXED_VAL_BOTH_DIR,
        SINGLE_NOTE_FIXED_VAL_ONE_DIR_0_OTHER_DIR,
        TWO_NOTE_FIXED_VAL_BOTH_DIR,
        AMOUNT
    };

    enum class position_t : uint8_t
    {
        STOPPED,
        CCW,
        CW,
    };

    enum class acceleration_t : uint8_t
    {
        DISABLED,
        SLOW,
        MEDIUM,
        FAST,
        AMOUNT
    };
}    // namespace io::encoders