/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../driver_base.h"
#include "count.h"

namespace opendeck::io::digital::drivers
{
    /**
     * @brief Digital input driver for shift-register based button inputs.
     */
    class Driver : public DriverBase
    {
        public:
        /**
         * @brief Initializes shift-register GPIOs.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
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

            return true;
        }

        /**
         * @brief Samples all inputs and returns one synchronous frame.
         *
         * @return Sampled frame.
         */
        std::optional<Frame> read() override
        {
            return scan();
        }

        /**
         * @brief Returns the number of encoders represented by the input layout.
         *
         * @return Encoder count.
         */
        size_t encoder_count() const override
        {
            return ENCODER_COUNT;
        }

        /**
         * @brief Maps a flattened input index to its paired encoder index.
         *
         * @param index Flattened button/input index.
         *
         * @return Encoder index associated with the input.
         */
        size_t button_to_encoder_index(size_t index) override
        {
            return index / 2;
        }

        /**
         * @brief Maps an encoder index and component to a flattened input index.
         *
         * @param index Encoder index to resolve.
         * @param component Encoder component to resolve.
         *
         * @return Flattened input index for the requested encoder component.
         */
        size_t encoder_component_from_encoder(size_t index, EncoderComponent component) override
        {
            index *= 2;
            return component == EncoderComponent::A ? index : index + 1;
        }

        private:
        static constexpr size_t BUTTON_COUNT              = OPENDECK_BUTTON_PHYSICAL_COUNT;
        static constexpr size_t ENCODER_COUNT             = BUTTON_COUNT / 2;
        static constexpr size_t INPUTS_PER_SHIFT_REGISTER = 8;

        gpio_dt_spec _data  = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_buttons), data_gpios);
        gpio_dt_spec _clock = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_buttons), clock_gpios);
        gpio_dt_spec _latch = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_buttons), latch_gpios);

        /**
         * @brief Samples all inputs through the shift-register chain and returns one frame.
         */
        Frame scan()
        {
            Frame frame = {};

            gpio_pin_set_dt(&_clock, 0);
            gpio_pin_set_dt(&_latch, 0);
            k_busy_wait(1);
            gpio_pin_set_dt(&_latch, 1);

            for (size_t shift_register = 0; shift_register < DT_PROP(DT_NODELABEL(opendeck_buttons), shift_registers); shift_register++)
            {
                for (size_t input = 0; input < INPUTS_PER_SHIFT_REGISTER; input++)
                {
                    const size_t index = (shift_register * INPUTS_PER_SHIFT_REGISTER) + ((INPUTS_PER_SHIFT_REGISTER - 1) - input);

                    gpio_pin_set_dt(&_clock, 0);
                    k_busy_wait(1);
                    frame[index] = gpio_pin_get_dt(&_data) > 0;
                    gpio_pin_set_dt(&_clock, 1);
                }
            }

            return frame;
        }
    };
}    // namespace opendeck::io::digital::drivers
