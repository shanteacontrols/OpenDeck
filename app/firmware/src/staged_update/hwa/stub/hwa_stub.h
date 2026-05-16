/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/staged_update/shared/deps.h"

namespace opendeck::staged_update
{
    /**
     * @brief Empty backend used when staged DFU storage is not present.
     */
    class HwaStub : public Hwa
    {
        public:
        bool init() override
        {
            return false;
        }

        uint32_t size() const override
        {
            return 0;
        }

        size_t write_block_size() const override
        {
            return 0;
        }

        std::optional<Sector> sector(size_t) const override
        {
            return std::nullopt;
        }

        bool erase(uint32_t, uint32_t) override
        {
            return false;
        }

        bool write(uint32_t, std::span<const uint8_t>) override
        {
            return false;
        }
    };
}    // namespace opendeck::staged_update
