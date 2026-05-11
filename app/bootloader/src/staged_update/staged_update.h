/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace opendeck::staged_update
{
    /**
     * @brief Applies a staged dfu.bin update when a valid marker is present.
     *
     * @return `true` when a staged update was found and consumed, otherwise `false`.
     */
    bool apply();

    /**
     * @brief Invalidates the staged-update marker.
     */
    void clear_pending();
}    // namespace opendeck::staged_update
