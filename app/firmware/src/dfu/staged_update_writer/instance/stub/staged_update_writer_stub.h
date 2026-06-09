/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/flash_area/shared/deps.h"
#include "common/src/dfu/writer/instance/impl/dfu_writer.h"

#include <optional>
#include <span>

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief Disabled staged-update destination used when staged updates are not supported.
     */
    class StagedUpdateWriter : public opendeck::common::dfu::writer::DfuWriter
    {
        private:
        class NullFlashArea : public opendeck::common::dfu::flash_area::Hwa
        {
            public:
            bool open() override
            {
                return false;
            }

            bool opened() const override
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

            bool read(uint32_t, std::span<uint8_t>) const override
            {
                return false;
            }

            bool write(uint32_t, std::span<const uint8_t>) const override
            {
                return false;
            }

            bool erase(uint32_t, uint32_t) const override
            {
                return false;
            }

            std::optional<opendeck::common::dfu::flash_area::Sector> sector(size_t) const override
            {
                return std::nullopt;
            }

            bool sectors(std::span<opendeck::common::dfu::flash_area::Sector>, size_t&) const override
            {
                return false;
            }
        };

        public:
        StagedUpdateWriter()
            : DfuWriter(_hwa)
        {}

        bool supported() const override
        {
            return false;
        }

        private:
        inline static NullFlashArea _hwa = {};
    };
}    // namespace opendeck::firmware::dfu::staged_update_writer
