/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/drivers/count.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace opendeck::io::digital::drivers
{
    /**
     * @brief Identifies which physical pin of an encoder is being addressed.
     */
    enum class EncoderComponent : uint8_t
    {
        A,
        B,
    };

    /**
     * @brief Frame of sampled digital input states, indexed by flattened input index.
     */
    using Frame = std::array<bool, OPENDECK_DIGITAL_INPUT_COUNT>;

    /**
     * @brief Base interface for digital input drivers.
     */
    class DriverBase
    {
        public:
        virtual ~DriverBase() = default;

        /**
         * @brief Initializes the concrete digital input driver.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Returns the next sampled digital frame when available.
         *
         * @return Sampled frame, or `std::nullopt` when no frame is ready.
         */
        virtual std::optional<Frame> read() = 0;

        /**
         * @brief Returns the number of encoders represented by the driver layout.
         *
         * @return Encoder count.
         */
        virtual size_t encoder_count() const = 0;

        /**
         * @brief Maps a switch index to the encoder index that shares its pins.
         *
         * @param index Flattened switch/input index.
         *
         * @return Encoder index associated with the input.
         */
        virtual size_t switch_to_encoder_index(size_t index) = 0;

        /**
         * @brief Maps an encoder index and pin component to a flattened input index.
         *
         * @param index Encoder index to resolve.
         * @param c Encoder component to resolve.
         *
         * @return Flattened input index for the requested encoder component.
         */
        virtual size_t encoder_component_from_encoder(size_t index, EncoderComponent c) = 0;
    };
}    // namespace opendeck::io::digital::drivers
