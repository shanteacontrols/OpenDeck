/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/updater/updater.h"
#include "bootloader/src/webusb/transport.h"

#include <zephyr/sys/util.h>

#include <algorithm>
#include <span>

using namespace opendeck;

namespace
{
    constexpr uint32_t BYTE_MASK                                = 0xFFU;
    constexpr uint8_t  FW_METADATA_WORD_BYTES                   = sizeof(uint32_t);
    constexpr uint8_t  START_MAGIC_BYTES                        = sizeof(updater::START_COMMAND);
    constexpr uint8_t  FW_METADATA_WORD_COUNT_AFTER_START_MAGIC = 3U;
    constexpr uint8_t  FW_METADATA_BYTES_AFTER_START_MAGIC      = FW_METADATA_WORD_BYTES * FW_METADATA_WORD_COUNT_AFTER_START_MAGIC;
    constexpr uint8_t  END_COMMAND_BYTES                        = 4U;
}    // namespace

updater::Updater::Updater(Hwa& hwa)
    : _hwa(hwa)
{}

void updater::Updater::feed(const uint8_t data)
{
    if (_failed)
    {
        if (process_start(data) == ProcessStatus::Complete)
        {
            webusb::status("Restarting DFU session");
            reset();
            _current_stage = static_cast<uint8_t>(ReceiveStage::FwMetadata);
        }

        return;
    }

    if (_current_stage)
    {
        if (process_start(data) == ProcessStatus::Complete)
        {
            webusb::status("Restarting DFU session");
            reset();
            advance_stage();
            return;
        }
    }

    const auto ret = process_current_stage(data);

    if (ret == ProcessStatus::Complete)
    {
        advance_stage();
    }
    else if (ret == ProcessStatus::Invalid)
    {
        const bool write_failed = _failed;

        reset_state(!write_failed);

        if (write_failed)
        {
            return;
        }

        if (process_current_stage(data) == ProcessStatus::Invalid)
        {
            reset();
        }
    }
}

updater::Updater::ProcessStatus updater::Updater::process_start(const uint8_t data)
{
    if (((START_COMMAND >> (_start_bytes_received * BITS_PER_BYTE)) & BYTE_MASK) != data)
    {
        _start_bytes_received = 0;
        return ProcessStatus::Invalid;
    }

    if (++_start_bytes_received == START_MAGIC_BYTES)
    {
        _start_bytes_received = 0;
        webusb::status("DFU session started");
        return ProcessStatus::Complete;
    }

    return ProcessStatus::Incomplete;
}

updater::Updater::ProcessStatus updater::Updater::process_fw_metadata(const uint8_t data)
{
    if (_stage_bytes_received < FW_METADATA_WORD_BYTES)
    {
        _received_format_version |= (static_cast<uint32_t>(data) << (BITS_PER_BYTE * _stage_bytes_received));
    }
    else if (_stage_bytes_received < (FW_METADATA_WORD_BYTES * 2U))
    {
        _received_uid |= (static_cast<uint32_t>(data) << (BITS_PER_BYTE * (_stage_bytes_received - FW_METADATA_WORD_BYTES)));
    }
    else
    {
        _fw_size |= (static_cast<uint32_t>(data) << (BITS_PER_BYTE * (_stage_bytes_received - (FW_METADATA_WORD_BYTES * 2U))));
    }

    if (++_stage_bytes_received == FW_METADATA_BYTES_AFTER_START_MAGIC)
    {
        if (_received_format_version != FORMAT_VERSION)
        {
            webusb::status("Unsupported DFU format version");
            return ProcessStatus::Invalid;
        }

        if (_received_uid != OPENDECK_TARGET_UID)
        {
            webusb::status("DFU target UID mismatch");
            return ProcessStatus::Invalid;
        }

        webusb::status("DFU metadata accepted");
        _hwa.on_firmware_update_start();

        if (!prepare_write_block_size())
        {
            return ProcessStatus::Invalid;
        }

        return ProcessStatus::Complete;
    }

    return ProcessStatus::Incomplete;
}

updater::Updater::ProcessStatus updater::Updater::process_fw_chunk(const uint8_t data)
{
    if (!write_payload_byte(data))
    {
        return ProcessStatus::Invalid;
    }

    _fw_page_bytes_received++;
    _fw_bytes_received++;

    bool page_written = false;

    if (_fw_page_bytes_received == _hwa.page_size(_current_fw_page))
    {
        if (!finish_current_page())
        {
            return ProcessStatus::Invalid;
        }

        page_written = true;
    }

    if (_fw_bytes_received == _fw_size)
    {
        if (!finish_current_page())
        {
            return ProcessStatus::Invalid;
        }

        _stage_bytes_received = 0;
        webusb::status("Firmware payload fully received");
        return ProcessStatus::Complete;
    }

    if (page_written)
    {
        _current_fw_page++;
    }

    return ProcessStatus::Incomplete;
}

