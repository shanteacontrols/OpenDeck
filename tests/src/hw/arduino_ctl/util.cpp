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

#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

namespace Util
{
    str2Int_t str2int(int& result, const char* string, int base)
    {
        char* end;
        errno  = 0;
        result = 0;

        auto converted = strtol(string, &end, base);

        if ((errno == ERANGE && converted == LONG_MAX) || converted > INT_MAX)
        {
            return str2Int_t::overflow;
        }

        if ((errno == ERANGE && converted == LONG_MIN) || converted < INT_MIN)
        {
            return str2Int_t::underflow;
        }

        if (*string == '\0' || *end != '\0')
        {
            return str2Int_t::inconvertible;
        }

        result = converted;
        return str2Int_t::success;
    }

    void trimSpace(char* str)
    {
        char* dst = str;

        for (; *str; ++str)
        {
            *dst++ = *str;

            if (isspace(*str))
            {
                do
                {
                    ++str;
                } while (isspace(*str));

                --str;
            }
        }

        *dst = 0;
    }
}    // namespace Util
