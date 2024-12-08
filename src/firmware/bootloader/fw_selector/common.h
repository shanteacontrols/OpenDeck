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

namespace fw_selector
{
    /// List of all possible firmwares which can be loaded and their magic values.
    /// Bootloader will load a firmware based on read value.
    /// Location of this value is board-specific.
    enum class fwType_t : uint32_t
    {
        APPLICATION = 0xFFFFFFFF,
        BOOTLOADER  = 0x47474747
    };
}    // namespace fw_selector
