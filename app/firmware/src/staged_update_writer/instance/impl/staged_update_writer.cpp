/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/staged_update_writer/instance/impl/staged_update_writer.h"

#include <zephyr/logging/log.h>

#include <algorithm>
#include <array>

using namespace opendeck::staged_update_writer;

namespace
{
    LOG_MODULE_REGISTER(staged_update_writer, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

StagedUpdateWriter::StagedUpdateWriter(Hwa& hwa)
    : _hwa(hwa)
    , _writer(*this)
{}

bool StagedUpdateWriter::begin(const dfu_stream::Header& header, const uint32_t expected_size)
{
    LOG_INF("Staged DFU upload started: size=%u", expected_size);

    if (expected_size == 0U)
    {
        LOG_WRN("Invalid staged firmware payload size: %u", expected_size);
        return false;
    }

    if (expected_size > staging_capacity())
    {
        LOG_WRN("Staged firmware payload does not fit: %u", expected_size);
        return false;
    }

    if (!init_flash_area())
    {
        return false;
    }

    reset_state();

    if (!erase_header_sector() || !_writer.begin(_hwa.write_block_size(), header_storage_size()))
    {
        return false;
    }

    _expected_size = expected_size;
    _header        = header;
    _erased_sector = 0;
    _active        = true;

    return true;
}

bool StagedUpdateWriter::write(std::span<const uint8_t> data)
{
    if (!_active)
    {
        LOG_WRN("Staged DFU write requested without active upload");
        return false;
    }

    if (data.empty())
    {
        return true;
    }

    for (const auto byte : data)
    {
        if (!append_stream_byte(byte))
        {
            abort();
            return false;
        }
    }

    return true;
}

bool StagedUpdateWriter::finish()
{
    if (!_active)
    {
        LOG_WRN("Incomplete staged DFU upload");
        abort();
        return false;
    }

    if (!flush_write_block())
    {
        abort();
        return false;
    }

    if (!write_header(_header))
    {
        abort();
        return false;
    }

    LOG_INF("Staged DFU upload committed (%u bytes)", _expected_size);
    _active = false;

    return true;
}

void StagedUpdateWriter::abort()
{
    if (_active)
    {
        LOG_WRN("Staged DFU upload aborted");
    }

    if (_initialized)
    {
        erase_header_sector();
    }

    reset_state();
}

uint32_t StagedUpdateWriter::staging_capacity() const
{
    const uint32_t size = _hwa.size();

    const uint32_t header_size = header_storage_size();

    if (size <= header_size)
    {
        return 0;
    }

    return size - header_size;
}

uint32_t StagedUpdateWriter::header_storage_size() const
{
    const size_t write_block_size = _hwa.write_block_size();

    if ((write_block_size == 0U) ||
        (write_block_size > flash_stream_writer::FlashStreamWriter::MAX_FLASH_WRITE_BLOCK_SIZE))
    {
        return 0;
    }

    return static_cast<uint32_t>(((dfu_stream::HEADER_SIZE + write_block_size - 1U) / write_block_size) *
                                 write_block_size);
}

bool StagedUpdateWriter::init_flash_area()
{
    if (_initialized)
    {
        return true;
    }

    const size_t write_block_size = _hwa.write_block_size();

    if ((write_block_size == 0U) ||
        (write_block_size > flash_stream_writer::FlashStreamWriter::MAX_FLASH_WRITE_BLOCK_SIZE) ||
        (header_storage_size() == 0U))
    {
        LOG_ERR("Unsupported staged DFU flash write block size");
        return false;
    }

    if (!_hwa.init())
    {
        LOG_ERR("Failed to initialize staged DFU flash area");
        return false;
    }

    LOG_DBG("Staged DFU flash area initialized: size=%u write_block=%u header=%u",
            _hwa.size(),
            static_cast<unsigned int>(write_block_size),
            header_storage_size());

    _initialized = true;

    return true;
}

bool StagedUpdateWriter::erase_header_sector()
{
    const auto sector = _hwa.sector(0);

    if (!sector)
    {
        LOG_ERR("Failed to read staged DFU header sector");
        return false;
    }

    if (!_hwa.erase(sector->offset, sector->size))
    {
        LOG_ERR("Failed to erase staged DFU header sector");
        return false;
    }

    return true;
}

bool StagedUpdateWriter::erase_payload_range(const uint32_t offset, const size_t size)
{
    if (size == 0U)
    {
        return true;
    }

    const uint32_t end          = offset + size;
    bool           found_sector = false;

    for (size_t index = 0;; index++)
    {
        const auto sector = _hwa.sector(index);

        if (!sector)
        {
            break;
        }

        const uint32_t sector_end = sector->offset + sector->size;

        if ((offset >= sector_end) || (end <= sector->offset))
        {
            continue;
        }

        found_sector = true;

        if (_erased_sector == index)
        {
            continue;
        }

        if (!_hwa.erase(sector->offset, sector->size))
        {
            LOG_ERR("Failed to erase staged DFU sector %u", static_cast<unsigned int>(index));
            return false;
        }

        _erased_sector = index;
    }

    if (!found_sector)
    {
        LOG_ERR("Staged DFU flash write has no backing sector: offset=%u size=%zu", offset, size);
    }

    return found_sector;
}

bool StagedUpdateWriter::write_block(const uint32_t offset, std::span<const uint8_t> data)
{
    const auto write_block_size = _writer.write_block_size();

    if ((write_block_size == 0U) ||
        (data.size() % write_block_size) != 0U ||
        (offset % write_block_size) != 0U)
    {
        LOG_ERR("Unaligned staged DFU flash write");
        return false;
    }

    const uint32_t size = _hwa.size();

    if ((offset > size) ||
        (data.size() > (size - offset)))
    {
        LOG_ERR("Staged DFU flash write out of range");
        return false;
    }

    if (!erase_payload_range(offset, data.size()))
    {
        return false;
    }

    if (!_hwa.write(offset, data))
    {
        LOG_ERR("Failed to write staged DFU block: offset=%u size=%zu", offset, data.size());
        return false;
    }

    return true;
}

bool StagedUpdateWriter::append_stream_byte(const uint8_t data)
{
    if (!_writer.write(data))
    {
        return false;
    }

    return true;
}

bool StagedUpdateWriter::flush_write_block()
{
    if (!_writer.flush())
    {
        LOG_ERR("Failed to write staged DFU block");
        return false;
    }

    return true;
}

bool StagedUpdateWriter::write_header(const dfu_stream::Header& header)
{
    const auto write_block_size = _hwa.write_block_size();

    if ((write_block_size == 0U) ||
        (write_block_size > flash_stream_writer::FlashStreamWriter::MAX_FLASH_WRITE_BLOCK_SIZE))
    {
        LOG_ERR("Unsupported staged DFU flash write block size");
        return false;
    }

    const uint32_t header_size = header_storage_size();

    if (header_size == 0U)
    {
        LOG_ERR("Unsupported staged DFU flash write block size");
        return false;
    }

    std::array<uint8_t, flash_stream_writer::FlashStreamWriter::MAX_FLASH_WRITE_BLOCK_SIZE> data = {};

    std::fill(data.begin(), data.end(), flash_stream_writer::FlashStreamWriter::ERASED_BYTE);
    std::copy(header.begin(), header.end(), data.begin());

    if (!_hwa.write(0, std::span<const uint8_t>(data.data(), header_size)))
    {
        LOG_ERR("Failed to write staged DFU header");
        return false;
    }

    return true;
}

void StagedUpdateWriter::reset_state()
{
    _expected_size = 0;
    _header        = {};
    _erased_sector = std::nullopt;
    _writer.reset();
    _active = false;
}
