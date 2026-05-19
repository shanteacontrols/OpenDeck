/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/shared/common.h"

#include <optional>

namespace opendeck::io::digital
{
    /**
     * @brief Hardware abstraction interface used by the digital subsystem.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Initializes the digital hardware backend.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Returns the next sampled logical digital frame when available.
         *
         * @return Sampled frame, or `std::nullopt` when no frame is ready.
         */
        virtual std::optional<Frame> read() = 0;

        /**
         * @brief Returns the number of encoders represented by the hardware layout.
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
}    // namespace opendeck::io::digital
