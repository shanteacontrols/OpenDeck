/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/staged_update_writer/instance/impl/staged_update_writer.h"

#include <zephyr/sys/crc.h>
#include <zephyr/logging/log.h>

#include <array>
#include <cstring>

using namespace opendeck::staged_update_writer;

namespace
{
    LOG_MODULE_REGISTER(staged_update_writer, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

StagedUpdateWriter::StagedUpdateWriter(Hwa& hwa)
    : _hwa(hwa)
    , _writer(*this)
{}

bool StagedUpdateWriter::begin(const uint32_t expected_size)
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

    if (!_writer.begin(_hwa.write_block_size(), opendeck::staged_update::METADATA_SIZE))
    {
        LOG_ERR("Unsupported staged DFU flash write block size");
        return false;
    }

    _expected_size = expected_size;
    _active        = true;

    return true;
}

bool StagedUpdateWriter::write(std::span<const uint8_t> data)
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

bool StagedUpdateWriter::finish()
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

void StagedUpdateWriter::abort()
{
    if (_initialized)
    {
        erase_metadata_sector();
    }

    reset_state();
}

uint32_t StagedUpdateWriter::bytes_written() const
{
    return _bytes_written;
}

uint32_t StagedUpdateWriter::capacity() const
{
    const uint32_t size = _hwa.size();

    if (size <= opendeck::staged_update::METADATA_SIZE)
    {
        return 0;
    }

    return size - opendeck::staged_update::METADATA_SIZE;
}

bool StagedUpdateWriter::init_flash_area()
{
    if (_initialized)
    {
        return true;
    }

    const size_t write_block_size = _hwa.write_block_size();

    if ((write_block_size == 0U) || ((opendeck::staged_update::METADATA_SIZE % write_block_size) != 0U))
    {
        LOG_ERR("Unsupported staged DFU flash write block size");
        return false;
    }

    if (!_hwa.init())
    {
        LOG_ERR("Failed to initialize staged DFU flash area");
        return false;
    }

    _initialized = true;

    return true;
}

bool StagedUpdateWriter::erase_partition()
{
    if (!_hwa.erase(0, _hwa.size()))
    {
        LOG_ERR("Failed to erase staged DFU partition");
        return false;
    }

    return true;
}

bool StagedUpdateWriter::erase_metadata_sector()
{
    const auto sector = _hwa.sector(0);

    if (!sector)
    {
        return false;
    }

    return _hwa.erase(sector->offset, sector->size);
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

    return _hwa.write(offset, data);
}

bool StagedUpdateWriter::append_stream_byte(const uint8_t data)
{
    if (!_writer.write(data))
    {
        return false;
    }

    _crc = crc32_ieee_update(_crc, &data, sizeof(data));
    _bytes_written++;

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

bool StagedUpdateWriter::write_metadata()
{
    opendeck::staged_update::Metadata metadata;
    metadata.target_uid = OPENDECK_TARGET_UID;
    metadata.size       = _bytes_written;
    metadata.crc32      = _crc;

    std::array<uint8_t, opendeck::staged_update::METADATA_SIZE> data = {};
    std::memcpy(data.data(), &metadata, sizeof(metadata));

    return write_block(0, data);
}

void StagedUpdateWriter::reset_state()
{
    _expected_size = 0;
    _bytes_written = 0;
    _crc           = 0;
    _writer.reset();
    _active = false;
}
