/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../scan_driver_base.h"
#include "count.h"

#include <zephyr/kernel.h>

#define OPENDECK_ANALOG_MUX_CHANNEL_ENTRY(index, node_id) ADC_DT_SPEC_GET_BY_IDX(node_id, index)

namespace io::analog
{
    /**
     * @brief ADC driver for boards that route analog inputs through per-channel multiplexers.
     */
    class Driver : public drivers::ScanDriverBase<Driver>
    {
        public:
        /**
         * @brief Initializes the ADC device, selector pins, and mux channels.
         *
         * @return `true` if all hardware resources were prepared successfully, otherwise `false`.
         */
        bool init_driver()
        {
            if (!device_is_ready(_adc_device))
            {
                return false;
            }

            if (!configure_selector_pin(_selector_s0) ||
                !configure_selector_pin(_selector_s1) ||
                !configure_selector_pin(_selector_s2))
            {
                return false;
            }

#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), selector_s3_gpios)
            if (!configure_selector_pin(_selector_s3))
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

            select_input(0);
            return true;
        }

        /**
         * @brief Returns the total number of physical analog inputs scanned across all muxes.
         *
         * @return Number of mux channels multiplied by inputs per mux.
         */
        size_t physical_input_count() const
        {
            return _channels.size() * INPUTS_PER_MUX;
        }

        /**
         * @brief Selects the mux channel and input line to sample next.
         *
         * @param index Logical analog input index to sample.
         */
        void select_input(size_t index)
        {
            _active_mux_index = index / INPUTS_PER_MUX;
            set_mux_input(index % INPUTS_PER_MUX);
        }

        /**
         * @brief Performs one synchronous ADC conversion for the active mux channel.
         *
         * @param sample Storage location for the converted sample.
         *
         * @return `true` if the read succeeded, otherwise `false`.
         */
        bool read_sample(uint16_t& sample)
        {
            adc_sequence sequence = {};
            sequence.buffer       = &sample;
            sequence.buffer_size  = sizeof(sample);

            if (adc_sequence_init_dt(&_channels[_active_mux_index], &sequence) != 0)
            {
                return false;
            }

            return adc_read(_channels[_active_mux_index].dev, &sequence) == 0;
        }

        private:
        static constexpr size_t  MUX_COUNT      = DT_PROP_LEN(DT_NODELABEL(opendeck_analog), io_channels);
        static constexpr uint8_t INPUTS_PER_MUX = 16;

        const device* const _adc_device  = DEVICE_DT_GET(DT_IO_CHANNELS_CTLR_BY_IDX(DT_NODELABEL(opendeck_analog), 0));
        const gpio_dt_spec  _selector_s0 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_analog), selector_s0_gpios);
        const gpio_dt_spec  _selector_s1 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_analog), selector_s1_gpios);
        const gpio_dt_spec  _selector_s2 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_analog), selector_s2_gpios);
#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), selector_s3_gpios)
        const gpio_dt_spec _selector_s3 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_analog), selector_s3_gpios);
#endif
        std::array<adc_dt_spec, MUX_COUNT> _channels         = { { LISTIFY(DT_PROP_LEN(DT_NODELABEL(opendeck_analog), io_channels), OPENDECK_ANALOG_MUX_CHANNEL_ENTRY, (, ), DT_NODELABEL(opendeck_analog)) } };
        size_t                             _active_mux_index = 0;

        /**
         * @brief Configures one mux selector GPIO for output use.
         *
         * @param pin GPIO specification describing the selector pin.
         *
         * @return `true` if the pin is ready and configured, otherwise `false`.
         */
        bool configure_selector_pin(const gpio_dt_spec& pin)
        {
            if (!gpio_is_ready_dt(&pin))
            {
                return false;
            }

            return gpio_pin_configure_dt(&pin, GPIO_OUTPUT_INACTIVE) == 0;
        }

        /**
         * @brief Selects one input on the currently active analog multiplexer.
         *
         * @param input Mux input number to route to the ADC.
         */
        void set_mux_input(uint8_t input)
        {
            gpio_pin_set_dt(&_selector_s0, static_cast<int>((input >> 0) & 0x1U));
            gpio_pin_set_dt(&_selector_s1, static_cast<int>((input >> 1) & 0x1U));
            gpio_pin_set_dt(&_selector_s2, static_cast<int>((input >> 2) & 0x1U));
#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), selector_s3_gpios)
            gpio_pin_set_dt(&_selector_s3, static_cast<int>((input >> 3) & 0x1U));
#endif
        }
    };
}    // namespace io::analog

#undef OPENDECK_ANALOG_MUX_CHANNEL_ENTRY
