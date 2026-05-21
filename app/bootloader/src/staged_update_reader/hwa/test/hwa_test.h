/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/staged_update_reader/shared/deps.h"
#include "common/src/flash_area/hwa/test/hwa_test.h"
#include "common/src/staged_update/shared/common.h"

#include <zephyr/sys/crc.h>

#include <cstring>
#include <optional>
#include <span>

namespace opendeck::staged_update_reader
{
    /**
     * @brief In-memory staged DFU storage used by reader tests.
     */
    class HwaTest : public Hwa
    {
        public:
        bool init() override
        {
            return _area.open(0);
        }

        uint32_t size() const override
        {
            return _area.size();
        }

        bool read(uint32_t offset, std::span<uint8_t> data) override
        {
            return _area.read(offset, data);
        }

        void clear_pending() override
        {
            _clear_pending_calls++;

            const uint32_t invalid_magic = 0U;
            _area.write(
                0,
                std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&invalid_magic), sizeof(invalid_magic)));
        }

        void on_update_start() override
        {
            _update_start_calls++;
        }

        void reset_storage()
        {
            _area.reset_storage();
            _clear_pending_calls = 0;
            _update_start_calls  = 0;
        }

        void stage(std::span<const uint8_t> payload,
                   std::optional<uint32_t>  crc_override = std::nullopt,
                   uint32_t                 magic        = staged_update::METADATA_MAGIC,
                   uint32_t                 format       = staged_update::METADATA_FORMAT_VERSION,
                   uint32_t                 target_uid   = OPENDECK_TARGET_UID)
        {
            staged_update::Metadata metadata = {
                .magic          = magic,
                .format_version = format,
                .target_uid     = target_uid,
                .size           = static_cast<uint32_t>(payload.size()),
                .crc32          = crc_override.value_or(crc32_ieee(payload.data(), payload.size())),
            };

            _area.write(
                0,
                std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&metadata), sizeof(metadata)));
            _area.write(staged_update::METADATA_SIZE, payload);
        }

        uint32_t metadata_magic() const
        {
            uint32_t   magic   = 0;
            const auto storage = _area.storage();
            std::memcpy(&magic, storage.data(), sizeof(magic));
            return magic;
        }

        size_t clear_pending_calls() const
        {
            return _clear_pending_calls;
        }

        size_t update_start_calls() const
        {
            return _update_start_calls;
        }

        bool init_called() const
        {
            return _area.open_called();
        }

        void set_init_result(bool result)
        {
            _area.set_open_result(result);
        }

        void set_read_result(bool result)
        {
            _area.set_read_result(result);
        }

        private:
        flash_area::HwaTest _area;
        size_t              _clear_pending_calls = 0;
        size_t              _update_start_calls  = 0;
    };
}    // namespace opendeck::staged_update_reader
