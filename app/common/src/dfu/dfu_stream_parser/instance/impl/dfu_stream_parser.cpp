/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/src/dfu/dfu_stream_parser/instance/impl/dfu_stream_parser.h"

#include "zlibs/utils/misc/bit.h"

#include <array>

using namespace opendeck::common::dfu::dfu_stream_parser;

namespace
{
    constexpr uint8_t WORD_BYTES             = sizeof(uint32_t);
    constexpr uint8_t METADATA_WORD_COUNT    = 3U;
    constexpr uint8_t METADATA_PAYLOAD_BYTES = WORD_BYTES * METADATA_WORD_COUNT;

    uint8_t expected_byte(const uint32_t value, const uint8_t byte_index)
    {
        return static_cast<uint8_t>((value >> (byte_index * zlibs::utils::misc::BYTE_BIT_COUNT)) & zlibs::utils::misc::BYTE_MASK);
    }

    uint32_t read_word(const Header& header, const size_t word_index)
    {
        uint32_t value  = 0;
        size_t   offset = word_index * sizeof(value);

        for (size_t i = 0; i < sizeof(value); i++)
        {
            value |= static_cast<uint32_t>(header[offset + i]) << (i * zlibs::utils::misc::BYTE_BIT_COUNT);
        }

        return value;
    }
}    // namespace

DfuStreamParser::DfuStreamParser(opendeck::common::dfu::writer::DfuWriter& writer)
    : _writer(writer)
{}

uint32_t DfuStreamParser::payload_size(const Header& header)
{
    return read_word(header, 3);
}

bool DfuStreamParser::header_valid(const Header& header)
{
    return (read_word(header, 0) == START_COMMAND) &&
           (read_word(header, 1) == FORMAT_VERSION) &&
           (read_word(header, 2) == OPENDECK_TARGET_UID) &&
           (payload_size(header) > 0U);
}

void DfuStreamParser::reset()
{
    _stage                   = ReceiveStage::Start;
    _status                  = StreamStatus::Incomplete;
    _stage_bytes_received    = 0;
    _received_format_version = 0;
    _received_uid            = 0;
    _expected_size           = 0;
    _bytes_written           = 0;
    _header.fill(0);
    _writer_active = false;
}

void DfuStreamParser::abort()
{
    _writer.abort();
    reset();
}

bool DfuStreamParser::supported() const
{
    return _writer.supported();
}

StreamStatus DfuStreamParser::feed(const uint8_t data)
{
    if (_status == StreamStatus::Invalid)
    {
        return _status;
    }

    if (_stage == ReceiveStage::Done)
    {
        _status = reject();
        return _status;
    }

    const auto status = process_current_stage(data);

    if (status == StreamStatus::Complete)
    {
        _status = advance_stage();
        return _status;
    }

    if (status == StreamStatus::Invalid)
    {
        _status = reject();
        return _status;
    }

    _status = StreamStatus::Incomplete;
    return _status;
}

StreamStatus DfuStreamParser::feed(std::span<const uint8_t> data)
{
    auto status = _status;

    for (const auto byte : data)
    {
        status = feed(byte);

        if (status == StreamStatus::Invalid)
        {
            break;
        }
    }

    return status;
}

StreamStatus DfuStreamParser::status() const
{
    return _status;
}

uint32_t DfuStreamParser::bytes_written() const
{
    return _bytes_written;
}

uint32_t DfuStreamParser::expected_size() const
{
    return _expected_size;
}

StreamStatus DfuStreamParser::process_start(const uint8_t data)
{
    _header[_stage_bytes_received] = data;

    if (expected_byte(START_COMMAND, _stage_bytes_received) != data)
    {
        return StreamStatus::Invalid;
    }

    if (++_stage_bytes_received == WORD_BYTES)
    {
        return StreamStatus::Complete;
    }

    return StreamStatus::Incomplete;
}

StreamStatus DfuStreamParser::process_metadata(const uint8_t data)
{
    _header[WORD_BYTES + _stage_bytes_received] = data;

    if (_stage_bytes_received < WORD_BYTES)
    {
        _received_format_version |= (static_cast<uint32_t>(data) << (zlibs::utils::misc::BYTE_BIT_COUNT * _stage_bytes_received));
    }
    else if (_stage_bytes_received < (WORD_BYTES * 2U))
    {
        _received_uid |= (static_cast<uint32_t>(data) << (zlibs::utils::misc::BYTE_BIT_COUNT * (_stage_bytes_received - WORD_BYTES)));
    }
    else
    {
        _expected_size |= (static_cast<uint32_t>(data) << (zlibs::utils::misc::BYTE_BIT_COUNT * (_stage_bytes_received - (WORD_BYTES * 2U))));
    }

    if (++_stage_bytes_received != METADATA_PAYLOAD_BYTES)
    {
        return StreamStatus::Incomplete;
    }

    if (!header_valid(_header) || !_writer.begin(_header, _expected_size))
    {
        return StreamStatus::Invalid;
    }

    _writer_active = true;
    return StreamStatus::Complete;
}

StreamStatus DfuStreamParser::process_payload(const uint8_t data)
{
    const std::array<uint8_t, 1> payload = { data };

    if (!_writer.write(payload))
    {
        return StreamStatus::Invalid;
    }

    _bytes_written++;

    if (_bytes_written == _expected_size)
    {
        return StreamStatus::Complete;
    }

    return StreamStatus::Incomplete;
}

StreamStatus DfuStreamParser::process_end(const uint8_t data)
{
    if (expected_byte(END_COMMAND, _stage_bytes_received) != data)
    {
        return StreamStatus::Invalid;
    }

    if (++_stage_bytes_received == WORD_BYTES)
    {
        return _writer.finish() ? StreamStatus::Complete : StreamStatus::Invalid;
    }

    return StreamStatus::Incomplete;
}

StreamStatus DfuStreamParser::process_current_stage(const uint8_t data)
{
    switch (_stage)
    {
    case ReceiveStage::Start:
        return process_start(data);

    case ReceiveStage::Metadata:
        return process_metadata(data);

    case ReceiveStage::Payload:
        return process_payload(data);

    case ReceiveStage::End:
        return process_end(data);

    case ReceiveStage::Done:
    case ReceiveStage::Count:
        break;
    }

    return StreamStatus::Invalid;
}

StreamStatus DfuStreamParser::advance_stage()
{
    if (_stage == ReceiveStage::End)
    {
        _stage         = ReceiveStage::Done;
        _writer_active = false;
        return StreamStatus::Complete;
    }

    _stage_bytes_received = 0;
    _stage                = static_cast<ReceiveStage>(static_cast<uint8_t>(_stage) + 1U);

    return StreamStatus::Incomplete;
}

StreamStatus DfuStreamParser::reject()
{
    if (_writer_active)
    {
        _writer.abort();
    }

    reset();
    _status = StreamStatus::Invalid;

    return StreamStatus::Invalid;
}
