/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/installer/instance/impl/installer.h"
#include "bootloader/src/signaling/signaling.h"

#include <zephyr/sys/util.h>

#include <span>

using namespace opendeck;

namespace
{
    constexpr uint32_t BYTE_MASK                                = 0xFFU;
    constexpr uint8_t  FW_METADATA_WORD_BYTES                   = sizeof(uint32_t);
    constexpr uint8_t  START_MAGIC_BYTES                        = sizeof(installer::START_COMMAND);
    constexpr uint8_t  FW_METADATA_WORD_COUNT_AFTER_START_MAGIC = 3U;
    constexpr uint8_t  FW_METADATA_BYTES_AFTER_START_MAGIC      = FW_METADATA_WORD_BYTES * FW_METADATA_WORD_COUNT_AFTER_START_MAGIC;
    constexpr uint8_t  END_COMMAND_BYTES                        = 4U;
}    // namespace

installer::Installer::Installer(Hwa& hwa)
    : _hwa(hwa)
    , _writer(*this)
{}

installer::Installer::StreamStatus installer::Installer::feed(const uint8_t data)
{
    if (_failed)
    {
        if (process_start(data) == StreamStatus::Complete)
        {
            status("Restarting DFU session");
            reset();
            _current_stage = static_cast<uint8_t>(ReceiveStage::FwMetadata);
            return StreamStatus::Incomplete;
        }

        return StreamStatus::Invalid;
    }

    if (_current_stage)
    {
        if (process_start(data) == StreamStatus::Complete)
        {
            status("Restarting DFU session");
            reset();
            return advance_stage();
        }
    }

    const auto ret = process_current_stage(data);

    if (ret == StreamStatus::Complete)
    {
        return advance_stage();
    }

    if (ret == StreamStatus::Invalid)
    {
        const bool write_failed = _failed;

        reset_state(!write_failed);

        if (write_failed)
        {
            return StreamStatus::Invalid;
        }

        if (process_current_stage(data) == StreamStatus::Invalid)
        {
            reset();
        }

        return StreamStatus::Invalid;
    }

    return StreamStatus::Incomplete;
}

installer::Installer::StreamStatus installer::Installer::process_start(const uint8_t data)
{
    if (((START_COMMAND >> (_start_bytes_received * BITS_PER_BYTE)) & BYTE_MASK) != data)
    {
        _start_bytes_received = 0;
        return StreamStatus::Invalid;
    }

    if (++_start_bytes_received == START_MAGIC_BYTES)
    {
        _start_bytes_received = 0;
        status("DFU session started");
        return StreamStatus::Complete;
    }

    return StreamStatus::Incomplete;
}

installer::Installer::StreamStatus installer::Installer::process_fw_metadata(const uint8_t data)
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
            status("Unsupported DFU format version");
            return StreamStatus::Invalid;
        }

        if (_received_uid != OPENDECK_TARGET_UID)
        {
            status("DFU target UID mismatch");
            return StreamStatus::Invalid;
        }

        status("DFU metadata accepted");
        bootloader::signaling::publish(bootloader::signaling::FirmwareUpdateStartedSignal{});
        status("Firmware update started");

        if (!prepare_write_block_size())
        {
            return StreamStatus::Invalid;
        }

        return StreamStatus::Complete;
    }

    return StreamStatus::Incomplete;
}

installer::Installer::StreamStatus installer::Installer::process_fw_chunk(const uint8_t data)
{
    if (!write_payload_byte(data))
    {
        return StreamStatus::Invalid;
    }

    _sector_bytes_received++;
    _fw_bytes_received++;

    if (_fw_bytes_received == _fw_size)
    {
        if (!finish_current_sector())
        {
            return StreamStatus::Invalid;
        }

        _stage_bytes_received = 0;
        status("Firmware payload fully received");
        return StreamStatus::Complete;
    }

    if (_sector_bytes_received == _hwa.sector_size(_current_sector))
    {
        if (!finish_current_sector())
        {
            return StreamStatus::Invalid;
        }

        _current_sector++;
    }

    return StreamStatus::Incomplete;
}

