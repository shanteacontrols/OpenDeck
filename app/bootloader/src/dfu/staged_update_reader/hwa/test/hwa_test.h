/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/staged_update_reader/shared/deps.h"
#include "common/src/dfu/dfu_stream/shared/common.h"
#include "common/src/dfu/flash_area/hwa/test/hwa_test.h"

#include "zlibs/utils/misc/bit.h"

#include <cstring>
#include <span>

namespace opendeck::bootloader::dfu::staged_update_reader
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

        size_t write_block_size() const override
        {
            return _area.write_block_size();
        }

        bool read(uint32_t offset, std::span<uint8_t> data) override
        {
            return _area.read(offset, data);
        }

        bool clear_pending() override
        {
            _clear_pending_calls++;

            const auto sector = _area.sector(0);

            return sector.has_value() &&
                   _area.erase(sector->offset, sector->size);
        }

        void reset_storage()
        {
            _area.reset_storage();
            _clear_pending_calls = 0;
        }

        void stage(std::span<const uint8_t> payload,
                   uint32_t                 start_magic    = opendeck::common::dfu::dfu_stream::START_COMMAND,
                   uint32_t                 format_version = opendeck::common::dfu::dfu_stream::FORMAT_VERSION,
                   uint32_t                 target_uid     = OPENDECK_TARGET_UID)
        {
            opendeck::common::dfu::dfu_stream::Header header = {};
            write_word(header, 0, start_magic);
            write_word(header, 1, format_version);
            write_word(header, 2, target_uid);
            write_word(header, 3, payload.size());

            _area.write(0, header);
            _area.write(header_storage_size(), payload);
        }

        uint32_t header_start_magic() const
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
        size_t header_storage_size() const
        {
            const size_t write_block_size = _area.write_block_size();

            return ((opendeck::common::dfu::dfu_stream::HEADER_SIZE + write_block_size - 1U) / write_block_size) * write_block_size;
        }

        static void write_word(opendeck::common::dfu::dfu_stream::Header& header, size_t word_index, uint32_t value)
        {
            const size_t offset = word_index * sizeof(value);

            for (size_t i = 0; i < sizeof(value); i++)
            {
                header[offset + i] = (value >> (i * zlibs::utils::misc::BYTE_BIT_COUNT)) & zlibs::utils::misc::BYTE_MASK;
            }
        }

        opendeck::common::dfu::flash_area::HwaTest _area;
        size_t                                     _clear_pending_calls = 0;
    };
}    // namespace opendeck::bootloader::dfu::staged_update_reader
