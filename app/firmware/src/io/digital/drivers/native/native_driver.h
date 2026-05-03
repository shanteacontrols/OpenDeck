/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../driver_base.h"
#include "count.h"

#define OPENDECK_BUTTON_NATIVE_GPIO_ENTRY(index, node_id) GPIO_DT_SPEC_GET_BY_IDX(node_id, native_gpios, index)

namespace opendeck::io::digital::drivers
{
    /**
     * @brief Digital input driver for directly connected GPIO buttons.
     */
    class Driver : public DriverBase
    {
        public:
        /**
         * @brief Initializes GPIO inputs.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init() override
        {
            for (size_t i = 0; i < _pins.size(); i++)
            {
                if (!gpio_is_ready_dt(&_pins[i]))
                {
                    return false;
                }

                if (gpio_pin_configure_dt(&_pins[i], GPIO_INPUT) != 0)
                {
                    return false;
                }
            }

            return true;
        }

        /**
         * @brief Samples all GPIO inputs and returns one synchronous frame.
         *
         * @return Sampled frame.
         */
        std::optional<Frame> read() override
        {
            return scan();
        }

        /**
         * @brief Returns the number of encoders represented by the button matrix.
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
        static constexpr size_t BUTTON_COUNT  = OPENDECK_BUTTON_PHYSICAL_COUNT;
        static constexpr size_t ENCODER_COUNT = BUTTON_COUNT / 2;

        std::array<gpio_dt_spec, BUTTON_COUNT> _pins = { { LISTIFY(OPENDECK_BUTTON_PHYSICAL_COUNT, OPENDECK_BUTTON_NATIVE_GPIO_ENTRY, (, ), DT_NODELABEL(opendeck_buttons)) } };

        /**
         * @brief Reads one GPIO input pin.
         *
         * @param index Pin index to read.
         *
         * @return `true` when the input is active, otherwise `false`.
         */
        bool read_pin(size_t index) const
        {
            return gpio_pin_get_dt(&_pins[index]) > 0;
        }

        /**
         * @brief Samples all native GPIO inputs and returns one frame.
         */
        Frame scan()
        {
            Frame frame = {};

            for (size_t i = 0; i < BUTTON_COUNT; i++)
            {
                frame[i] = read_pin(i);
            }

            return frame;
        }
    };
}    // namespace opendeck::io::digital::drivers

#undef OPENDECK_BUTTON_NATIVE_GPIO_ENTRY
