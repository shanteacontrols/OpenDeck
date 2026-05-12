/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "driver_base.h"

#if defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_NATIVE)
#include "native/native_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_SHIFT_REGISTER)
#include "shift_register/shift_register_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_MATRIX_NATIVE_ROWS)
#include "matrix_native_rows/matrix_native_rows_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_MATRIX_SHIFT_REGISTER_ROWS)
#include "matrix_shift_register_rows/matrix_shift_register_rows_driver.h"
#else
namespace opendeck::io::digital::drivers
{
    /**
     * @brief Fallback no-op digital driver used when no hardware-specific driver is selected.
     */
    class Driver : public DriverBase
    {
        public:
        /**
         * @brief Initializes the fallback driver.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            return true;
        }

        /**
         * @brief Returns no sampled data.
         *
         * @return Always `std::nullopt`.
         */
        std::optional<Frame> read() override
        {
            return {};
        }

        /**
         * @brief Returns no encoders when no digital backend is configured.
         *
         * @return Always `0`.
         */
        size_t encoder_count() const override
        {
            return 0;
        }

        /**
         * @brief Maps a flattened input index to its paired encoder index.
         *
         * @param index Flattened switch/input index.
         *
         * @return Encoder index associated with the input.
         */
        size_t switch_to_encoder_index(size_t index) override
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
    };
}    // namespace opendeck::io::digital::drivers
#endif
