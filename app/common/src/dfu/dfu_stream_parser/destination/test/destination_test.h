/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/dfu_stream_parser/shared/deps.h"

#include <cstddef>
#include <vector>

namespace opendeck::common::dfu::dfu_stream_parser
{
    /**
     * @brief DFU stream destination test implementation.
     */
    class DestinationTest : public Destination
    {
        public:
        bool supported() const override
        {
            return supported_result;
        }

        bool begin(const Header& header, uint32_t size) override
        {
            begin_called++;
            accepted_header = header;
            expected_size   = size;
            return begin_result;
        }

        bool write(std::span<const uint8_t> data) override
        {
            write_called++;

            if (!write_result)
            {
                return false;
            }

            payload.insert(payload.end(), data.begin(), data.end());
            return true;
        }

        bool finish() override
        {
            finish_called++;
            return finish_result;
        }

        void abort() override
        {
            abort_called++;
        }

        bool                 supported_result = true;
        bool                 begin_result     = true;
        bool                 write_result     = true;
        bool                 finish_result    = true;
        size_t               begin_called     = 0;
        size_t               write_called     = 0;
        size_t               finish_called    = 0;
        size_t               abort_called     = 0;
        uint32_t             expected_size    = 0;
        Header               accepted_header  = {};
        std::vector<uint8_t> payload;
    };
}    // namespace opendeck::common::dfu::dfu_stream_parser
