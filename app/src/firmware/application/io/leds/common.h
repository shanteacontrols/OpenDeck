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

namespace io::leds
{
    class Collection : public io::common::BaseCollection<PROJECT_TARGET_SUPPORTED_NR_OF_DIGITAL_OUTPUTS,
                                                         PROJECT_TARGET_SUPPORTED_NR_OF_TOUCHSCREEN_COMPONENTS>
    {
        public:
        Collection() = delete;
    };

    enum
    {
        GROUP_DIGITAL_OUTPUTS,
        GROUP_TOUCHSCREEN_COMPONENTS
    };

    enum class rgbComponent_t : uint8_t
    {
        R,
        G,
        B
    };

    enum class color_t : uint8_t
    {
        OFF,
        RED,
        GREEN,
        YELLOW,
        BLUE,
        MAGENTA,
        CYAN,
        WHITE,
        AMOUNT
    };

    enum class setting_t : uint8_t
    {
        BLINK_WITH_MIDI_CLOCK,
        UNUSED,
        USE_STARTUP_ANIMATION,
        USE_MIDI_PROGRAM_OFFSET,
        AMOUNT
    };

    enum class controlType_t : uint8_t
    {
        MIDI_IN_NOTE_SINGLE_VAL,
        LOCAL_NOTE_SINGLE_VAL,
        MIDI_IN_CC_SINGLE_VAL,
        LOCAL_CC_SINGLE_VAL,
        PC_SINGLE_VAL,
        PRESET,
        MIDI_IN_NOTE_MULTI_VAL,
        LOCAL_NOTE_MULTI_VAL,
        MIDI_IN_CC_MULTI_VAL,
        LOCAL_CC_MULTI_VAL,
        STATIC,
        AMOUNT
    };

    enum class blinkSpeed_t : uint8_t
    {
        S1000MS,
        S500MS,
        S250MS,
        NO_BLINK
    };

    enum class blinkType_t : uint8_t
    {
        TIMER,
        MIDI_CLOCK
    };

    enum class brightness_t : uint8_t
    {
        OFF,
        B25,
        B50,
        B75,
        B100
    };
}    // namespace io::leds