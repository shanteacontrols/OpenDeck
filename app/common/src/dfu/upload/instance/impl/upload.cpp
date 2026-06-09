/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/src/dfu/upload/instance/impl/upload.h"

#include <zephyr/sys/byteorder.h>

using namespace opendeck::common::dfu::upload;

Upload::Upload(writer::DfuWriter& writer)
    : _dfu_stream(writer)
{}

std::optional<Frame> Upload::frame_info(const std::span<const uint8_t> data)
{
    if (data.empty())
    {
        return std::nullopt;
    }

    const auto           command = static_cast<Command>(data.front());
    std::optional<Frame> frame;

    switch (command)
    {
    case Command::Begin:
    case Command::Finish:
    case Command::Abort:
    {
        frame = Frame{
            .command  = command,
            .size     = COMMAND_FRAME_SIZE,
            .complete = true,
        };
    }
    break;

    case Command::Chunk:
    {
        if (data.size() < CHUNK_FRAME_OVERHEAD)
        {
            // Need the full chunk header before payload length can be parsed.
            frame = Frame{
                .command  = command,
                .size     = CHUNK_FRAME_OVERHEAD,
                .complete = false,
            };

            break;
        }

        const uint16_t payload_size = sys_get_le16(&data[1]);

        if (payload_size > CHUNK_SIZE)
        {
            return std::nullopt;
        }

        const size_t total_size = chunk_frame_size(payload_size);

        frame = Frame{
            .command  = command,
            .payload  = data.size() >= total_size ? data.subspan(CHUNK_FRAME_OVERHEAD, payload_size) : std::span<const uint8_t>{},
            .size     = total_size,
            .complete = data.size() >= total_size,
        };
    }
    break;

    default:
    {
        return std::nullopt;
    }
    break;
    }

    return frame;
}

std::optional<CommandResult> Upload::handle(std::span<const uint8_t> data)
{
    if (data.empty())
    {
        return std::nullopt;
    }

    const auto frame = frame_info(data);

    if (!frame)
    {
        return std::nullopt;
    }

    switch (frame->command)
    {
    case Command::Begin:
    {
        if (!frame->complete || (frame->size != data.size()))
        {
            return result(Command::Begin, Status::BadRequest);
        }

        return handle_begin(*frame);
    }
    break;

    case Command::Chunk:
    {
        if (!frame->complete || (frame->size != data.size()))
        {
            return result(Command::Chunk, Status::BadRequest);
        }

        return handle_chunk(*frame);
    }
    break;

    case Command::Finish:
    {
        if (!frame->complete || (frame->size != data.size()))
        {
            return result(Command::Finish, Status::BadRequest);
        }

        return handle_finish(*frame);
    }
    break;

    case Command::Abort:
    {
        if (!frame->complete || (frame->size != data.size()))
        {
            return result(Command::Abort, Status::BadRequest);
        }

        return handle_abort(*frame);
    }
    break;

    default:
        return std::nullopt;
    }
}

void Upload::abort()
{
    _dfu_stream.abort();
}

CommandResult Upload::handle_begin([[maybe_unused]] const Frame& frame)
{
    if (!_dfu_stream.supported())
    {
        return result(Command::Begin, Status::Unsupported);
    }

    abort();

    return result(Command::Begin, Status::Ok);
}

CommandResult Upload::handle_chunk(const Frame& frame)
{
    if (frame.payload.empty())
    {
        return result(Command::Chunk, Status::BadRequest);
    }

    const auto status = _dfu_stream.feed(frame.payload);

    return result(Command::Chunk,
                  status == opendeck::common::dfu::dfu_stream_parser::StreamStatus::Invalid ? Status::Failed : Status::Ok);
}

CommandResult Upload::handle_finish([[maybe_unused]] const Frame& frame)
{
    const bool finished = _dfu_stream.status() == opendeck::common::dfu::dfu_stream_parser::StreamStatus::Complete;

    if (!finished)
    {
        const auto command_result = result(Command::Finish, Status::Failed);

        _dfu_stream.abort();

        return command_result;
    }

    return result(Command::Finish, Status::Ok, true);
}

CommandResult Upload::handle_abort([[maybe_unused]] const Frame& frame)
{
    abort();

    return result(Command::Abort, Status::Ok);
}

CommandResult Upload::result(const Command command, const Status status, const bool finished) const
{
    return CommandResult{
        .response = make_ack(command, status),
        .finished = finished,
    };
}

Ack Upload::make_ack(const Command command, const Status status) const
{
    return upload::make_ack(command, status, _dfu_stream.bytes_written());
}
