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
#include <functional>
#include <stddef.h>

namespace util
{
    template<typename T, T minValue, T maxValue>
    class IncDec
    {
        public:
        enum class type_t : uint8_t
        {
            EDGE,
            OVERFLOW,
        };

        IncDec() = delete;

        static T increment(T value, T steps, type_t type)
        {
            if ((maxValue - value) < steps)
            {
                if (type == type_t::EDGE)
                {
                    return maxValue;
                }

                // handle overflow
                return (value + (steps % maxValue)) % (maxValue + 1);
            }

            return value + steps;
        }

        static T decrement(T value, T steps, type_t type)
        {
            if (value < steps)
            {
                if (type == type_t::EDGE)
                {
                    return minValue;
                }

                // handle overflow
                return (value + maxValue + 1 - (steps % maxValue)) % (maxValue + 1);
            }

            return value - steps;
        }
    };
}    // namespace util