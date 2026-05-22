/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/dfu/staged_update_reader/instance/impl/staged_update_reader.h"
#include "common/src/dfu/dfu_stream/instance/impl/dfu_stream.h"

#include <zephyr/logging/log.h>

#include <algorithm>
#include <array>
#include <span>

namespace
{
    LOG_MODULE_REGISTER(staged_update_reader, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    static constexpr size_t BUFFER_SIZE = 256U;

    /**
     * @brief Returns the flash-aligned space reserved for the DFU header.
     */
    uint32_t header_storage_size(const size_t write_block_size)
    {
        if (write_block_size == 0U)
        {
            return 0;
        }

        return static_cast<uint32_t>(((opendeck::common::dfu::dfu_stream::HEADER_SIZE + write_block_size - 1U) / write_block_size) *
                                     write_block_size);
    }

    /**
     * @brief Reads the staged DFU header from the start of the partition.
     */
    bool read_header(opendeck::bootloader::dfu::staged_update_reader::Hwa& hwa, opendeck::common::dfu::dfu_stream::Header& header)
    {
        return hwa.read(0, header);
    }

    /**
     * @brief Checks whether the header describes a readable staged payload.
     */
    bool staged_payload_fits(const opendeck::common::dfu::dfu_stream::Header& header, const uint32_t storage_size)
    {
        const uint32_t payload_size = opendeck::common::dfu::dfu_stream::DfuStream::payload_size(header);

        return payload_size <= storage_size;
    }

}    // namespace

opendeck::bootloader::dfu::staged_update_reader::StagedUpdateReader::StagedUpdateReader(Hwa& hwa)
    : _hwa(hwa)
{}

bool opendeck::bootloader::dfu::staged_update_reader::StagedUpdateReader::consume(opendeck::common::dfu::dfu_stream::Sink& consumer)
{
    opendeck::common::dfu::dfu_stream::Header header = {};

    if (!_hwa.init())
    {
        LOG_ERR("Failed to initialize staged DFU storage");
        return false;
    }

    const uint32_t header_size = header_storage_size(_hwa.write_block_size());

    if (!read_header(_hwa, header))
    {
        LOG_ERR("Failed to read staged DFU header");
        return false;
    }

    if (!opendeck::common::dfu::dfu_stream::DfuStream::header_valid(header))
    {
        LOG_DBG("No valid staged DFU header");
        return false;
    }

    if ((header_size == 0U) || (_hwa.size() < header_size))
    {
        LOG_ERR("Invalid staged DFU storage geometry");
        return false;
    }

    if (!staged_payload_fits(header, _hwa.size() - header_size))
    {
        LOG_ERR("Staged firmware payload does not fit");
        return false;
    }

    const uint32_t payload_size = opendeck::common::dfu::dfu_stream::DfuStream::payload_size(header);

    LOG_INF("Applying staged firmware payload (%u bytes)", payload_size);

    if (!consumer.begin(header, payload_size))
    {
        LOG_ERR("Staged firmware payload rejected");

        if (!clear_pending())
        {
            LOG_ERR("Failed to clear staged firmware marker");
        }

        return false;
    }

    std::array<uint8_t, BUFFER_SIZE> buffer = {};
    uint32_t                         offset = 0;

    while (offset < payload_size)
    {
        const uint32_t chunk_size = std::min<uint32_t>(buffer.size(), payload_size - offset);

        if (!_hwa.read(header_size + offset, std::span<uint8_t>(buffer.data(), chunk_size)))
        {
            LOG_ERR("Failed to read staged DFU data");

            if (!clear_pending())
            {
                LOG_ERR("Failed to clear staged firmware marker");
            }

            return true;
        }

        if (!consumer.write(std::span<uint8_t>(buffer.data(), chunk_size)))
        {
            LOG_ERR("Staged firmware payload rejected");
            consumer.abort();

            if (!clear_pending())
            {
                LOG_ERR("Failed to clear staged firmware marker");
            }

            return false;
        }

        offset += chunk_size;
    }

    if (!consumer.finish())
    {
        LOG_ERR("Staged firmware payload rejected");
        consumer.abort();

        if (!clear_pending())
        {
            LOG_ERR("Failed to clear staged firmware marker");
        }

        return false;
    }

    if (!clear_pending())
    {
        LOG_ERR("Failed to clear staged firmware marker");
        return false;
    }

    LOG_INF("Staged firmware update applied");
    return true;
}

bool opendeck::bootloader::dfu::staged_update_reader::StagedUpdateReader::clear_pending()
{
    return _hwa.clear_pending();
}