installer::Installer::StreamStatus installer::Installer::process_end(const uint8_t data)
{
    if (((END_COMMAND >> (_stage_bytes_received * BITS_PER_BYTE)) & static_cast<uint32_t>(BYTE_MASK)) != data)
    {
        status("DFU end marker mismatch");
        _stage_bytes_received = 0;
        return StreamStatus::Invalid;
    }

    if (++_stage_bytes_received == END_COMMAND_BYTES)
    {
        _stage_bytes_received = 0;
        status("DFU end marker detected");
        return StreamStatus::Complete;
    }

    return StreamStatus::Incomplete;
}

installer::Installer::StreamStatus installer::Installer::process_current_stage(const uint8_t data)
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

    return StreamStatus::Invalid;
}

installer::Installer::StreamStatus installer::Installer::advance_stage()
{
    if (_current_stage == static_cast<uint8_t>(ReceiveStage::End))
    {
        if (!_failed)
        {
            status("DFU stream complete, applying update");
            status("Firmware update complete, rebooting");
            _hwa.apply();
        }

        reset_state(true);
        return StreamStatus::Complete;
    }

    _stage_bytes_received = 0;
    _current_stage++;

    return StreamStatus::Incomplete;
}

void installer::Installer::reset()
{
    reset_state(true);
}

void installer::Installer::reset_state(const bool clear_failure)
{
    _current_stage           = 0;
    _current_sector          = 0;
    _sector_bytes_received   = 0;
    _stage_bytes_received    = 0;
    _fw_bytes_received       = 0;
    _fw_size                 = 0;
    _received_uid            = 0;
    _received_format_version = 0;
    _start_bytes_received    = 0;
    _write_block_size        = 0;
    _writer.reset();

    if (clear_failure)
    {
        _failed = false;
    }
}

bool installer::Installer::prepare_write_block_size()
{
    const size_t native_write_block_size = _hwa.write_block_size();

    if ((native_write_block_size == 0) ||
        (native_write_block_size > flash_stream_writer::FlashStreamWriter::MAX_FLASH_WRITE_BLOCK_SIZE))
    {
        return fail("Unsupported flash write block size");
    }

    const auto sector_size = _hwa.sector_size(_current_sector);

    if ((sector_size == 0) || ((sector_size % native_write_block_size) != 0))
    {
        return fail("Unsupported firmware sector size");
    }

    _write_block_size = std::min(static_cast<size_t>(sector_size),
                                 flash_stream_writer::FlashStreamWriter::MAX_FLASH_WRITE_BLOCK_SIZE);

    while ((_write_block_size > native_write_block_size) &&
           (((_write_block_size % native_write_block_size) != 0) ||
            ((sector_size % _write_block_size) != 0)))
    {
        _write_block_size--;
    }

    if (((_write_block_size % native_write_block_size) != 0) ||
        ((sector_size % _write_block_size) != 0))
    {
        return fail("Unsupported firmware sector size");
    }

    return true;
}

bool installer::Installer::write_payload_byte(const uint8_t data)
{
    const auto sector_size = _hwa.sector_size(_current_sector);

    if ((sector_size == 0) || ((sector_size % _write_block_size) != 0))
    {
        return fail("Unsupported firmware sector size");
    }

    if (_sector_bytes_received >= sector_size)
    {
        return fail("Flash buffer overflow");
    }

    if (_sector_bytes_received == 0)
    {
        if (!_hwa.erase_sector(_current_sector))
        {
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

bool installer::Installer::flush_write_block()
{
    if (!_writer.flush())
    {
        return fail("Flash write failed");
    }

    return true;
}

bool installer::Installer::write_block(const uint32_t offset, std::span<const uint8_t> data)
{
    return _hwa.write_sector(_current_sector, offset, data);
}

bool installer::Installer::finish_current_sector()
{
    if (!flush_write_block())
    {
        return false;
    }

    _sector_bytes_received = 0;
    _writer.reset();

    return true;
}

bool installer::Installer::fail(std::string_view status)
{
    this->status(status);
    _failed = true;
    reset_state(false);

    return false;
}

void installer::Installer::status(std::string_view message)
{
    bootloader::signaling::publish(bootloader::signaling::StatusSignal(message));
}
