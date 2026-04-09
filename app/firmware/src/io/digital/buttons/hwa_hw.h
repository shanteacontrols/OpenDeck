/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "io/digital/frame_store.h"

namespace io::buttons
{
    /**
     * @brief Hardware-backed button backend that reads state from the shared frame store.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Constructs the button backend around a shared frame store.
         *
         * @param frame_store Frame store that exposes current button and encoder state.
         */
        explicit HwaHw(io::digital::FrameStore& frame_store)
            : _frame_store(frame_store)
        {}

        /**
         * @brief Initializes the hardware-backed button backend.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            return true;
        }

        /**
         * @brief Returns the current state for one button.
         *
         * @param index Button index to query.
         *
         * @return Button state, or `std::nullopt` when unavailable.
         */
        std::optional<bool> state(size_t index) override
        {
            return _frame_store.state(index);
        }

        /**
         * @brief Returns the current encoded state for the encoder paired with a button.
         *
         * @param index Button index to query.
         *
         * @return Encoded encoder state, or `std::nullopt` when unavailable.
         */
        std::optional<uint8_t> encoder_state(size_t index) override
        {
            return _frame_store.encoder_state(index);
        }

        /**
         * @brief Maps a button index to the corresponding encoder index.
         *
         * @param index Button index to map.
         *
         * @return Encoder index associated with the button.
         */
        size_t button_to_encoder_index(size_t index) override
        {
            return _frame_store.button_to_encoder_index(index);
        }

        private:
        io::digital::FrameStore& _frame_store;
    };
}    // namespace io::buttons
