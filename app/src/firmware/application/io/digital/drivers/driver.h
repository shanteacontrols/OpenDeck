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

#include "driver_base.h"

#if defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_NATIVE)
#include "native/native_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_SHIFT_REGISTER)
#include "shift_register/shift_register_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_MATRIX_NATIVE_ROWS)
#include "matrix_native_rows/matrix_native_rows_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_MATRIX_SHIFT_REGISTER_ROWS)
#include "matrix_shift_register_rows/matrix_shift_register_rows_driver.h"
#else
namespace io::digital::drivers
{
    class Driver : public DriverBase
    {
        public:
        bool init() override
        {
            return true;
        }

        std::optional<Frame> read() override
        {
            return {};
        }

        size_t encoderCount() const override
        {
            return OPENDECK_ENCODER_COUNT;
        }

        size_t buttonToEncoderIndex(size_t index) override
        {
            return index / 2;
        }

        size_t encoderComponentFromEncoder(size_t index, encoderComponent_t component) override
        {
            index *= 2;
            return component == encoderComponent_t::A ? index : index + 1;
        }
    };
}    // namespace io::digital::drivers
#endif
