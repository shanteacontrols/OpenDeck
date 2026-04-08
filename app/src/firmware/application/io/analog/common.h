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
#include "application/database/database.h"

namespace io::analog
{
    class Collection : public io::common::BaseCollection<PROJECT_TARGET_SUPPORTED_NR_OF_ANALOG_INPUTS>
    {
        public:
        Collection() = delete;
    };

    enum
    {
        GROUP_ANALOG_INPUTS,
        GROUP_TOUCHSCREEN_COMPONENTS
    };

    enum class type_t : uint8_t
    {
        POTENTIOMETER_CONTROL_CHANGE,
        POTENTIOMETER_NOTE,
        FSR,
        BUTTON,
        NRPN_7BIT,
        NRPN_14BIT,
        PITCH_BEND,
        CONTROL_CHANGE_14BIT,
        RESERVED,
        AMOUNT
    };

    enum class pressureType_t : uint8_t
    {
        VELOCITY,
        AFTERTOUCH
    };
}    // namespace io::analog