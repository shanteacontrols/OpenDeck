/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../scan_driver_base.h"
#include "count.h"

#include <zephyr/kernel.h>

namespace opendeck::io::analog
{
    /**
     * @brief ADC driver for cascaded controller and node multiplexers.
     */
    class Driver : public drivers::ScanDriverBase<Driver>
    {
        public:
        Driver() = default;

        /**
         * @brief Initializes the ADC channel and all controller/node selector pins.
         *
         * @return `true` if all hardware resources were prepared successfully, otherwise `false`.
         */
        bool init_driver()
        {
            if (!device_is_ready(_channel.dev))
            {
                return false;
            }

            if (!configure_selector_pin(_controller_s0) ||
                !configure_selector_pin(_controller_s1) ||
                !configure_selector_pin(_node_s0) ||
                !configure_selector_pin(_node_s1) ||
                !configure_selector_pin(_node_s2) ||
                !configure_selector_pin(_node_s3))
            {
                return false;
            }

#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), controller_s2_gpios)
            if (!configure_selector_pin(_controller_s2))
            {
                return false;
            }
#endif

#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), controller_s3_gpios)
            if (!configure_selector_pin(_controller_s3))
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

            _selected_controller = static_cast<size_t>(-1);
            _selected_node_input = static_cast<size_t>(-1);

            adc_sequence_init_dt(&_channel, &_sequence);
            _sequence.buffer      = nullptr;
            _sequence.buffer_size = sizeof(uint16_t);

            return true;
        }

        /**
         * @brief Returns the total number of physical analog inputs scanned by the mux tree.
         *
         * @return Number of controller muxes multiplied by inputs per node mux.
         */
        size_t physical_input_count() const
        {
            return mux_count() * INPUTS_PER_MUX;
        }

        /**
         * @brief Selects the controller mux and node input for the given physical index.
         *
         * @param index Physical analog input index to sample.
         */
        void select_input(size_t index)
        {
            const auto controller = index / INPUTS_PER_MUX;
            const auto node       = index % INPUTS_PER_MUX;

            select_controller(controller);
            select_node_input(node);
        }

        /**
         * @brief Performs one ADC conversion for the currently selected route.
         *
         * @param sample Storage location for the converted sample.
         *
         * @return `true` if the sample was read successfully, otherwise `false`.
         */
        bool read_sample(uint16_t& sample)
        {
            return read_active_sample(sample);
        }

        private:
        static constexpr uint8_t INPUTS_PER_MUX = 16;

        const gpio_dt_spec _controller_s0 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_analog), controller_s0_gpios);
        const gpio_dt_spec _controller_s1 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_analog), controller_s1_gpios);
#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), controller_s2_gpios)
        const gpio_dt_spec _controller_s2 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_analog), controller_s2_gpios);
#endif
#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), controller_s3_gpios)
        const gpio_dt_spec _controller_s3 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_analog), controller_s3_gpios);
#endif
        const gpio_dt_spec _node_s0 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_analog), node_s0_gpios);
        const gpio_dt_spec _node_s1 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_analog), node_s1_gpios);
        const gpio_dt_spec _node_s2 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_analog), node_s2_gpios);
        const gpio_dt_spec _node_s3 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_analog), node_s3_gpios);
        const adc_dt_spec  _channel = ADC_DT_SPEC_GET(DT_NODELABEL(opendeck_analog));

        adc_sequence _sequence            = {};
        size_t       _selected_controller = static_cast<size_t>(-1);
        size_t       _selected_node_input = static_cast<size_t>(-1);

        /**
         * @brief Returns the number of controller multiplexers addressable by the available selector pins.
         *
         * @return Number of controller muxes in the first mux stage.
         */
        static constexpr size_t mux_count()
        {
            return static_cast<size_t>(1U << controller_selector_count());
        }

        /**
         * @brief Returns the number of controller-selector GPIO bits present in the devicetree.
         *
         * @return Number of selector bits used to address controller muxes.
         */
        static constexpr size_t controller_selector_count()
        {
            return 2 +
#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), controller_s2_gpios)
                   1 +
#endif
#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), controller_s3_gpios)
                   1 +
#endif
                   0;
        }

        /**
         * @brief Configures one selector GPIO for output use.
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
         * @brief Selects which controller mux is currently active.
         *
         * @param mux Controller mux index to activate.
         */
        void select_controller(size_t mux)
        {
            if (_selected_controller == mux)
            {
                return;
            }

            if (_selected_controller == static_cast<size_t>(-1) ||
                (((_selected_controller >> 0U) & 0x1U) != ((mux >> 0U) & 0x1U)))
            {
                gpio_pin_set_dt(&_controller_s0, static_cast<int>((mux >> 0U) & 0x1U));
            }

            if (_selected_controller == static_cast<size_t>(-1) ||
                (((_selected_controller >> 1U) & 0x1U) != ((mux >> 1U) & 0x1U)))
            {
                gpio_pin_set_dt(&_controller_s1, static_cast<int>((mux >> 1U) & 0x1U));
            }
#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), controller_s2_gpios)
            if (_selected_controller == static_cast<size_t>(-1) ||
                (((_selected_controller >> 2U) & 0x1U) != ((mux >> 2U) & 0x1U)))
            {
                gpio_pin_set_dt(&_controller_s2, static_cast<int>((mux >> 2U) & 0x1U));
            }
#endif
#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_analog), controller_s3_gpios)
            if (_selected_controller == static_cast<size_t>(-1) ||
                (((_selected_controller >> 3U) & 0x1U) != ((mux >> 3U) & 0x1U)))
            {
                gpio_pin_set_dt(&_controller_s3, static_cast<int>((mux >> 3U) & 0x1U));
            }
#endif

            _selected_controller = mux;
        }

        /**
         * @brief Selects one input on the active node mux.
         *
         * @param input Node-mux input number to route to the ADC.
         */
        void select_node_input(size_t input)
        {
            if (_selected_node_input == input)
            {
                return;
            }

            if (_selected_node_input == static_cast<size_t>(-1) ||
                (((_selected_node_input >> 0U) & 0x1U) != ((input >> 0U) & 0x1U)))
            {
                gpio_pin_set_dt(&_node_s0, static_cast<int>((input >> 0U) & 0x1U));
            }

            if (_selected_node_input == static_cast<size_t>(-1) ||
                (((_selected_node_input >> 1U) & 0x1U) != ((input >> 1U) & 0x1U)))
            {
                gpio_pin_set_dt(&_node_s1, static_cast<int>((input >> 1U) & 0x1U));
            }

            if (_selected_node_input == static_cast<size_t>(-1) ||
                (((_selected_node_input >> 2U) & 0x1U) != ((input >> 2U) & 0x1U)))
            {
                gpio_pin_set_dt(&_node_s2, static_cast<int>((input >> 2U) & 0x1U));
            }

            if (_selected_node_input == static_cast<size_t>(-1) ||
                (((_selected_node_input >> 3U) & 0x1U) != ((input >> 3U) & 0x1U)))
            {
                gpio_pin_set_dt(&_node_s3, static_cast<int>((input >> 3U) & 0x1U));
            }

            _selected_node_input = input;
        }

        /**
         * @brief Performs one ADC conversion for the currently selected route.
         *
         * @param sample Storage location for the converted sample.
         *
         * @return `true` if the conversion succeeded, otherwise `false`.
         */
        bool read_active_sample(uint16_t& sample)
        {
            _sequence.buffer = &sample;

            return adc_read(_channel.dev, &_sequence) == 0;
        }
    };
}    // namespace opendeck::io::analog
