/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/encoders/deps.h"
#include "firmware/src/io/digital/frame_store.h"

namespace opendeck::io::encoders
{
    /**
     * @brief Hardware-backed encoder backend that reads state from the digital frame store.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Constructs the encoder backend around a shared frame store.
         *
         * @param frame_store Frame store that exposes current encoder states.
         */
        explicit HwaHw(io::digital::FrameStore& frame_store)
            : _frame_store(frame_store)
        {}

        /**
         * @brief Returns the current encoded state for one encoder.
         *
         * @param index Encoder index to query.
         *
         * @return Encoded pair state, or `std::nullopt` when unavailable.
         */
        std::optional<uint8_t> state(size_t index) override
        {
            return _frame_store.encoder_state(index);
        }

        private:
        io::digital::FrameStore& _frame_store;
    };
}    // namespace opendeck::io::encoders
