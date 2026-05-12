/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace opendeck::io::switches
{
    /**
     * @brief Stub switch backend that never returns live hardware state.
     */
    class HwaStub : public Hwa
    {
        public:
        HwaStub() = default;

        /**
         * @brief Initializes the stub backend.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            return true;
        }

        /**
         * @brief Returns no switch state.
         *
         * @param index Switch index to query.
         *
         * @return Always `std::nullopt`.
         */
        std::optional<bool> state([[maybe_unused]] size_t index) override
        {
            return {};
        }

        /**
         * @brief Returns no switch-to-encoder mapping.
         *
         * @param index Switch index to map.
         *
         * @return Always `0`.
         */
        size_t switch_to_encoder_index([[maybe_unused]] size_t index) override
        {
            return 0;
        }

        /**
         * @brief Returns no paired encoder state.
         *
         * @param index Switch index to query.
         *
         * @return Always `std::nullopt`.
         */
        std::optional<uint8_t> encoder_state([[maybe_unused]] size_t index) override
        {
            return {};
        }
    };
}    // namespace opendeck::io::switches
