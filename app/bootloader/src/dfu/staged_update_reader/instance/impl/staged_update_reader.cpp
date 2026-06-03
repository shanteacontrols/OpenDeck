/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/dfu/staged_update_reader/instance/impl/staged_update_reader.h"
#include "common/src/dfu/dfu_stream_parser/instance/impl/dfu_stream_parser.h"

#include <zephyr/logging/log.h>

#include <algorithm>
#include <array>
#include <span>

using namespace opendeck::bootloader::dfu::staged_update_reader;

namespace
{
    LOG_MODULE_REGISTER(staged_update_reader, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    static constexpr size_t BUFFER_SIZE = 256U;
}    // namespace

StagedUpdateReader::StagedUpdateReader(Hwa& hwa)
    : _hwa(hwa)
{}

bool StagedUpdateReader::consume(opendeck::common::dfu::writer::DfuWriter& writer)
{
    opendeck::common::dfu::dfu_stream_parser::Header header = {};

    if (!_hwa.init())
    {
        LOG_ERR("Failed to initialize staged DFU storage");
        return false;
    }

    const uint32_t header_size = StagedUpdate::header_storage_size();

    if (!read_header(header))
    {
        LOG_ERR("Failed to read staged DFU header");
        return false;
    }

    if (!opendeck::common::dfu::dfu_stream_parser::DfuStreamParser::header_valid(header))
    {
        LOG_DBG("No valid staged DFU header");
        return false;
    }

    if (header_size == 0U)
    {
        LOG_ERR("Invalid staged DFU storage geometry");
        return false;
    }

    const uint32_t payload_size = opendeck::common::dfu::dfu_stream_parser::DfuStreamParser::payload_size(header);

    LOG_INF("Applying staged firmware payload (%u bytes)", payload_size);

    if (!writer.begin(header, payload_size))
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

        if (!writer.write(std::span<uint8_t>(buffer.data(), chunk_size)))
        {
            LOG_ERR("Staged firmware payload rejected");
            writer.abort();

            if (!clear_pending())
            {
                LOG_ERR("Failed to clear staged firmware marker");
            }

            return false;
        }

        offset += chunk_size;
    }

    if (!writer.finish())
    {
        LOG_ERR("Staged firmware payload rejected");
        writer.abort();

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

bool StagedUpdateReader::clear_pending()
{
    return _hwa.clear_pending();
}

bool StagedUpdateReader::read_header(opendeck::common::dfu::dfu_stream_parser::Header& header)
{
    return _hwa.read(0, header);
}
