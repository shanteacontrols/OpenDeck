/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/encoders/deps.h"

namespace opendeck::io::encoders
{
    /**
     * @brief Stub encoder backend that never returns hardware state.
     */
    class HwaStub : public Hwa
    {
        public:
        HwaStub() = default;

        /**
         * @brief Returns no encoder state.
         *
         * @param index Encoder index to query.
         *
         * @return Always `std::nullopt`.
         */
        std::optional<uint8_t> state([[maybe_unused]] size_t index) override
        {
            return {};
        }
    };
}    // namespace opendeck::io::encoders
