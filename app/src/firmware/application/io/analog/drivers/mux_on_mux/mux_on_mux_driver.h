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

#include "../driver_base.h"
#include "count.h"

#include <zephyr/kernel.h>

namespace io::analog
{
    class Driver : public DriverBase
    {
        public:
        bool init() override
        {
            if (!device_is_ready(_channel.dev))
            {
                return false;
            }

            if (!configureSelectorPin(_controllerS0) ||
                !configureSelectorPin(_controllerS1) ||
                !configureSelectorPin(_nodeS0) ||
                !configureSelectorPin(_nodeS1) ||
                !configureSelectorPin(_nodeS2) ||
                !configureSelectorPin(_nodeS3))
            {
                return false;
            }

#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), controller_s2_gpios)
            if (!configureSelectorPin(_controllerS2))
            {
                return false;
            }
#endif

#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), controller_s3_gpios)
            if (!configureSelectorPin(_controllerS3))
            {
                return false;
            }
#endif

            if (!adc_is_ready_dt(&_channel))
            {
                return false;
            }

            if (adc_channel_setup_dt(&_channel) != 0)
            {
                return false;
            }

            selectController(0);
            selectNodeInput(0);
            return true;
        }

        std::optional<uint16_t> value(size_t index) override
        {
            if (index >= (muxCount() * INPUTS_PER_MUX))
            {
                return {};
            }

            const size_t muxIndex   = index / INPUTS_PER_MUX;
            const size_t inputIndex = index % INPUTS_PER_MUX;

            selectController(muxIndex);
            selectNodeInput(inputIndex);
            k_busy_wait(SETTLE_TIME_US);

            int16_t sample = 0;

            adc_sequence sequence = {};
            sequence.buffer       = &sample;
            sequence.buffer_size  = sizeof(sample);

            if (adc_sequence_init_dt(&_channel, &sequence) != 0)
            {
                return {};
            }

            if (adc_read_dt(&_channel, &sequence) != 0)
            {
                return {};
            }

            return static_cast<uint16_t>(sample);
        }

        uint8_t adcBits() const override
        {
            return 12;
        }

        private:
        static constexpr uint8_t  INPUTS_PER_MUX = 16;
        static constexpr uint32_t SETTLE_TIME_US = 10;

        const gpio_dt_spec _controllerS0 = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_analog), controller_s0_gpios);
        const gpio_dt_spec _controllerS1 = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_analog), controller_s1_gpios);
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), controller_s2_gpios)
        const gpio_dt_spec _controllerS2 = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_analog), controller_s2_gpios);
#endif
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), controller_s3_gpios)
        const gpio_dt_spec _controllerS3 = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_analog), controller_s3_gpios);
#endif
        const gpio_dt_spec _nodeS0  = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_analog), node_s0_gpios);
        const gpio_dt_spec _nodeS1  = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_analog), node_s1_gpios);
        const gpio_dt_spec _nodeS2  = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_analog), node_s2_gpios);
        const gpio_dt_spec _nodeS3  = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_analog), node_s3_gpios);
        const adc_dt_spec  _channel = ADC_DT_SPEC_GET(DT_CHOSEN(opendeck_analog));

        static constexpr size_t muxCount()
        {
            return static_cast<size_t>(1U << controllerSelectorCount());
        }

        static constexpr size_t controllerSelectorCount()
        {
            return 2U +
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), controller_s2_gpios)
                   1U +
#endif
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), controller_s3_gpios)
                   1U +
#endif
                   0U;
        }

        bool configureSelectorPin(const gpio_dt_spec& pin)
        {
            if (!gpio_is_ready_dt(&pin))
            {
                return false;
            }

            return gpio_pin_configure_dt(&pin, GPIO_OUTPUT_INACTIVE) == 0;
        }

        void selectController(size_t mux)
        {
            gpio_pin_set_dt(&_controllerS0, (mux >> 0) & 0x1U);
            gpio_pin_set_dt(&_controllerS1, (mux >> 1) & 0x1U);
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), controller_s2_gpios)
            gpio_pin_set_dt(&_controllerS2, (mux >> 2) & 0x1U);
#endif
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), controller_s3_gpios)
            gpio_pin_set_dt(&_controllerS3, (mux >> 3) & 0x1U);
#endif
        }

        void selectNodeInput(size_t input)
        {
            gpio_pin_set_dt(&_nodeS0, (input >> 0) & 0x1U);
            gpio_pin_set_dt(&_nodeS1, (input >> 1) & 0x1U);
            gpio_pin_set_dt(&_nodeS2, (input >> 2) & 0x1U);
            gpio_pin_set_dt(&_nodeS3, (input >> 3) & 0x1U);
        }
    };
}    // namespace io::analog
