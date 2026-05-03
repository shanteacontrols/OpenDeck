/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

namespace opendeck::updater
{
    /**
     * @brief Magic value that marks the beginning of a firmware update stream.
     */
    constexpr inline uint32_t START_COMMAND = OPENDECK_DFU_BEGIN_MAGIC;

    /**
     * @brief Supported DFU stream format version.
     */
    constexpr inline uint32_t FORMAT_VERSION = OPENDECK_DFU_FORMAT_VERSION;

    /**
     * @brief Magic value that marks the end of a firmware update stream.
     */
    constexpr inline uint32_t END_COMMAND = OPENDECK_DFU_END_MAGIC;
}    // namespace opendeck::updater
