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

#include "../buffer.h"
#include "../driver_base.h"
#include "count.h"

#include <zephyr/kernel.h>

#include "zlibs/utils/misc/timer.h"

#define OPENDECK_BUTTON_NATIVE_GPIO_ENTRY(index, node_id) GPIO_DT_SPEC_GET_BY_IDX(node_id, native_gpios, index)

namespace io::digital::drivers
{
    class Driver : public DriverBase
    {
        public:
        Driver()
            : _scanTimer(zlibs::utils::misc::Timer::Type::Repeating,
                         [this]()
                         {
                             scan();
                         })
        {}

        bool init() override
        {
            for (size_t i = 0; i < _pins.size(); i++)
            {
                if (!gpio_is_ready_dt(&_pins[i]))
                {
                    return false;
                }

                if (gpio_pin_configure_dt(&_pins[i], INPUT_FLAGS) != 0)
                {
                    return false;
                }
            }

            _buffer.clear();
            scan();
            _scanTimer.start(SCAN_PERIOD_MS);

            return true;
        }

        std::optional<Frame> read() override
        {
            return _buffer.pop();
        }

        size_t encoderCount() const override
        {
            return ENCODER_COUNT;
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

        private:
        static constexpr size_t   BUTTON_COUNT   = OPENDECK_BUTTON_COUNT;
        static constexpr size_t   ENCODER_COUNT  = BUTTON_COUNT / 2;
        static constexpr uint32_t SCAN_PERIOD_MS = 1;
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_buttons), ext_pullups)
        static constexpr gpio_flags_t INPUT_FLAGS = GPIO_INPUT;
#else
        static constexpr gpio_flags_t INPUT_FLAGS = GPIO_INPUT | GPIO_PULL_UP;
#endif

        std::array<gpio_dt_spec, BUTTON_COUNT> _pins   = { { LISTIFY(OPENDECK_BUTTON_COUNT, OPENDECK_BUTTON_NATIVE_GPIO_ENTRY, (, ), DT_CHOSEN(opendeck_buttons)) } };
        Buffer<Frame>                          _buffer = {};
        zlibs::utils::misc::Timer              _scanTimer;

        bool readPin(size_t index) const
        {
            return gpio_pin_get_dt(&_pins[index]) > 0;
        }

        void scan()
        {
            Frame frame = {};

            for (size_t i = 0; i < BUTTON_COUNT; i++)
            {
                frame[i] = readPin(i);
            }

            _buffer.push(frame);
        }
    };
}    // namespace io::digital::drivers

#undef OPENDECK_BUTTON_NATIVE_GPIO_ENTRY
