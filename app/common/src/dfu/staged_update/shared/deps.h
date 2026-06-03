/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/dfu_stream_parser/shared/common.h"
#include "common/src/dfu/writer/shared/common.h"

#include <cstdint>

namespace opendeck::common::dfu::staged_update
{
    /**
     * @brief Shared staged-DFU storage layout.
     */
    class StagedUpdate
    {
        public:
        /**
         * @brief Returns the staged partition space reserved for the DFU header.
         */
        static constexpr uint32_t header_storage_size()
        {
            return HEADER_STORAGE_SIZE;
        }

        private:
        static constexpr uint32_t HEADER_STORAGE_SIZE = opendeck::common::dfu::writer::MAX_FLASH_WRITE_BLOCK_SIZE;

        static_assert(HEADER_STORAGE_SIZE >= opendeck::common::dfu::dfu_stream_parser::HEADER_SIZE,
                      "Staged DFU header storage must fit the raw DFU stream header.");
    };
}    // namespace opendeck::common::dfu::staged_update
