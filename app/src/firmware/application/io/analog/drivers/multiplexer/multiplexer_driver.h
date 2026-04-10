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

#define OPENDECK_ANALOG_MUX_CHANNEL_ENTRY(index, node_id) ADC_DT_SPEC_GET_BY_IDX(node_id, index)

namespace io::analog
{
    class Driver : public DriverBase
    {
        public:
        bool init() override
        {
            if (!device_is_ready(_device))
            {
                return false;
            }

            if (!configureSelectorPin(_selectorS0) ||
                !configureSelectorPin(_selectorS1) ||
                !configureSelectorPin(_selectorS2))
            {
                return false;
            }

#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), selector_s3_gpios)
            if (!configureSelectorPin(_selectorS3))
            {
                return false;
            }
#else
#error "Analog multiplexer driver requires selector-s3-gpios for 16-channel mux support."
#endif

            for (size_t i = 0; i < _channels.size(); i++)
            {
                if (!adc_is_ready_dt(&_channels[i]))
                {
                    return false;
                }

                if (adc_channel_setup_dt(&_channels[i]) != 0)
                {
                    return false;
                }
            }

            selectInput(0);
            return true;
        }

        std::optional<uint16_t> value(size_t index) override
        {
            if (index >= (_channels.size() * INPUTS_PER_MUX))
            {
                return {};
            }

            const size_t muxIndex   = index / INPUTS_PER_MUX;
            const size_t inputIndex = index % INPUTS_PER_MUX;

            selectInput(inputIndex);
            k_busy_wait(SETTLE_TIME_US);

            return readChannel(_channels[muxIndex]);
        }

        uint8_t adcBits() const override
        {
            return 12;
        }

        private:
        static constexpr size_t   MUX_COUNT      = DT_PROP_LEN(DT_CHOSEN(opendeck_analog), io_channels);
        static constexpr uint8_t  INPUTS_PER_MUX = 16;
        static constexpr uint32_t SETTLE_TIME_US = 10;

        const device* const _device     = DEVICE_DT_GET(DT_IO_CHANNELS_CTLR_BY_IDX(DT_CHOSEN(opendeck_analog), 0));
        const gpio_dt_spec  _selectorS0 = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_analog), selector_s0_gpios);
        const gpio_dt_spec  _selectorS1 = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_analog), selector_s1_gpios);
        const gpio_dt_spec  _selectorS2 = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_analog), selector_s2_gpios);
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), selector_s3_gpios)
        const gpio_dt_spec _selectorS3 = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_analog), selector_s3_gpios);
#endif
        std::array<adc_dt_spec, MUX_COUNT> _channels = { { LISTIFY(MUX_COUNT, OPENDECK_ANALOG_MUX_CHANNEL_ENTRY, (, ), DT_CHOSEN(opendeck_analog)) } };

        bool configureSelectorPin(const gpio_dt_spec& pin)
        {
            if (!gpio_is_ready_dt(&pin))
            {
                return false;
            }

            return gpio_pin_configure_dt(&pin, GPIO_OUTPUT_INACTIVE) == 0;
        }

        void selectInput(uint8_t input)
        {
            gpio_pin_set_dt(&_selectorS0, (input >> 0) & 0x1U);
            gpio_pin_set_dt(&_selectorS1, (input >> 1) & 0x1U);
            gpio_pin_set_dt(&_selectorS2, (input >> 2) & 0x1U);
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), selector_s3_gpios)
            gpio_pin_set_dt(&_selectorS3, (input >> 3) & 0x1U);
#endif
        }

        std::optional<uint16_t> readChannel(const adc_dt_spec& channel)
        {
            int16_t sample = 0;

            adc_sequence sequence = {};
            sequence.buffer       = &sample;
            sequence.buffer_size  = sizeof(sample);

            if (adc_sequence_init_dt(&channel, &sequence) != 0)
            {
                return {};
            }

            if (adc_read_dt(&channel, &sequence) != 0)
            {
                return {};
            }

            return static_cast<uint16_t>(sample);
        }
    };
}    // namespace io::analog

#undef OPENDECK_ANALOG_MUX_CHANNEL_ENTRY
