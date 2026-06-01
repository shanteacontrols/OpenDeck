/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/protocols/websockets/handler/firmware_upload/firmware_upload_handler.h"

#include <zephyr/logging/log.h>

using namespace opendeck::bootloader::protocols::websockets::firmware_upload;

namespace
{
    LOG_MODULE_REGISTER(opendeck_bootloader_websockets_firmware_upload, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

FirmwareUploadHandler::FirmwareUploadHandler(bootloader::dfu::direct_update_writer::DirectUpdateWriter& direct_update_writer)
    : _firmware_upload(direct_update_writer)
{}

std::optional<std::span<const uint8_t>> FirmwareUploadHandler::handle_frame(std::span<const uint8_t> data, [[maybe_unused]] uint32_t session_id)
{
    const auto response = _firmware_upload.handle(data);

    if (!response)
    {
        return std::nullopt;
    }

    _response = response->response;

    if (response->finished)
    {
        LOG_INF("Bootloader network DFU upload complete");
    }

    return std::span<const uint8_t>(_response.data(), _response.size());
}

void FirmwareUploadHandler::on_close_session([[maybe_unused]] uint32_t session_id)
{
    _firmware_upload.abort();
}