updater::Updater::ProcessStatus updater::Updater::process_end(const uint8_t data)
{
    if (((END_COMMAND >> (_stage_bytes_received * BITS_PER_BYTE)) & static_cast<uint32_t>(BYTE_MASK)) != data)
    {
        webusb::status("DFU end marker mismatch");
        _stage_bytes_received = 0;
        return ProcessStatus::Invalid;
    }

    if (++_stage_bytes_received == END_COMMAND_BYTES)
    {
        _stage_bytes_received = 0;
        webusb::status("DFU end marker detected");
        return ProcessStatus::Complete;
    }

    return ProcessStatus::Incomplete;
}

updater::Updater::ProcessStatus updater::Updater::process_current_stage(const uint8_t data)
{
    switch (static_cast<ReceiveStage>(_current_stage))
    {
    case ReceiveStage::Start:
        return process_start(data);

    case ReceiveStage::FwMetadata:
        return process_fw_metadata(data);

    case ReceiveStage::FwChunk:
        return process_fw_chunk(data);

    case ReceiveStage::End:
        return process_end(data);

    case ReceiveStage::Count:
        break;
    }

    return ProcessStatus::Invalid;
}

void updater::Updater::advance_stage()
{
    if (_current_stage == static_cast<uint8_t>(ReceiveStage::End))
    {
        if (!_failed)
        {
            webusb::status("DFU stream complete, applying update");
            _completed = true;
            _hwa.apply();
        }

        reset_state(true);
        return;
    }

    _stage_bytes_received = 0;
    _current_stage++;
}

void updater::Updater::reset()
{
    _completed = false;
    reset_state(true);
}

bool updater::Updater::completed() const
{
    return _completed;
}

bool updater::Updater::failed() const
{
    return _failed;
}

void updater::Updater::reset_state(const bool clear_failure)
{
    _current_stage           = 0;
    _current_fw_page         = 0;
    _fw_page_bytes_received  = 0;
    _stage_bytes_received    = 0;
    _fw_bytes_received       = 0;
    _fw_size                 = 0;
    _received_uid            = 0;
    _received_format_version = 0;
    _start_bytes_received    = 0;
    _write_block_size        = 0;
    _write_block_cache       = {};

    if (clear_failure)
    {
        _failed = false;
    }
}

bool updater::Updater::prepare_write_block_size()
{
    _write_block_size = _hwa.write_block_size();

    if ((_write_block_size == 0) ||
        (_write_block_size > MAX_FLASH_WRITE_BLOCK_SIZE))
    {
        return fail("Unsupported flash write block size");
    }

    const auto page_size = _hwa.page_size(_current_fw_page);

    if ((page_size == 0) || ((page_size % _write_block_size) != 0))
    {
        return fail("Unsupported app flash page size");
    }

    return true;
}

bool updater::Updater::write_payload_byte(const uint8_t data)
{
    const auto page_size = _hwa.page_size(_current_fw_page);

    if ((page_size == 0) || ((page_size % _write_block_size) != 0))
    {
        return fail("Unsupported app flash page size");
    }

    if (_fw_page_bytes_received >= page_size)
    {
        return fail("Flash buffer overflow");
    }

    if ((_fw_page_bytes_received == 0) && !_write_block_cache.dirty)
    {
        if (!_hwa.erase_page(_current_fw_page))
        {
            return fail("Flash erase failed");
        }
    }

    if (!_write_block_cache.dirty)
    {
        _write_block_cache.offset       = _fw_page_bytes_received;
        _write_block_cache.bytes_filled = 0;
        _write_block_cache.dirty        = true;
    }

    if (_write_block_cache.bytes_filled >= _write_block_size)
    {
        return fail("Invalid flash write block");
    }

    _write_block_buffer[_write_block_cache.bytes_filled++] = data;

    if (_write_block_cache.bytes_filled == _write_block_size)
    {
        return flush_write_block();
    }

    return true;
}

bool updater::Updater::flush_write_block()
{
    if (!_write_block_cache.dirty)
    {
        return true;
    }

    if (_write_block_cache.bytes_filled < _write_block_size)
    {
        std::fill(_write_block_buffer.begin() + _write_block_cache.bytes_filled,
                  _write_block_buffer.begin() + _write_block_size,
                  ERASED_BYTE);
    }

    const auto block = std::span<const uint8_t>(_write_block_buffer.data(), _write_block_size);

    if (!_hwa.write_page(_current_fw_page, _write_block_cache.offset, block))
    {
        return fail("Flash write failed");
    }

    _write_block_cache = {};

    return true;
}

bool updater::Updater::finish_current_page()
{
    if (!flush_write_block())
    {
        return false;
    }

    _fw_page_bytes_received = 0;

    return true;
}

bool updater::Updater::fail(const char* status)
{
    webusb::status(status);
    _failed = true;
    reset_state(false);

    return false;
}
