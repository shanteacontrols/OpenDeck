/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/flash_area/hwa/test/hwa_test.h"
#include "common/src/dfu/writer/instance/impl/dfu_writer.h"

#include <cstddef>
#include <vector>

namespace opendeck::common::dfu::dfu_stream_parser
{
    /**
     * @brief DFU writer test implementation.
     */
    class DfuWriterTest : private opendeck::common::dfu::flash_area::HwaTest, public opendeck::common::dfu::writer::DfuWriter
    {
        public:
        DfuWriterTest()
            : DfuWriter(static_cast<opendeck::common::dfu::flash_area::HwaTest&>(*this))
        {}

        bool supported() const override
        {
            return supported_result;
        }

        void fail_write()
        {
            set_write_result(false);
        }

        std::vector<uint8_t> payload() const
        {
            const auto data = storage();

            std::vector<uint8_t> result(data.begin(), data.begin() + expected_size);

            while (!result.empty() && (result.back() == opendeck::common::dfu::flash_area::ERASED_BYTE))
            {
                result.pop_back();
            }

            return result;
        }

        bool begin(const Header& header, uint32_t size) override
        {
            begin_called++;
            accepted_header = header;
            expected_size   = size;
            return begin_result && DfuWriter::begin(header, size);
        }

        bool     supported_result = true;
        bool     begin_result     = true;
        bool     finish_result    = true;
        size_t   begin_called     = 0;
        size_t   finish_called    = 0;
        size_t   abort_called     = 0;
        uint32_t expected_size    = 0;
        Header   accepted_header  = {};

        protected:
        bool commit(const Header&, uint32_t) override
        {
            finish_called++;
            return finish_result;
        }

        void cancel() override
        {
            abort_called++;
        }
    };
}    // namespace opendeck::common::dfu::dfu_stream_parser
