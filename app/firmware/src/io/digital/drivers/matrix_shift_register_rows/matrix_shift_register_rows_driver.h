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
     * @brief Digital input driver for matrices that read rows through a shift register.
     */
    class Driver : public DriverBase
    {
        public:
        /**
         * @brief Initializes shift-register and column-selection GPIOs.
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

            if (!init_columns())
            {
                return false;
            }

            return true;
        }

        /**
         * @brief Samples the matrix and returns one synchronous frame.
         *
         * @return Sampled frame.
         */
        std::optional<Frame> read() override
        {
            return scan();
        }

        /**
         * @brief Returns the number of encoders represented by the matrix layout.
         *
         * @return Encoder count.
         */
        size_t encoder_count() const override
        {
            return ENCODER_COUNT;
        }

        /**
         * @brief Maps a flattened matrix input index to its paired encoder index.
         *
         * @param index Flattened switch/input index.
         *
         * @return Encoder index associated with the input.
         */
        size_t switch_to_encoder_index(size_t index) override
        {
            const size_t row    = index / COLUMN_COUNT;
            const size_t column = index % COLUMN_COUNT;

            return ((row & ~static_cast<size_t>(1)) * COLUMN_COUNT) / 2 + column;
        }

        /**
         * @brief Maps an encoder index and component to a flattened matrix input index.
         *
         * @param index Encoder index to resolve.
         * @param component Encoder component to resolve.
         *
         * @return Flattened input index for the requested encoder component.
         */
        size_t encoder_component_from_encoder(size_t index, EncoderComponent component) override
        {
            const size_t column = index % COLUMN_COUNT;
            const size_t row    = (index / COLUMN_COUNT) * 2;
            const size_t base   = (row * COLUMN_COUNT) + column;

            return component == EncoderComponent::A ? base : base + COLUMN_COUNT;
        }

        private:
        static constexpr size_t   ROW_COUNT       = DT_PROP(DT_NODELABEL(opendeck_switches), rows);
        static constexpr size_t   COLUMN_COUNT    = DT_PROP(DT_NODELABEL(opendeck_switches), columns);
        static constexpr size_t   ENCODER_COUNT   = (ROW_COUNT * COLUMN_COUNT) / 2;
        static constexpr int      GPIO_INACTIVE   = 0;
        static constexpr int      GPIO_ACTIVE     = 1;
        static constexpr uint8_t  COLUMN_BIT_MASK = 0x01;
        static constexpr uint32_t SHIFT_DELAY_US  = 1;

        gpio_dt_spec _data  = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_switches), data_gpios);
        gpio_dt_spec _clock = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_switches), clock_gpios);
        gpio_dt_spec _latch = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_switches), latch_gpios);

#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_switches), column_gpios)
        std::array<gpio_dt_spec, DT_PROP_LEN(DT_NODELABEL(opendeck_switches), column_gpios)> _columns = { {
#define OPENDECK_SWITCH_MATRIX_SR_COLUMN_GPIO_ENTRY(index, _) GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(opendeck_switches), column_gpios, index)
            LISTIFY(DT_PROP_LEN(DT_NODELABEL(opendeck_switches), column_gpios), OPENDECK_SWITCH_MATRIX_SR_COLUMN_GPIO_ENTRY, (, ), _)
#undef OPENDECK_SWITCH_MATRIX_SR_COLUMN_GPIO_ENTRY
        } };
#else
        gpio_dt_spec _decoder_a0 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_switches), decoder_a0_gpios);
        gpio_dt_spec _decoder_a1 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_switches), decoder_a1_gpios);
        gpio_dt_spec _decoder_a2 = GPIO_DT_SPEC_GET(DT_NODELABEL(opendeck_switches), decoder_a2_gpios);
#endif

        /**
         * @brief Initializes column-selection outputs.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init_columns()
        {
#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_switches), column_gpios)
            for (auto& column : _columns)
            {
                if (!gpio_is_ready_dt(&column))
                {
                    return false;
                }

                if (gpio_pin_configure_dt(&column, GPIO_OUTPUT_INACTIVE) != 0)
                {
                    return false;
                }
            }

            return true;
#else
            return gpio_is_ready_dt(&_decoder_a0) &&
                   gpio_is_ready_dt(&_decoder_a1) &&
                   gpio_is_ready_dt(&_decoder_a2) &&
                   (gpio_pin_configure_dt(&_decoder_a0, GPIO_OUTPUT_INACTIVE) == 0) &&
                   (gpio_pin_configure_dt(&_decoder_a1, GPIO_OUTPUT_INACTIVE) == 0) &&
                   (gpio_pin_configure_dt(&_decoder_a2, GPIO_OUTPUT_INACTIVE) == 0);
#endif
        }

        /**
         * @brief Selects one active column in the matrix.
         *
         * @param column Column index to activate.
         */
        void select_column(size_t column)
        {
#if DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_switches), column_gpios)
            for (size_t i = 0; i < _columns.size(); i++)
            {
                gpio_pin_set_dt(&_columns[i], static_cast<int>(i == column));
            }
#else
            gpio_pin_set_dt(&_decoder_a0, static_cast<int>((column >> 0U) & COLUMN_BIT_MASK));
            gpio_pin_set_dt(&_decoder_a1, static_cast<int>((column >> 1U) & COLUMN_BIT_MASK));
            gpio_pin_set_dt(&_decoder_a2, static_cast<int>((column >> 2U) & COLUMN_BIT_MASK));
#endif
        }

        /**
         * @brief Samples the full matrix and returns one frame.
         */
        Frame scan()
        {
            Frame frame = {};

            for (size_t column = 0; column < COLUMN_COUNT; column++)
            {
                select_column(column);

                gpio_pin_set_dt(&_clock, GPIO_INACTIVE);
                gpio_pin_set_dt(&_latch, GPIO_INACTIVE);
                k_busy_wait(SHIFT_DELAY_US);
                gpio_pin_set_dt(&_latch, GPIO_ACTIVE);

                for (size_t row = 0; row < ROW_COUNT; row++)
                {
                    const size_t index = ((ROW_COUNT - 1 - row) * COLUMN_COUNT) + column;

                    gpio_pin_set_dt(&_clock, GPIO_INACTIVE);
                    k_busy_wait(SHIFT_DELAY_US);
                    frame[index] = gpio_pin_get_dt(&_data) > 0;
                    gpio_pin_set_dt(&_clock, GPIO_ACTIVE);
                }
            }

            return frame;
        }
    };
}    // namespace opendeck::io::digital::drivers
