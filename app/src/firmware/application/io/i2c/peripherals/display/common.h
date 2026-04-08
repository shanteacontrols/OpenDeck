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

namespace io::i2c::display
{
    enum class setting_t : uint8_t
    {
        DEVICE_INFO_MSG,
        CONTROLLER,
        RESOLUTION,
        EVENT_TIME,
        MIDI_NOTES_ALTERNATE,
        OCTAVE_NORMALIZATION,
        ENABLE,
        AMOUNT
    };

    enum class displayController_t : uint8_t
    {
        INVALID,
        SSD1306,
        AMOUNT
    };

    enum displayResolution_t : uint8_t
    {
        INVALID,
        R128X64,
        R128X32,
        AMOUNT
    };
}    // namespace io::i2c::display