/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>

namespace opendeck::mcu
{
    /**
     * @brief Maximum number of hardware serial-number bytes exposed by firmware APIs.
     */
    constexpr inline size_t SERIAL_NUMBER_BUFFER_SIZE = 16;
}    // namespace opendeck::mcu
