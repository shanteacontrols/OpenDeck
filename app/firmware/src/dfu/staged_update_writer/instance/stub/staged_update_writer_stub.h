/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/dfu_stream_parser/shared/deps.h"

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief Disabled staged-update destination used when staged updates are not supported.
     */
    class StagedUpdateWriter : public opendeck::common::dfu::dfu_stream_parser::Destination
    {
        public:
        bool supported() const override
        {
            return false;
        }

        bool begin(const opendeck::common::dfu::dfu_stream_parser::Header&, uint32_t) override
        {
            return false;
        }

        bool write(std::span<const uint8_t>) override
        {
            return false;
        }

        bool finish() override
        {
            return false;
        }

        void abort() override
        {}
    };
}    // namespace opendeck::firmware::dfu::staged_update_writer
