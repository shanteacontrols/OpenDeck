/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"
#include "bootloader/src/signaling/signaling.h"

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/sys/util.h>

#include <span>

using namespace opendeck::bootloader;

namespace
{
    LOG_MODULE_REGISTER(direct_update_writer, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

dfu::direct_update_writer::DirectUpdateWriter::DirectUpdateWriter(Hwa& hwa)
    : _hwa(hwa)
    , _writer(*this)
{}

void dfu::direct_update_writer::DirectUpdateWriter::reset()
{
    reset_state(true);
}

bool dfu::direct_update_writer::DirectUpdateWriter::begin([[maybe_unused]] const opendeck::common::dfu::dfu_stream_parser::Header& header,
                                                          const uint32_t                                                           size)
{
    reset();

    LOG_INF("Direct firmware update started: size=%u", size);

    if (!payload_fits(size))
    {
        LOG_ERR("Firmware payload does not fit: size=%u capacity=%u", size, _hwa.size());
        return fail("Firmware payload does not fit");
    }

    status("DFU header accepted");
    bootloader::signaling::publish(bootloader::signaling::FirmwareUpdateStartedSignal{});
    status("Firmware update started");

    _active = prepare_write_block_size();

    return _active;
}

bool dfu::direct_update_writer::DirectUpdateWriter::write(std::span<const uint8_t> data)
{
    if (data.empty())
    {
        return true;
    }

    if (!_active)
    {
        return fail("Firmware update not started");
    }

    for (const auto byte : data)
    {
        if (!write_payload_byte(byte))
        {
            return false;
        }

        _sector_bytes_received++;

        if (_sector_bytes_received == _hwa.sector_size(_current_sector))
        {
            if (!finish_current_sector())
            {
                return false;
            }

            _current_sector++;
        }
    }

    return true;
}

bool dfu::direct_update_writer::DirectUpdateWriter::finish()
{
    if (_failed || !_active)
    {
        LOG_ERR("Firmware update finish requested without active transfer");
        return fail("Firmware update not started");
    }

    if (_sector_bytes_received != 0U)
    {
        if (!finish_current_sector())
        {
            return false;
        }
    }

    status("Firmware payload fully received");
    LOG_INF("Direct firmware update payload written");
    status("Firmware update complete, rebooting");
    reset_state(true);

    _hwa.apply();

    LOG_PANIC();

    return true;
}

void dfu::direct_update_writer::DirectUpdateWriter::abort()
{
    reset();
}

void dfu::direct_update_writer::DirectUpdateWriter::reset_state(const bool clear_failure)
{
    _current_sector        = 0;
    _sector_bytes_received = 0;
    _write_block_size      = 0;
    _active                = false;
    _writer.reset();

    if (clear_failure)
    {
        _failed = false;
    }
}

bool dfu::direct_update_writer::DirectUpdateWriter::payload_fits(const uint32_t size)
{
    return size <= _hwa.size();
}

bool dfu::direct_update_writer::DirectUpdateWriter::prepare_write_block_size()
{
    const size_t native_write_block_size = _hwa.write_block_size();

    if ((native_write_block_size == 0) ||
        (native_write_block_size > opendeck::common::dfu::flash_stream_writer::MAX_FLASH_WRITE_BLOCK_SIZE))
    {
        LOG_ERR("Unsupported flash write block size: %u", static_cast<unsigned int>(native_write_block_size));
        return fail("Unsupported flash write block size");
    }

    const auto sector_size = _hwa.sector_size(_current_sector);

    if ((sector_size == 0) || ((sector_size % native_write_block_size) != 0))
    {
        LOG_ERR("Unsupported firmware sector size: sector=%u size=%u",
                static_cast<unsigned int>(_current_sector),
                sector_size);
        return fail("Unsupported firmware sector size");
    }

    _write_block_size = std::min(static_cast<size_t>(sector_size),
                                 opendeck::common::dfu::flash_stream_writer::MAX_FLASH_WRITE_BLOCK_SIZE);

    while ((_write_block_size > native_write_block_size) &&
           (((_write_block_size % native_write_block_size) != 0) ||
            ((sector_size % _write_block_size) != 0)))
    {
        _write_block_size--;
    }

    if (((_write_block_size % native_write_block_size) != 0) ||
        ((sector_size % _write_block_size) != 0))
    {
        LOG_ERR("Unable to derive aligned flash write block size");
        return fail("Unsupported firmware sector size");
    }

    return true;
}

bool dfu::direct_update_writer::DirectUpdateWriter::write_payload_byte(const uint8_t data)
{
    const auto sector_size = _hwa.sector_size(_current_sector);

    if ((sector_size == 0) || ((sector_size % _write_block_size) != 0))
    {
        LOG_ERR("Unsupported firmware sector during write: sector=%u size=%u",
                static_cast<unsigned int>(_current_sector),
                sector_size);
        return fail("Unsupported firmware sector size");
    }

    if (_sector_bytes_received >= sector_size)
    {
        return fail("Flash buffer overflow");
    }

    if (_sector_bytes_received == 0)
    {
        LOG_DBG("Erasing firmware sector %u", static_cast<unsigned int>(_current_sector));

        if (!_hwa.erase_sector(_current_sector))
        {
            LOG_ERR("Failed to erase firmware sector %u", static_cast<unsigned int>(_current_sector));
            return fail("Flash erase failed");
        }

        if (!_writer.begin(_write_block_size))
        {
            return fail("Unsupported flash write block size");
        }
    }

    if (!_writer.write(data))
    {
        return fail("Flash write failed");
    }

    return true;
}

bool dfu::direct_update_writer::DirectUpdateWriter::flush_write_block()
{
    if (!_writer.flush())
    {
        LOG_ERR("Failed to flush firmware write block");
        return fail("Flash write failed");
    }

    return true;
}

bool dfu::direct_update_writer::DirectUpdateWriter::write_block(const uint32_t offset, std::span<const uint8_t> data)
{
    if (!_hwa.write_sector(_current_sector, offset, data))
    {
        LOG_ERR("Failed to write firmware sector %u at offset %u",
                static_cast<unsigned int>(_current_sector),
                offset);
        return false;
    }

    return true;
}

bool dfu::direct_update_writer::DirectUpdateWriter::finish_current_sector()
{
    if (!flush_write_block())
    {
        return false;
    }

    _sector_bytes_received = 0;
    _writer.reset();

    return true;
}

bool dfu::direct_update_writer::DirectUpdateWriter::fail(std::string_view status)
{
    this->status(status);
    _failed = true;
    reset_state(false);

    return false;
}

void dfu::direct_update_writer::DirectUpdateWriter::status(std::string_view message)
{
    bootloader::signaling::publish(bootloader::signaling::StatusSignal(message));
}
