/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"

#include <cstdint>

namespace retained
{
    /**
     * @brief Retained variable container placed in the shared retained-memory section.
     *
     * Add future retained variables here so they all live in the same
     * fixed-layout object stored across resets.
     */
    struct Container
    {
        Retained<uint32_t> boot_mode;
    };

    static_assert(DT_REG_SIZE(OPENDECK_RETENTION_NODE) >= sizeof(Container),
                  "opendeck_retained must reserve enough space for the retained container.");

    inline Container data;
}    // namespace retained
