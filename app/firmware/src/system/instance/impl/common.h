/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace opendeck::firmware::sys
{
    /**
     * @brief Maximum number of component indexes processed during one staged forced-refresh step.
     */
    constexpr inline size_t FORCED_UPDATE_MAX_COMPONENTS_PER_RUN = 16;

    /**
     * @brief Delay between consecutive staged forced-refresh steps.
     *
     * This spaces out refresh traffic bursts while still completing the full refresh quickly enough
     * for UI state synchronization.
     */
    constexpr inline uint32_t FORCED_REFRESH_STAGE_DELAY_MS = 5;
}    // namespace opendeck::firmware::sys
