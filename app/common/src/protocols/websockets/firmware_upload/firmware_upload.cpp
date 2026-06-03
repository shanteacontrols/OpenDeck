/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/src/protocols/websockets/firmware_upload/firmware_upload.h"

using namespace opendeck::common::protocols::websockets;

FirmwareUpload::FirmwareUpload(opendeck::common::dfu::writer::DfuWriter& writer)
    : _dfu_stream(writer)
{}

FirmwareUploadAck FirmwareUpload::make_ack(const FirmwareUploadCommand command,
                                           const FirmwareUploadStatus  status,
                                           const uint32_t              bytes_written)
{
    return make_firmware_upload_ack(command, status, bytes_written);
}

std::optional<FirmwareUploadCommandResult> FirmwareUpload::handle(std::span<const uint8_t> data)
{
    if (data.empty())
    {
        return std::nullopt;
    }

    const auto command = static_cast<FirmwareUploadCommand>(data.front());
    const auto payload = data.subspan(1);

    switch (command)
    {
    case FirmwareUploadCommand::Begin:
        return handle_begin(payload);

    case FirmwareUploadCommand::Chunk:
        return handle_chunk(payload);

    case FirmwareUploadCommand::Finish:
        return handle_finish();

    case FirmwareUploadCommand::Abort:
        return handle_abort();

    default:
        return std::nullopt;
    }
}

void FirmwareUpload::abort()
{
    _dfu_stream.abort();
}

FirmwareUploadCommandResult FirmwareUpload::handle_begin(std::span<const uint8_t> payload)
{
    if (!payload.empty())
    {
        return result(FirmwareUploadCommand::Begin, FirmwareUploadStatus::BadRequest);
    }

    if (!_dfu_stream.supported())
    {
        return result(FirmwareUploadCommand::Begin, FirmwareUploadStatus::Unsupported);
    }

    abort();

    return result(FirmwareUploadCommand::Begin, FirmwareUploadStatus::Ok);
}

FirmwareUploadCommandResult FirmwareUpload::handle_chunk(std::span<const uint8_t> payload)
{
    if (payload.empty())
    {
        return result(FirmwareUploadCommand::Chunk, FirmwareUploadStatus::BadRequest);
    }

    const auto status = _dfu_stream.feed(payload);

    return result(FirmwareUploadCommand::Chunk,
                  status == opendeck::common::dfu::dfu_stream_parser::StreamStatus::Invalid ? FirmwareUploadStatus::Failed
                                                                                            : FirmwareUploadStatus::Ok);
}

FirmwareUploadCommandResult FirmwareUpload::handle_finish()
{
    const bool finished = _dfu_stream.status() == opendeck::common::dfu::dfu_stream_parser::StreamStatus::Complete;

    if (!finished)
    {
        const auto command_result = result(FirmwareUploadCommand::Finish, FirmwareUploadStatus::Failed);

        _dfu_stream.abort();

        return command_result;
    }

    return result(FirmwareUploadCommand::Finish, FirmwareUploadStatus::Ok, true);
}

FirmwareUploadCommandResult FirmwareUpload::handle_abort()
{
    abort();

    return result(FirmwareUploadCommand::Abort, FirmwareUploadStatus::Ok);
}

FirmwareUploadCommandResult FirmwareUpload::result(const FirmwareUploadCommand command,
                                                   const FirmwareUploadStatus  status,
                                                   const bool                  finished) const
{
    return FirmwareUploadCommandResult{
        .response = make_ack(command, status),
        .finished = finished,
    };
}

FirmwareUploadAck FirmwareUpload::make_ack(const FirmwareUploadCommand command,
                                           const FirmwareUploadStatus  status) const
{
    return make_ack(command, status, _dfu_stream.bytes_written());
}
