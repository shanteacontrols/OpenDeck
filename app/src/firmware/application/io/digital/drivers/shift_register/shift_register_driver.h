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
            if (!gpio_is_ready_dt(&_data) || !gpio_is_ready_dt(&_clock) || !gpio_is_ready_dt(&_latch))
            {
                return false;
            }

            if ((gpio_pin_configure_dt(&_data, GPIO_INPUT) != 0) ||
                (gpio_pin_configure_dt(&_clock, GPIO_OUTPUT_INACTIVE) != 0) ||
                (gpio_pin_configure_dt(&_latch, GPIO_OUTPUT_ACTIVE) != 0))
            {
                return false;
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
        gpio_dt_spec              _data          = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_buttons), data_gpios);
        gpio_dt_spec              _clock         = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_buttons), clock_gpios);
        gpio_dt_spec              _latch         = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_buttons), latch_gpios);
        Buffer<Frame>             _buffer        = {};
        zlibs::utils::misc::Timer _scanTimer;

        void scan()
        {
            Frame frame = {};

            gpio_pin_set_dt(&_clock, 0);
            gpio_pin_set_dt(&_latch, 0);
            k_busy_wait(1);
            gpio_pin_set_dt(&_latch, 1);

            for (size_t shiftRegister = 0; shiftRegister < DT_PROP(DT_CHOSEN(opendeck_buttons), shift_registers); shiftRegister++)
            {
                for (size_t input = 0; input < 8; input++)
                {
                    const size_t index = (shiftRegister * 8) + (7 - input);

                    gpio_pin_set_dt(&_clock, 0);
                    k_busy_wait(1);
                    frame[index] = gpio_pin_get_dt(&_data) > 0;
                    gpio_pin_set_dt(&_clock, 1);
                }
            }

            _buffer.push(frame);
        }
    };
}    // namespace io::digital::drivers
