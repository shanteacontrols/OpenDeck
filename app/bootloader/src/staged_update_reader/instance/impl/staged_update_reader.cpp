/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/staged_update_reader/instance/impl/staged_update_reader.h"
#include "common/src/staged_update/shared/common.h"

#include <zephyr/logging/log.h>
#include <zephyr/sys/crc.h>

#include <algorithm>
#include <array>
#include <span>

namespace
{
    LOG_MODULE_REGISTER(staged_update_reader, LOG_LEVEL_INF);    // NOLINT

    static constexpr uint32_t STAGED_DFU_DATA_OFFSET = opendeck::staged_update::METADATA_SIZE;
    static constexpr size_t   BUFFER_SIZE            = 256U;

    /**
     * @brief Reads the staged-update metadata block from the start of the partition.
     */
    bool read_metadata(opendeck::staged_update_reader::Hwa& hwa, opendeck::staged_update::Metadata& metadata)
    {
        return hwa.read(0, std::span<uint8_t>(reinterpret_cast<uint8_t*>(&metadata), sizeof(metadata)));
    }

    /**
     * @brief Checks whether the metadata describes a readable staged payload.
     */
    bool metadata_valid(const opendeck::staged_update::Metadata& metadata, const uint32_t storage_size)
    {
        return (metadata.magic == opendeck::staged_update::METADATA_MAGIC) &&
               (metadata.format_version == opendeck::staged_update::METADATA_FORMAT_VERSION) &&
               (metadata.size > 0U) &&
               (storage_size >= STAGED_DFU_DATA_OFFSET) &&
               (metadata.size <= (storage_size - STAGED_DFU_DATA_OFFSET));
    }

    /**
     * @brief Verifies that the staged dfu.bin payload matches the stored CRC.
     */
    bool crc_valid(opendeck::staged_update_reader::Hwa& hwa, const opendeck::staged_update::Metadata& metadata)
    {
        std::array<uint8_t, BUFFER_SIZE> buffer = {};
        uint32_t                         crc    = 0;
        uint32_t                         offset = 0;

        while (offset < metadata.size)
        {
            const uint32_t chunk_size = std::min<uint32_t>(buffer.size(), metadata.size - offset);

            if (!hwa.read(STAGED_DFU_DATA_OFFSET + offset, std::span<uint8_t>(buffer.data(), chunk_size)))
            {
                return false;
            }

            crc = crc32_ieee_update(crc, buffer.data(), chunk_size);
            offset += chunk_size;
        }

        return crc == metadata.crc32;
    }

}    // namespace

opendeck::staged_update_reader::StagedUpdateReader::StagedUpdateReader(Hwa& hwa)
    : _hwa(hwa)
{}

bool opendeck::staged_update_reader::StagedUpdateReader::consume(Consumer& consumer)
{
    opendeck::staged_update::Metadata metadata;

    if (!_hwa.init() || !read_metadata(_hwa, metadata) || !metadata_valid(metadata, _hwa.size()))
    {
        return false;
    }

    if (!crc_valid(_hwa, metadata))
    {
        LOG_ERR("Staged DFU CRC mismatch");
        clear_pending();
        return false;
    }

    LOG_INF("Applying staged DFU update (%u bytes)", metadata.size);
    _hwa.on_update_start();

    consumer.reset();

    std::array<uint8_t, BUFFER_SIZE> buffer = {};
    uint32_t                         offset = 0;
    auto                             status = StreamStatus::Incomplete;

    while (offset < metadata.size)
    {
        const uint32_t chunk_size = std::min<uint32_t>(buffer.size(), metadata.size - offset);

        if (!_hwa.read(STAGED_DFU_DATA_OFFSET + offset, std::span<uint8_t>(buffer.data(), chunk_size)))
        {
            LOG_ERR("Failed to read staged DFU data");
            clear_pending();
            return true;
        }

        for (size_t i = 0; i < chunk_size; i++)
        {
            status = consumer.feed(buffer[i]);

            if (status == StreamStatus::Invalid)
            {
                LOG_ERR("Staged DFU stream rejected");
                clear_pending();
                return false;
            }
        }

        offset += chunk_size;
    }

    if (status != StreamStatus::Complete)
    {
        LOG_ERR("Staged DFU stream rejected");
        clear_pending();
        return false;
    }

    clear_pending();
    return true;
}

void opendeck::staged_update_reader::StagedUpdateReader::clear_pending()
{
    _hwa.clear_pending();
}
