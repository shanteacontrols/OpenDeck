/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/src/dfu/flash_stream_writer/instance/impl/flash_stream_writer.h"

#include <algorithm>

using namespace opendeck::common::dfu::flash_stream_writer;

FlashStreamWriter::FlashStreamWriter(Sink& sink)
    : _sink(sink)
{}

bool FlashStreamWriter::begin(const size_t write_block_size, const uint32_t start_offset)
{
    reset();

    if ((write_block_size == 0U) || (write_block_size > MAX_FLASH_WRITE_BLOCK_SIZE))
    {
        return false;
    }

    _write_block_size = write_block_size;
    _next_offset      = start_offset;

    return true;
}

bool FlashStreamWriter::write(const uint8_t data)
{
    if (_write_block_size == 0U)
    {
        return false;
    }

    if (!_write_block_cache.dirty)
    {
        _write_block_cache.offset       = _next_offset;
        _write_block_cache.bytes_filled = 0;
        _write_block_cache.dirty        = true;
    }

    if (_write_block_cache.bytes_filled >= _write_block_size)
    {
        return false;
    }

    _write_block_buffer[_write_block_cache.bytes_filled++] = data;
    _next_offset++;

    if (_write_block_cache.bytes_filled == _write_block_size)
    {
        return flush();
    }

    return true;
}

bool FlashStreamWriter::flush()
{
    if (!_write_block_cache.dirty)
    {
        return true;
    }

    std::fill(_write_block_buffer.begin() + _write_block_cache.bytes_filled,
              _write_block_buffer.begin() + _write_block_size,
              ERASED_BYTE);

    const auto block = std::span<const uint8_t>(_write_block_buffer.data(), _write_block_size);

    if (!_sink.write_block(_write_block_cache.offset, block))
    {
        return false;
    }

    _write_block_cache = {};

    return true;
}

void FlashStreamWriter::reset()
{
    _write_block_size  = 0;
    _next_offset       = 0;
    _write_block_cache = {};
}

size_t FlashStreamWriter::write_block_size() const
{
    return _write_block_size;
}
