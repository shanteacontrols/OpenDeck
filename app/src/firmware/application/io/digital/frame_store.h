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

#include "drivers/driver_base.h"

#include <optional>

namespace io::digital
{
    class FrameStore
    {
        public:
        explicit FrameStore(drivers::DriverBase& driver)
            : _driver(driver)
        {}

        void setFrame(const drivers::Frame& frame)
        {
            _frame      = frame;
            _frameValid = true;
        }

        void clear()
        {
            _frameValid = false;
        }

        std::optional<bool> state(size_t index) const
        {
            if (!_frameValid || (index >= _frame.size()))
            {
                return {};
            }

            return _frame[index];
        }

        std::optional<uint8_t> encoderState(size_t index) const
        {
            const size_t encoderCount = _driver.encoderCount();

            if (!_frameValid || (encoderCount == 0) || (index >= encoderCount))
            {
                return {};
            }

            const size_t componentA = _driver.encoderComponentFromEncoder(index, drivers::encoderComponent_t::A);
            const size_t componentB = _driver.encoderComponentFromEncoder(index, drivers::encoderComponent_t::B);

            if ((componentA >= _frame.size()) || (componentB >= _frame.size()))
            {
                return {};
            }

            return static_cast<uint8_t>((_frame[componentA] ? 0x02U : 0x00U) |
                                        (_frame[componentB] ? 0x01U : 0x00U));
        }

        size_t buttonToEncoderIndex(size_t index) const
        {
            return _driver.buttonToEncoderIndex(index);
        }

        private:
        drivers::DriverBase& _driver;
        drivers::Frame       _frame      = {};
        bool                 _frameValid = false;
    };
}    // namespace io::digital
