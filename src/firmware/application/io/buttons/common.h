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

#include <inttypes.h>

namespace io::buttons
{
    enum
    {
        GROUP_DIGITAL_INPUTS,
        GROUP_ANALOG_INPUTS,
        GROUP_TOUCHSCREEN_COMPONENTS
    };

    enum class type_t : uint8_t
    {
        MOMENTARY,    ///< Event on press and release.
        LATCHING,     ///< Event between presses only.
        AMOUNT        ///< Total number of button types.
    };

    enum class messageType_t : uint8_t
    {
        NOTE,
        PROGRAM_CHANGE,
        CONTROL_CHANGE,
        CONTROL_CHANGE_RESET,
        MMC_STOP,
        MMC_PLAY,
        MMC_RECORD,
        MMC_PAUSE,
        REAL_TIME_CLOCK,
        REAL_TIME_START,
        REAL_TIME_CONTINUE,
        REAL_TIME_STOP,
        REAL_TIME_ACTIVE_SENSING,
        REAL_TIME_SYSTEM_RESET,
        PROGRAM_CHANGE_INC,
        PROGRAM_CHANGE_DEC,
        NONE,
        PRESET_CHANGE,
        MULTI_VAL_INC_RESET_NOTE,
        MULTI_VAL_INC_DEC_NOTE,
        MULTI_VAL_INC_RESET_CC,
        MULTI_VAL_INC_DEC_CC,
        NOTE_OFF_ONLY,
        CONTROL_CHANGE0_ONLY,
        RESERVED,
        PROGRAM_CHANGE_OFFSET_INC,
        PROGRAM_CHANGE_OFFSET_DEC,
        BPM_INC,
        BPM_DEC,
        AMOUNT
    };
}    // namespace io::buttons