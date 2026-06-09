/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/src/dfu/writer/instance/impl/dfu_writer.h"
#include "common/src/signaling/shared/signaling.h"

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

#include <algorithm>

using namespace opendeck::common::dfu::writer;

namespace
{
    LOG_MODULE_REGISTER(dfu_writer, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

DfuWriter::DfuWriter(opendeck::common::dfu::flash_area::Hwa& hwa)
    : _hwa(hwa)
{}

bool DfuWriter::init()
{
    if (_initialized)
    {
        return true;
    }

    _initialized = _hwa.open();

    if (!_initialized)
    {
        LOG_ERR("Failed to initialize DFU flash area");
        return false;
    }

    const size_t native_block_size = _hwa.write_block_size();

    if ((native_block_size == 0U) ||
        (native_block_size > MAX_FLASH_WRITE_BLOCK_SIZE))
    {
        LOG_ERR("Unsupported DFU flash write block size");
        return false;
    }

    return true;
}

bool DfuWriter::begin(const opendeck::common::dfu::dfu_stream_parser::Header& header, const uint32_t expected_size)
{
    reset();
    LOG_INF("Firmware update started: size=%u", expected_size);

    if ((expected_size == 0U) || !init())
    {
        return false;
    }

    if (!prepare())
    {
        return false;
    }

    const uint32_t available = capacity();

    if (expected_size > available)
    {
        LOG_ERR("DFU payload does not fit: size=%u capacity=%u", expected_size, available);
        return false;
    }

    _header        = header;
    _expected_size = expected_size;
    _erased_sector = std::nullopt;
    _next_offset   = payload_offset();
    _block_offset  = 0;
    _block_size    = 0;
    _block_dirty   = false;
    _active        = true;

    status("DFU header accepted");
    opendeck::common::signaling::DirectBackend::publish(opendeck::common::signaling::FirmwareUpdateStartedSignal{});
    status("Firmware update started");

    return true;
}

bool DfuWriter::prepare()
{
    const size_t native_block_size = _hwa.write_block_size();

    if ((native_block_size == 0U) ||
        (native_block_size > MAX_FLASH_WRITE_BLOCK_SIZE))
    {
        LOG_ERR("Unsupported DFU flash write block size");
        return false;
    }

    bool found_sector = false;

    for (size_t candidate = MAX_FLASH_WRITE_BLOCK_SIZE;
         candidate >= native_block_size;
         candidate--)
    {
        if ((candidate % native_block_size) != 0U)
        {
            continue;
        }

        if ((payload_offset() % candidate) != 0U)
        {
            continue;
        }

        bool sector_compatible = true;

        for (size_t index = 0;; index++)
        {
            const auto sector = _hwa.sector(index);

            if (!sector)
            {
                break;
            }

            found_sector = true;

            if ((sector->size % candidate) != 0U)
            {
                sector_compatible = false;
                break;
            }
        }

        if (found_sector && sector_compatible)
        {
            _write_block_size = candidate;
            return true;
        }
    }

    LOG_ERR("Unable to derive DFU flash write block size");
    return false;
}

bool DfuWriter::write(std::span<const uint8_t> data)
{
    if (!_active)
    {
        LOG_ERR("DFU write requested without active transfer");
        return false;
    }

    if (data.size() > (_expected_size - _bytes_written))
    {
        LOG_ERR("DFU payload write exceeds expected size");
        abort();
        return false;
    }

    for (const auto byte : data)
    {
        if (!write_byte(byte))
        {
            abort();
            return false;
        }
    }

    _bytes_written += data.size();

    return true;
}

bool DfuWriter::finish()
{
    if (!_active)
    {
        LOG_ERR("DFU finish requested without active transfer");
        return false;
    }

    if (_bytes_written != _expected_size)
    {
        LOG_ERR("Incomplete DFU payload: received=%u expected=%u", _bytes_written, _expected_size);
        abort();
        return false;
    }

    if (!_block_dirty)
    {
        if (!commit(_header, _expected_size))
        {
            abort();
            return false;
        }

        return true;
    }

    std::fill(_block_buffer.begin() + _block_size,
              _block_buffer.begin() + _write_block_size,
              opendeck::common::dfu::flash_area::ERASED_BYTE);

    const auto block = std::span<const uint8_t>(_block_buffer.data(), _write_block_size);

    if (!write_block(_block_offset, block))
    {
        abort();
        return false;
    }

    _block_offset = 0;
    _block_size   = 0;
    _block_dirty  = false;

    if (!commit(_header, _expected_size))
    {
        abort();
        return false;
    }

    return true;
}

void DfuWriter::abort()
{
    const bool was_active = _active;

    reset();

    if (was_active)
    {
        cancel();
    }
}

bool DfuWriter::supported() const
{
    return true;
}

void DfuWriter::reset()
{
    _header           = {};
    _erased_sector    = std::nullopt;
    _write_block_size = 0;
    _expected_size    = 0;
    _bytes_written    = 0;
    _next_offset      = 0;
    _block_offset     = 0;
    _block_size       = 0;
    _block_dirty      = false;
    _active           = false;
}

size_t DfuWriter::write_block_size() const
{
    return _write_block_size;
}

uint32_t DfuWriter::capacity() const
{
    const uint32_t size   = _hwa.size();
    const uint32_t offset = payload_offset();

    if (size <= offset)
    {
        return 0;
    }

    return size - offset;
}

void DfuWriter::status(std::string_view message)
{
    LOG_INF("%.*s", static_cast<int>(message.size()), message.data());
    opendeck::common::signaling::DirectBackend::publish(opendeck::common::signaling::DfuStatusSignal(message));
}

bool DfuWriter::commit([[maybe_unused]] const opendeck::common::dfu::dfu_stream_parser::Header& header,
                       [[maybe_unused]] const uint32_t                                          expected_size)
{
    status("Firmware payload fully received");
    reset();

    return true;
}

void DfuWriter::cancel()
{}

uint32_t DfuWriter::payload_offset() const
{
    return 0;
}

bool DfuWriter::erase_range(const uint32_t offset, const size_t size)
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
            LOG_ERR("Failed to erase DFU sector %u", static_cast<unsigned int>(index));
            return false;
        }

