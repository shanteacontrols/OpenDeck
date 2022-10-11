/*

Copyright 2015-2022 Igor Petrovic

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
#include <stddef.h>

namespace IO::Common
{
    enum class initAction_t : uint8_t
    {
        AS_IS,
        INIT,
        DE_INIT
    };

    template<size_t... T>
    class BaseCollection
    {
        public:
        BaseCollection() = delete;

        static constexpr size_t groups()
        {
            return sizeof...(T);
        }

        static constexpr size_t size()
        {
            return (T + ...);
        }

        static constexpr size_t size(size_t group)
        {
            constexpr size_t VALUES[] = { T... };
            return VALUES[group];
        }

        static constexpr size_t startIndex(size_t group)
        {
            size_t index = 0;

            for (size_t i = 0; i < group; i++)
            {
                index += size(i);
            }

            return index;
        }
    };

    class Allocatable
    {
        public:
        enum class interface_t : uint8_t
        {
            UART,
            CDC
        };

        virtual ~Allocatable() = default;

        virtual bool allocated(interface_t interface) = 0;
    };
}    // namespace IO::Common