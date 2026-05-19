/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstdint>

namespace opendeck::io::digital
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
    using Frame = std::array<bool, CONFIG_PROJECT_TARGET_DIGITAL_INPUT_COUNT>;
}    // namespace opendeck::io::digital
