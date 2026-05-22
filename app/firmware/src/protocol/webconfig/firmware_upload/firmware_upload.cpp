/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/webconfig/firmware_upload/firmware_upload.h"

#include <cstring>

using namespace opendeck::protocol::webconfig;

FirmwareUploadHandler::FirmwareUploadHandler(staged_update_writer::StagedUpdateWriter& staged_update_writer)
    : _staged_update(staged_update_writer)
    , _dfu_stream(staged_update_writer)
{}

FirmwareUploadHandler::Response FirmwareUploadHandler::make_ack(const FirmwareUploadCommand command,
                                                                const FirmwareUploadStatus  status,
                                                                const uint32_t              bytes_written)
{
    Response response = {
        static_cast<uint8_t>(FirmwareUploadResponse::Ack),
        static_cast<uint8_t>(command),
        static_cast<uint8_t>(status),
        0,
        0,
        0,
        0,
    };

    std::memcpy(response.data() + 3, &bytes_written, sizeof(bytes_written));

    return response;
}

std::optional<FirmwareUploadHandler::Response> FirmwareUploadHandler::handle(std::span<const uint8_t> data)
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
        return handle_firmware_begin(payload);

    case FirmwareUploadCommand::Chunk:
        return handle_firmware_chunk(payload);

    case FirmwareUploadCommand::Finish:
        return handle_firmware_finish();

    case FirmwareUploadCommand::Abort:
        return handle_firmware_abort();

    default:
        return std::nullopt;
    }
}

bool FirmwareUploadHandler::take_reboot_request()
{
    const bool requested = _reboot_requested;
    _reboot_requested    = false;

    return requested;
}

FirmwareUploadHandler::Response FirmwareUploadHandler::handle_firmware_begin(std::span<const uint8_t> payload)
{
    if (!payload.empty())
    {
        return make_ack(FirmwareUploadCommand::Begin, FirmwareUploadStatus::BadRequest);
    }

    _dfu_stream.reset();

    return make_ack(FirmwareUploadCommand::Begin, FirmwareUploadStatus::Ok);
}

FirmwareUploadHandler::Response FirmwareUploadHandler::handle_firmware_chunk(std::span<const uint8_t> payload)
{
    if (payload.empty())
    {
        return make_ack(FirmwareUploadCommand::Chunk, FirmwareUploadStatus::BadRequest);
    }

    const auto status = _dfu_stream.feed(payload);

    return make_ack(
        FirmwareUploadCommand::Chunk,
        status == dfu_stream::StreamStatus::Invalid ? FirmwareUploadStatus::Failed : FirmwareUploadStatus::Ok);
}

FirmwareUploadHandler::Response FirmwareUploadHandler::handle_firmware_finish()
{
    const bool finished = _dfu_stream.status() == dfu_stream::StreamStatus::Complete;

    if (finished)
    {
        _reboot_requested = true;
    }

    if (!finished)
    {
        _staged_update.abort();
    }

    return make_ack(
        FirmwareUploadCommand::Finish,
        finished ? FirmwareUploadStatus::Ok : FirmwareUploadStatus::Failed);
}

FirmwareUploadHandler::Response FirmwareUploadHandler::handle_firmware_abort()
{
    _staged_update.abort();
    _dfu_stream.reset();
    return make_ack(FirmwareUploadCommand::Abort, FirmwareUploadStatus::Ok);
}

FirmwareUploadHandler::Response FirmwareUploadHandler::make_ack(const FirmwareUploadCommand command,
                                                                const FirmwareUploadStatus  status) const
{
    return make_ack(command, status, _dfu_stream.bytes_written());
}
