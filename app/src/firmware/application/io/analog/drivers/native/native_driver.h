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

#define OPENDECK_ANALOG_NATIVE_CHANNEL_ENTRY(index, node_id) ADC_DT_SPEC_GET_BY_IDX(node_id, index)

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

            return true;
        }

        std::optional<uint16_t> value(size_t index) override
        {
            if (index >= _channels.size())
            {
                return {};
            }

            int16_t sample = 0;

            adc_sequence sequence = {};
            sequence.buffer       = &sample;
            sequence.buffer_size  = sizeof(sample);

            if (adc_sequence_init_dt(&_channels[index], &sequence) != 0)
            {
                return {};
            }

            if (adc_read_dt(&_channels[index], &sequence) != 0)
            {
                return {};
            }

            return static_cast<uint16_t>(sample);
        }

        uint8_t adcBits() const override
        {
            return _channels[0].resolution;
        }

        private:
        static constexpr size_t               ANALOG_COUNT = OPENDECK_ANALOG_COUNT;
        const device* const                   _device      = DEVICE_DT_GET(DT_IO_CHANNELS_CTLR_BY_IDX(DT_CHOSEN(opendeck_analog), 0));
        std::array<adc_dt_spec, ANALOG_COUNT> _channels    = { { LISTIFY(OPENDECK_ANALOG_COUNT, OPENDECK_ANALOG_NATIVE_CHANNEL_ENTRY, (, ), DT_CHOSEN(opendeck_analog)) } };
    };
}    // namespace io::analog

#undef OPENDECK_ANALOG_NATIVE_CHANNEL_ENTRY
