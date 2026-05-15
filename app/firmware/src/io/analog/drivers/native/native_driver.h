/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/analog/drivers/scan_driver_base.h"
#include "firmware/src/io/analog/drivers/native/count.h"

#define OPENDECK_ANALOG_NATIVE_CHANNEL_ENTRY(index, node_id) ADC_DT_SPEC_GET_BY_IDX(node_id, index)

namespace opendeck::io::analog
{
    /**
     * @brief Native ADC driver that scans each analog input directly.
     */
    class Driver : public drivers::ScanDriverBase<Driver>
    {
        public:
        /**
         * @brief Initializes every configured ADC channel.
         *
         * @return `true` if the ADC device and channels are ready, otherwise `false`.
         */
        bool init_driver()
        {
            if (!device_is_ready(_adc_device))
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

        /**
         * @brief Returns the number of physical analog inputs scanned by the driver.
         *
         * @return Number of configured ADC channels.
         */
        size_t physical_input_count() const
        {
            return _channels.size();
        }

        /**
         * @brief Selects which channel should be sampled next.
         *
         * @param index Logical analog input index to sample.
         */
        void select_input(size_t index)
        {
            _active_index = index;
        }

        /**
         * @brief Performs one synchronous ADC conversion for the selected channel.
         *
         * @return Converted sample, or `std::nullopt` if the read failed.
         */
        std::optional<uint16_t> read_sample()
        {
            uint16_t     sample   = 0;
            adc_sequence sequence = {};
            sequence.buffer       = &sample;
            sequence.buffer_size  = sizeof(sample);

            if (adc_sequence_init_dt(&_channels[_active_index], &sequence) != 0)
            {
                return {};
            }

            if (adc_read(_channels[_active_index].dev, &sequence) != 0)
            {
                return {};
            }

            return sample;
        }

        private:
        static constexpr size_t ANALOG_COUNT = OPENDECK_ANALOG_PHYSICAL_COUNT;

        const device* const                   _adc_device   = DEVICE_DT_GET(DT_IO_CHANNELS_CTLR_BY_IDX(DT_NODELABEL(opendeck_analog), 0));
        std::array<adc_dt_spec, ANALOG_COUNT> _channels     = { { LISTIFY(OPENDECK_ANALOG_PHYSICAL_COUNT, OPENDECK_ANALOG_NATIVE_CHANNEL_ENTRY, (, ), DT_NODELABEL(opendeck_analog)) } };
        size_t                                _active_index = 0;
    };
}    // namespace opendeck::io::analog

#undef OPENDECK_ANALOG_NATIVE_CHANNEL_ENTRY
