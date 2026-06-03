/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>

namespace opendeck::common::dfu::writer
{
    /**
     * @brief Maximum buffered DFU flash write size.
     */
    static constexpr size_t MAX_FLASH_WRITE_BLOCK_SIZE = 2048U;
}    // namespace opendeck::common::dfu::writer
