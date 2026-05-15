/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/webconfig/firmware_upload/firmware_upload.h"

#include <cstring>

using namespace opendeck::protocol::webconfig;

FirmwareUploadHandler::FirmwareUploadHandler(staged_update::StagedUpdate& staged_update)
    : _staged_update(staged_update)
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
    static constexpr size_t SIZE_FIELD_BYTES = sizeof(uint32_t);

    if (payload.size() != SIZE_FIELD_BYTES)
    {
        return make_ack(FirmwareUploadCommand::Begin, FirmwareUploadStatus::BadRequest);
    }

    uint32_t size = 0;
    std::memcpy(&size, payload.data(), sizeof(size));

    return make_ack(
        FirmwareUploadCommand::Begin,
        _staged_update.begin(size) ? FirmwareUploadStatus::Ok : FirmwareUploadStatus::Failed);
}

FirmwareUploadHandler::Response FirmwareUploadHandler::handle_firmware_chunk(std::span<const uint8_t> payload)
{
    if (payload.empty())
    {
        return make_ack(FirmwareUploadCommand::Chunk, FirmwareUploadStatus::BadRequest);
    }

    return make_ack(
        FirmwareUploadCommand::Chunk,
        _staged_update.write(payload) ? FirmwareUploadStatus::Ok : FirmwareUploadStatus::Failed);
}

FirmwareUploadHandler::Response FirmwareUploadHandler::handle_firmware_finish()
{
    const bool finished = _staged_update.finish();

    if (finished)
    {
        _reboot_requested = true;
    }

    return make_ack(
        FirmwareUploadCommand::Finish,
        finished ? FirmwareUploadStatus::Ok : FirmwareUploadStatus::Failed);
}

FirmwareUploadHandler::Response FirmwareUploadHandler::handle_firmware_abort()
{
    _staged_update.abort();
    return make_ack(FirmwareUploadCommand::Abort, FirmwareUploadStatus::Ok);
}

FirmwareUploadHandler::Response FirmwareUploadHandler::make_ack(const FirmwareUploadCommand command,
                                                                const FirmwareUploadStatus  status) const
{
    return make_ack(command, status, _staged_update.bytes_written());
}
