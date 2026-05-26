/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/shared/deps.h"

#include "zlibs/utils/misc/kwork_delayable.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <array>

#define OPENDECK_OUTPUTS_NODE DT_NODELABEL(opendeck_outputs)

#if DT_NODE_HAS_PROP(OPENDECK_OUTPUTS_NODE, decoder_a0_gpios)
#define OPENDECK_OUTPUT_MATRIX_HAS_DECODER  1
#define OPENDECK_OUTPUT_MATRIX_COLUMN_COUNT 8
#elif DT_NODE_HAS_PROP(OPENDECK_OUTPUTS_NODE, column_gpios)
#define OPENDECK_OUTPUT_MATRIX_HAS_DECODER  0
#define OPENDECK_OUTPUT_MATRIX_COLUMN_COUNT DT_PROP_LEN(OPENDECK_OUTPUTS_NODE, column_gpios)
#else
#error "Matrix output topology requires decoder GPIOs or column GPIOs."
#endif

namespace opendeck::io::outputs
{
    /**
     * @brief Matrix output driver with native row GPIOs.
     *
     * Output levels are cached in the driver. A periodic work item selects one
     * column at a time and drives row GPIOs for that column.
     */
    class Driver : public Hwa
    {
        public:
        /**
         * @brief Configures matrix GPIOs.
         */
        Driver()
            : _scan_work([this]()
                         {
                             scan();
                             _scan_work.reschedule(SCAN_INTERVAL_MS);
                         })
        {
            for (const auto& row : _rows)
            {
                if (device_is_ready(row.port))
                {
                    gpio_pin_configure_dt(&row, GPIO_OUTPUT_INACTIVE);
                }
            }

#if OPENDECK_OUTPUT_MATRIX_HAS_DECODER
            if (device_is_ready(_decoder_a0.port))
            {
                gpio_pin_configure_dt(&_decoder_a0, GPIO_OUTPUT_INACTIVE);
            }

            if (device_is_ready(_decoder_a1.port))
            {
                gpio_pin_configure_dt(&_decoder_a1, GPIO_OUTPUT_INACTIVE);
            }

            if (device_is_ready(_decoder_a2.port))
            {
                gpio_pin_configure_dt(&_decoder_a2, GPIO_OUTPUT_INACTIVE);
            }
#else
            for (const auto& column : _columns)
            {
                if (device_is_ready(column.port))
                {
                    gpio_pin_configure_dt(&column, GPIO_OUTPUT_INACTIVE);
                }
            }
#endif

            _scan_work.reschedule(0);
        }

        /**
         * @brief Caches one matrix-backed output level.
         *
         * @param index Output index to update.
         * @param level Output level percentage in the range [0, 100].
         */
        void set_level(size_t index, uint8_t level) override
        {
            if (index >= OUTPUT_COUNT)
            {
                return;
            }

            _levels[index] = level;
        }

        private:
        static constexpr size_t   ROW_COUNT        = DT_PROP_LEN(OPENDECK_OUTPUTS_NODE, row_gpios);
        static constexpr size_t   COLUMN_COUNT     = OPENDECK_OUTPUT_MATRIX_COLUMN_COUNT;
        static constexpr size_t   OUTPUT_COUNT     = ROW_COUNT * COLUMN_COUNT;
        static constexpr uint32_t SCAN_INTERVAL_MS = 1;
        static constexpr size_t   COLUMN_BIT_MASK  = 0x01U;

        std::array<uint8_t, OUTPUT_COUNT>   _levels = {};
        std::array<gpio_dt_spec, ROW_COUNT> _rows   = { {
#define OPENDECK_OUTPUT_MATRIX_ROW_GPIO_ENTRY(index, _) GPIO_DT_SPEC_GET_BY_IDX(OPENDECK_OUTPUTS_NODE, row_gpios, index)
            LISTIFY(DT_PROP_LEN(OPENDECK_OUTPUTS_NODE, row_gpios), OPENDECK_OUTPUT_MATRIX_ROW_GPIO_ENTRY, (, ), _)
#undef OPENDECK_OUTPUT_MATRIX_ROW_GPIO_ENTRY
        } };
#if OPENDECK_OUTPUT_MATRIX_HAS_DECODER
        gpio_dt_spec _decoder_a0 = GPIO_DT_SPEC_GET(OPENDECK_OUTPUTS_NODE, decoder_a0_gpios);
        gpio_dt_spec _decoder_a1 = GPIO_DT_SPEC_GET(OPENDECK_OUTPUTS_NODE, decoder_a1_gpios);
        gpio_dt_spec _decoder_a2 = GPIO_DT_SPEC_GET(OPENDECK_OUTPUTS_NODE, decoder_a2_gpios);
#else
        std::array<gpio_dt_spec, COLUMN_COUNT> _columns = { {
#define OPENDECK_OUTPUT_MATRIX_COLUMN_GPIO_ENTRY(index, _) GPIO_DT_SPEC_GET_BY_IDX(OPENDECK_OUTPUTS_NODE, column_gpios, index)
            LISTIFY(DT_PROP_LEN(OPENDECK_OUTPUTS_NODE, column_gpios), OPENDECK_OUTPUT_MATRIX_COLUMN_GPIO_ENTRY, (, ), _)
#undef OPENDECK_OUTPUT_MATRIX_COLUMN_GPIO_ENTRY
        } };
#endif
        size_t                             _active_column = 0;
        zlibs::utils::misc::KworkDelayable _scan_work;

        /**
         * @brief Selects one active matrix column.
         *
         * @param column Column index to activate.
         */
        void select_column(size_t column)
        {
#if OPENDECK_OUTPUT_MATRIX_HAS_DECODER
            gpio_pin_set_dt(&_decoder_a0, static_cast<int>(column & COLUMN_BIT_MASK));
            gpio_pin_set_dt(&_decoder_a1, static_cast<int>((column >> 1U) & COLUMN_BIT_MASK));
            gpio_pin_set_dt(&_decoder_a2, static_cast<int>((column >> 2U) & COLUMN_BIT_MASK));
#else
            for (auto& column_gpio : _columns)
            {
                gpio_pin_set_dt(&column_gpio, 0);
            }

            gpio_pin_set_dt(&_columns[column], 1);
#endif
        }

        /**
         * @brief Turns all row outputs off.
         */
        void clear_rows()
        {
            for (auto& row : _rows)
            {
                gpio_pin_set_dt(&row, 0);
            }
        }

        /**
         * @brief Drives rows for the currently selected column.
         */
        void scan()
        {
            clear_rows();

            _active_column++;

            if (_active_column >= COLUMN_COUNT)
            {
                _active_column = 0;
            }

            select_column(_active_column);

            for (size_t row = 0; row < ROW_COUNT; row++)
            {
                const size_t index = _active_column + row * COLUMN_COUNT;
                gpio_pin_set_dt(&_rows[row], _levels[index] > OUTPUT_LEVEL_MIN);
            }
        }
    };
}    // namespace opendeck::io::outputs

#undef OPENDECK_OUTPUT_MATRIX_COLUMN_COUNT
#undef OPENDECK_OUTPUT_MATRIX_HAS_DECODER
#undef OPENDECK_OUTPUTS_NODE
