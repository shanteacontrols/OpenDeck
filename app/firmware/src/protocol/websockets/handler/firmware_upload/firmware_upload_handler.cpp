/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/websockets/handler/firmware_upload/firmware_upload_handler.h"
#include "firmware/src/signaling/signaling.h"

#include <zephyr/logging/log.h>

using namespace opendeck::firmware::protocol::websockets::firmware_upload;
using namespace opendeck::firmware;

namespace
{
    LOG_MODULE_REGISTER(websockets_firmware_upload, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    void publish_network_traffic(signaling::SignalDirection direction)
    {
        signaling::publish(signaling::TrafficSignal{
            .transport = signaling::TrafficTransport::Network,
            .direction = direction,
        });
    }
}    // namespace

FirmwareUploadHandler::FirmwareUploadHandler()
    : _firmware_upload(_firmware_builder.instance())
{}

std::optional<std::span<const uint8_t>> FirmwareUploadHandler::handle_frame(std::span<const uint8_t> data, uint32_t session_id)
{
    if (data.empty())
    {
        return std::nullopt;
    }

    const auto command = static_cast<opendeck::common::protocols::websockets::FirmwareUploadCommand>(data.front());

    if ((command == opendeck::common::protocols::websockets::FirmwareUploadCommand::Begin) && (data.size() == 1U))
    {
        LOG_INF("Closing WebSockets SysEx configuration session before firmware upload");

        signaling::publish(signaling::ConfigDisconnectSignal{
            .transport  = signaling::ConfigTransport::WebSockets,
            .session_id = session_id,
        });
    }

    const auto response = _firmware_upload.handle(data);

    if (!response)
    {
        return std::nullopt;
    }

    publish_network_traffic(signaling::SignalDirection::In);

    _response = response->response;

    if (!_response.empty())
    {
        publish_network_traffic(signaling::SignalDirection::Out);
    }

    if (response->finished)
    {
        LOG_INF("Staged firmware upload complete, rebooting to apply update");
        signaling::publish(signaling::SystemSignal{
            .system_event = signaling::SystemEvent::BootloaderRebootReq,
        });
    }

    return std::span<const uint8_t>(_response.data(), _response.size());
}
