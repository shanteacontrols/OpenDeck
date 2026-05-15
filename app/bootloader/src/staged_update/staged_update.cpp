/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/staged_update/staged_update.h"
#include "common/src/staged_update/common.h"
#include "bootloader/src/indicators/indicators.h"
#include "bootloader/src/updater/builder.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/fixed-partitions.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/crc.h>

#include <array>
#include <span>

#define OPENDECK_BOOTLOADER_NODE DT_NODELABEL(opendeck_bootloader)

#if DT_NODE_EXISTS(OPENDECK_BOOTLOADER_NODE) && DT_NODE_HAS_PROP(OPENDECK_BOOTLOADER_NODE, staged_dfu_partition)

namespace
{
    LOG_MODULE_REGISTER(staged_update, LOG_LEVEL_INF);    // NOLINT

#define OPENDECK_STAGED_DFU_NODE DT_PHANDLE(OPENDECK_BOOTLOADER_NODE, staged_dfu_partition)

    static constexpr uint8_t  STAGED_DFU_AREA_ID        = DT_PARTITION_ID(OPENDECK_STAGED_DFU_NODE);
    static constexpr uint32_t STAGED_DFU_PARTITION_SIZE = DT_REG_SIZE(OPENDECK_STAGED_DFU_NODE);
    static constexpr uint32_t STAGED_DFU_DATA_OFFSET    = opendeck::staged_update::METADATA_SIZE;
    static constexpr size_t   BUFFER_SIZE               = 256U;

    /**
     * @brief Reads bytes from the staged DFU flash partition.
     */
    bool read_area(uint32_t offset, std::span<uint8_t> data)
    {
        const struct flash_area* area = nullptr;

        if (flash_area_open(STAGED_DFU_AREA_ID, &area) != 0)
        {
            return false;
        }

        const bool read = flash_area_read(area, static_cast<off_t>(offset), data.data(), data.size()) == 0;
        flash_area_close(area);

        return read;
    }

    /**
     * @brief Clears the staged-update marker without erasing the whole partition.
     */
    bool write_invalid_magic()
    {
        const struct flash_area* area = nullptr;

        if (flash_area_open(STAGED_DFU_AREA_ID, &area) != 0)
        {
            return false;
        }

        const uint32_t invalid_magic = 0U;
        const bool     written       = flash_area_write(area, 0, &invalid_magic, sizeof(invalid_magic)) == 0;
        flash_area_close(area);

        return written;
    }

    /**
     * @brief Reads the staged-update metadata block from the start of the partition.
     */
    bool read_metadata(opendeck::staged_update::Metadata& metadata)
    {
        return read_area(0, std::span<uint8_t>(reinterpret_cast<uint8_t*>(&metadata), sizeof(metadata)));
    }

    /**
     * @brief Checks whether the metadata describes an update meant for this target.
     */
    bool metadata_valid(const opendeck::staged_update::Metadata& metadata)
    {
        return (metadata.magic == opendeck::staged_update::METADATA_MAGIC) &&
               (metadata.format_version == opendeck::staged_update::METADATA_FORMAT_VERSION) &&
               (metadata.target_uid == OPENDECK_TARGET_UID) &&
               (metadata.size > 0U) &&
               (metadata.size <= (STAGED_DFU_PARTITION_SIZE - STAGED_DFU_DATA_OFFSET));
    }

    /**
     * @brief Verifies that the staged dfu.bin payload matches the stored CRC.
     */
    bool crc_valid(const opendeck::staged_update::Metadata& metadata)
    {
        std::array<uint8_t, BUFFER_SIZE> buffer = {};
        uint32_t                         crc    = 0;
        uint32_t                         offset = 0;

        while (offset < metadata.size)
        {
            const uint32_t chunk_size = std::min<uint32_t>(buffer.size(), metadata.size - offset);

            if (!read_area(STAGED_DFU_DATA_OFFSET + offset, std::span<uint8_t>(buffer.data(), chunk_size)))
            {
                return false;
            }

            crc = crc32_ieee_update(crc, buffer.data(), chunk_size);
            offset += chunk_size;
        }

        return crc == metadata.crc32;
    }

    /**
     * @brief Returns the normal DFU updater wired to clear the staged marker when done.
     */
    opendeck::updater::Builder& updater_builder()
    {
        static opendeck::updater::Builder instance(opendeck::staged_update::clear_pending);
        return instance;
    }
}    // namespace

bool opendeck::staged_update::apply()
{
    Metadata metadata;

    if (!read_metadata(metadata) || !metadata_valid(metadata))
    {
        return false;
    }

    if (!crc_valid(metadata))
    {
        LOG_ERR("Staged DFU CRC mismatch");
        clear_pending();
        return false;
    }

    LOG_INF("Applying staged DFU update (%u bytes)", metadata.size);
    indicators::init();

    auto& updater = updater_builder().instance();
    updater.reset();

    std::array<uint8_t, BUFFER_SIZE> buffer = {};
    uint32_t                         offset = 0;

    while (offset < metadata.size)
    {
        const uint32_t chunk_size = std::min<uint32_t>(buffer.size(), metadata.size - offset);

        if (!read_area(STAGED_DFU_DATA_OFFSET + offset, std::span<uint8_t>(buffer.data(), chunk_size)))
        {
            LOG_ERR("Failed to read staged DFU data");
            clear_pending();
            return true;
        }

        for (size_t i = 0; i < chunk_size; i++)
        {
            updater.feed(buffer[i]);
        }

        offset += chunk_size;
    }

    if (!updater.completed() || updater.failed())
    {
        LOG_ERR("Staged DFU stream rejected");
        clear_pending();
        return false;
    }

    clear_pending();
    return true;
}

void opendeck::staged_update::clear_pending()
{
    if (!write_invalid_magic())
    {
        LOG_WRN("Failed to clear staged DFU marker");
    }
}

#else

bool opendeck::staged_update::apply()
{
    return false;
}

void opendeck::staged_update::clear_pending()
{}

#endif
