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

#include "count.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace io::digital::drivers
{
    enum class encoderComponent_t : uint8_t
    {
        A,
        B,
    };

    using Frame = std::array<bool, OPENDECK_DIGITAL_INPUT_COUNT>;

    class DriverBase
    {
        public:
        virtual ~DriverBase() = default;

        virtual bool                 init()                                                          = 0;
        virtual std::optional<Frame> read()                                                          = 0;
        virtual size_t               encoderCount() const                                            = 0;
        virtual size_t               buttonToEncoderIndex(size_t index)                              = 0;
        virtual size_t               encoderComponentFromEncoder(size_t index, encoderComponent_t c) = 0;
    };
}    // namespace io::digital::drivers
