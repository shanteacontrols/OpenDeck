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

#include <limits.h>
#include <errno.h>
#include <inttypes.h>

namespace Util
{
    enum class str2Int_t : uint8_t
    {
        success,
        overflow,
        underflow,
        inconvertible
    };

    str2Int_t str2int(int& result, const char* string, int base = 0);
    void      trimSpace(char* str);
}    // namespace Util
