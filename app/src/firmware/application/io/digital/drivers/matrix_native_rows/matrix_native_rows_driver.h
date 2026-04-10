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

#include "../buffer.h"
#include "../driver_base.h"
#include "count.h"

#include "zlibs/utils/misc/timer.h"

namespace io::digital::drivers
{
    class Driver : public DriverBase
    {
        public:
        Driver()
            : _scanTimer(zlibs::utils::misc::Timer::Type::Repeating,
                         [this]()
                         {
                             scan();
                         })
        {}

        bool init() override
        {
            for (auto& row : _rows)
            {
                if (!gpio_is_ready_dt(&row))
                {
                    return false;
                }

                if (gpio_pin_configure_dt(&row, INPUT_FLAGS) != 0)
                {
                    return false;
                }
            }

            if (!initColumns())
            {
                return false;
            }

            _buffer.clear();
            scan();
            _scanTimer.start(SCAN_PERIOD_MS);

            return true;
        }

        std::optional<Frame> read() override
        {
            return _buffer.pop();
        }

        size_t encoderCount() const override
        {
            return ENCODER_COUNT;
        }

        size_t buttonToEncoderIndex(size_t index) override
        {
            const size_t row    = index / COLUMN_COUNT;
            const size_t column = index % COLUMN_COUNT;

            return ((row & ~static_cast<size_t>(1)) * COLUMN_COUNT) / 2 + column;
        }

        size_t encoderComponentFromEncoder(size_t index, encoderComponent_t component) override
        {
            const size_t column = index % COLUMN_COUNT;
            const size_t row    = (index / COLUMN_COUNT) * 2;
            const size_t base   = (row * COLUMN_COUNT) + column;

            return component == encoderComponent_t::A ? base : base + COLUMN_COUNT;
        }

        private:
        static constexpr size_t   ROW_COUNT      = DT_PROP_LEN(DT_CHOSEN(opendeck_buttons), row_gpios);
        static constexpr size_t   COLUMN_COUNT   = DT_PROP(DT_CHOSEN(opendeck_buttons), columns);
        static constexpr size_t   ENCODER_COUNT  = (ROW_COUNT * COLUMN_COUNT) / 2;
        static constexpr uint32_t SCAN_PERIOD_MS = 1;
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_buttons), ext_pullups)
        static constexpr gpio_flags_t INPUT_FLAGS = GPIO_INPUT;
#else
        static constexpr gpio_flags_t INPUT_FLAGS = GPIO_INPUT | GPIO_PULL_UP;
#endif
        std::array<gpio_dt_spec, ROW_COUNT> _rows = {
#define OPENDECK_BUTTON_MATRIX_ROW_GPIO_ENTRY(index, _) GPIO_DT_SPEC_GET_BY_IDX(DT_CHOSEN(opendeck_buttons), row_gpios, index),
            LISTIFY(ROW_COUNT, OPENDECK_BUTTON_MATRIX_ROW_GPIO_ENTRY, (, ), _)
#undef OPENDECK_BUTTON_MATRIX_ROW_GPIO_ENTRY
        };
        Buffer<Frame>             _buffer = {};
        zlibs::utils::misc::Timer _scanTimer;

#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_buttons), column_gpios)
        static constexpr size_t                       DIRECT_COLUMN_COUNT = DT_PROP_LEN(DT_CHOSEN(opendeck_buttons), column_gpios);
        std::array<gpio_dt_spec, DIRECT_COLUMN_COUNT> _columns            = {
#define OPENDECK_BUTTON_MATRIX_COLUMN_GPIO_ENTRY(index, _) GPIO_DT_SPEC_GET_BY_IDX(DT_CHOSEN(opendeck_buttons), column_gpios, index),
            LISTIFY(DIRECT_COLUMN_COUNT, OPENDECK_BUTTON_MATRIX_COLUMN_GPIO_ENTRY, (, ), _)
#undef OPENDECK_BUTTON_MATRIX_COLUMN_GPIO_ENTRY
        };
#else
        gpio_dt_spec _decoderA0 = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_buttons), decoder_a0_gpios);
        gpio_dt_spec _decoderA1 = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_buttons), decoder_a1_gpios);
        gpio_dt_spec _decoderA2 = GPIO_DT_SPEC_GET(DT_CHOSEN(opendeck_buttons), decoder_a2_gpios);
#endif

        bool initColumns()
        {
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_buttons), column_gpios)
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
            return gpio_is_ready_dt(&_decoderA0) &&
                   gpio_is_ready_dt(&_decoderA1) &&
                   gpio_is_ready_dt(&_decoderA2) &&
                   (gpio_pin_configure_dt(&_decoderA0, GPIO_OUTPUT_INACTIVE) == 0) &&
                   (gpio_pin_configure_dt(&_decoderA1, GPIO_OUTPUT_INACTIVE) == 0) &&
                   (gpio_pin_configure_dt(&_decoderA2, GPIO_OUTPUT_INACTIVE) == 0);
#endif
        }

        void selectColumn(size_t column)
        {
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_buttons), column_gpios)
            for (size_t i = 0; i < _columns.size(); i++)
            {
                gpio_pin_set_dt(&_columns[i], i == column);
            }
#else
            gpio_pin_set_dt(&_decoderA0, column & 0x01);
            gpio_pin_set_dt(&_decoderA1, (column >> 1) & 0x01);
            gpio_pin_set_dt(&_decoderA2, (column >> 2) & 0x01);
#endif
        }

        void scan()
        {
            Frame frame = {};

            for (size_t column = 0; column < COLUMN_COUNT; column++)
            {
                selectColumn(column);

                for (size_t row = 0; row < ROW_COUNT; row++)
                {
                    const size_t index = (row * COLUMN_COUNT) + column;
                    frame[index]       = gpio_pin_get_dt(&_rows[row]) > 0;
                }
            }

            _buffer.push(frame);
        }
    };
}    // namespace io::digital::drivers
