/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "staged_update.h"

#include <algorithm>
#include <cstring>

#include <zephyr/sys/crc.h>
#include <zephyr/logging/log.h>

using namespace opendeck::staged_update;

namespace
{
    LOG_MODULE_REGISTER(staged_update, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

StagedUpdate::StagedUpdate(Hwa& hwa)
    : _hwa(hwa)
{}

bool StagedUpdate::begin(const uint32_t expected_size)
{
    if ((expected_size == 0U) || (expected_size > capacity()))
    {
        LOG_WRN("Invalid staged DFU size: %u", expected_size);
        return false;
    }

    if (!init_flash_area() || !erase_partition())
    {
        return false;
    }

    reset_state();
    _expected_size = expected_size;
    _active        = true;

    return true;
}

bool StagedUpdate::write(std::span<const uint8_t> data)
{
    if (!_active)
    {
        return false;
    }

    if (data.empty())
    {
        return true;
    }

    if (data.size() > (_expected_size - _bytes_written))
    {
        LOG_WRN("Staged DFU write exceeds expected size");
        abort();
        return false;
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

bool StagedUpdate::finish()
{
    if (!_active || (_bytes_written != _expected_size))
    {
        LOG_WRN("Incomplete staged DFU upload");
        abort();
        return false;
    }

    if (!flush_write_block() || !write_metadata())
    {
        abort();
        return false;
    }

    LOG_INF("Staged DFU upload committed (%u bytes)", _bytes_written);
    _active = false;

    return true;
}

void StagedUpdate::abort()
{
    if (_initialized)
    {
        erase_metadata_sector();
    }

    reset_state();
}

uint32_t StagedUpdate::bytes_written() const
{
    return _bytes_written;
}

uint32_t StagedUpdate::capacity() const
{
    const uint32_t size = _hwa.size();

    if (size <= METADATA_SIZE)
    {
        return 0;
    }

    return size - METADATA_SIZE;
}

bool StagedUpdate::init_flash_area()
{
    if (_initialized)
    {
        return true;
    }

    const size_t write_block_size = _hwa.write_block_size();

    if ((write_block_size == 0U) ||
        (write_block_size > MAX_FLASH_WRITE_BLOCK_SIZE) ||
        ((METADATA_SIZE % write_block_size) != 0U))
    {
        LOG_ERR("Unsupported staged DFU flash write block size");
        return false;
    }

    if (!_hwa.init())
    {
        LOG_ERR("Failed to initialize staged DFU flash area");
        return false;
    }

    _write_block_size = write_block_size;
    _initialized      = true;

    return true;
}

bool StagedUpdate::erase_partition()
{
    if (!_hwa.erase(0, _hwa.size()))
    {
        LOG_ERR("Failed to erase staged DFU partition");
        return false;
    }

    return true;
}

bool StagedUpdate::erase_metadata_sector()
{
    const auto sector = _hwa.sector(0);

    if (!sector)
    {
        return false;
    }

    return _hwa.erase(sector->offset, sector->size);
}

bool StagedUpdate::write_bytes(const uint32_t offset, std::span<const uint8_t> data)
{
    if ((data.size() % _write_block_size) != 0U ||
        (offset % _write_block_size) != 0U)
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

    return _hwa.write(offset, data);
}

bool StagedUpdate::append_stream_byte(const uint8_t data)
{
    if (!_write_block_cache.dirty)
    {
        _write_block_cache.offset       = METADATA_SIZE + _bytes_written;
        _write_block_cache.bytes_filled = 0;
        _write_block_cache.dirty        = true;
    }

    _write_block_buffer[_write_block_cache.bytes_filled++] = data;
    _crc                                                   = crc32_ieee_update(_crc, &data, sizeof(data));
    _bytes_written++;

    if (_write_block_cache.bytes_filled == _write_block_size)
    {
        return flush_write_block();
    }

    return true;
}

bool StagedUpdate::flush_write_block()
{
    if (!_write_block_cache.dirty)
    {
        return true;
    }

    std::fill(_write_block_buffer.begin() + _write_block_cache.bytes_filled,
              _write_block_buffer.begin() + _write_block_size,
              ERASED_BYTE);

    const auto block = std::span<const uint8_t>(_write_block_buffer.data(), _write_block_size);

    if (!write_bytes(_write_block_cache.offset, block))
    {
        LOG_ERR("Failed to write staged DFU block");
        return false;
    }

    _write_block_cache = {};

    return true;
}

bool StagedUpdate::write_metadata()
{
    Metadata metadata;
    metadata.target_uid = OPENDECK_TARGET_UID;
    metadata.size       = _bytes_written;
    metadata.crc32      = _crc;

    std::array<uint8_t, METADATA_SIZE> data = {};
    std::memcpy(data.data(), &metadata, sizeof(metadata));

    return write_bytes(0, data);
}

void StagedUpdate::reset_state()
{
    _expected_size     = 0;
    _bytes_written     = 0;
    _crc               = 0;
    _write_block_cache = {};
    _active            = false;
}