        _erased_sector = index;
    }

    if (!found_sector)
    {
        LOG_ERR("DFU flash write has no backing sector: offset=%u size=%zu", offset, size);
    }

    return found_sector;
}

bool DfuWriter::write_byte(const uint8_t data)
{
    if (_write_block_size == 0U)
    {
        return false;
    }

    if (!_block_dirty)
    {
        _block_offset = _next_offset;
        _block_size   = 0;
        _block_dirty  = true;
    }

    if (_block_size >= _write_block_size)
    {
        return false;
    }

    _block_buffer[_block_size++] = data;
    _next_offset++;

    if (_block_size == _write_block_size)
    {
        const auto block = std::span<const uint8_t>(_block_buffer.data(), _write_block_size);

        if (!write_block(_block_offset, block))
        {
            return false;
        }

        _block_offset = 0;
        _block_size   = 0;
        _block_dirty  = false;
    }

    return true;
}

bool DfuWriter::write_block(const uint32_t offset, std::span<const uint8_t> data)
{
    if ((_write_block_size == 0U) ||
        (data.size() % _write_block_size) != 0U ||
        (offset % _write_block_size) != 0U)
    {
        LOG_ERR("Unaligned DFU flash write");
        return false;
    }

    const uint32_t size = _hwa.size();

    if ((offset > size) ||
        (data.size() > (size - offset)))
    {
        LOG_ERR("DFU flash write out of range");
        return false;
    }

    if (!erase_range(offset, data.size()))
    {
        return false;
    }

    if (!_hwa.write(offset, data))
    {
        LOG_ERR("Failed to write DFU block: offset=%u size=%zu", offset, data.size());
        return false;
    }

    return true;
}
