/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace opendeck::common::dfu::flash_area
{
    static constexpr uint8_t ERASED_BYTE = 0xFFU;

    /**
     * @brief Flash sector descriptor with area-relative offset.
     */
    struct Sector
    {
        uint32_t offset = 0;
        uint32_t size   = 0;
    };
}    // namespace opendeck::common::dfu::flash_area
