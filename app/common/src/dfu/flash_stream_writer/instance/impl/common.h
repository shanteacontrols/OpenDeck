/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace opendeck::common::dfu::flash_stream_writer
{
    /** @brief Erased flash byte used to pad the final partial block. */
    static constexpr uint8_t ERASED_BYTE = 0xFFU;

    /** @brief Largest native write block this helper buffers internally. */
    static constexpr size_t MAX_FLASH_WRITE_BLOCK_SIZE = 256U;
}    // namespace opendeck::common::dfu::flash_stream_writer
