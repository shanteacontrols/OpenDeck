/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/websockets/handler/sysex_config/sysex_config_handler.h"

#include "zlibs/utils/midi/midi.h"

#include <zephyr/logging/log.h>

using namespace opendeck::protocol::websockets::sysex_config;

namespace
{
    LOG_MODULE_REGISTER(websockets_sysex_config, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    void publish_network_traffic(opendeck::signaling::SignalDirection direction)
    {
        opendeck::signaling::publish(opendeck::signaling::TrafficSignal{
            .transport = opendeck::signaling::TrafficTransport::Network,
            .direction = direction,
        });
    }
}    // namespace

void SysexConfigHandler::init(opendeck::common::protocols::websockets::HandlerEndpoint& endpoint)
{
    _destination = &endpoint;

    opendeck::signaling::subscribe<opendeck::signaling::ConfigResponseSignal>(
        [this](const opendeck::signaling::ConfigResponseSignal& response)
        {
            if (response.transport != opendeck::signaling::ConfigTransport::WebSockets)
            {
                return;
            }

            send_response_packet(response.packet, response.session_id);
        });
}

std::optional<std::span<const uint8_t>> SysexConfigHandler::handle_frame(std::span<const uint8_t> data, uint32_t session_id)
{
    if (data.empty() || (data.front() != zlibs::utils::midi::SYS_EX_START))
    {
        return std::nullopt;
    }

    if (data.size() > opendeck::signaling::ConfigRequestSignal::DATA_SIZE)
    {
        LOG_WRN_ONCE("Ignoring oversized WebSockets SysEx frame");
        publish_network_traffic(opendeck::signaling::SignalDirection::In);
        return std::span<const uint8_t>();
    }

    opendeck::signaling::ConfigRequestSignal request(
        opendeck::signaling::ConfigTransport::WebSockets,
        data,
        session_id);

    _active_session_id = session_id;
    _session_active    = true;

    opendeck::signaling::publish(request);
    publish_network_traffic(opendeck::signaling::SignalDirection::In);

    return std::span<const uint8_t>();
}

void SysexConfigHandler::on_close_session(uint32_t session_id)
{
    if (!_session_active || (_active_session_id != session_id))
    {
        return;
    }

    _session_active = false;

    {
        const zlibs::utils::misc::LockGuard lock(_response_lock);
        _response_size = 0;
    }

    opendeck::signaling::publish(opendeck::signaling::ConfigDisconnectSignal{
        .transport  = opendeck::signaling::ConfigTransport::WebSockets,
        .session_id = session_id,
    });
}

void SysexConfigHandler::send_response_packet(const midi_ump& packet, uint32_t session_id)
{
    const zlibs::utils::misc::LockGuard lock(_response_lock);

    if ((_destination == nullptr) || !_destination->session_active(session_id))
    {
        return;
    }

    const bool serialized = zlibs::utils::midi::write_ump_as_midi1_bytes(
        packet,
        zlibs::utils::midi::DEFAULT_RX_GROUP,
        [this](uint8_t data)
        {
            if (_response_size >= _response_buffer.size())
            {
                return false;
            }

            _response_buffer.at(_response_size++) = data;
            return true;
        });

    if (!serialized)
    {
        LOG_WRN_ONCE("Failed to serialize WebSockets response packet");
        _response_size = 0;
        return;
    }

    if ((_response_size > 0) &&
        (_response_buffer.at(_response_size - 1) == zlibs::utils::midi::SYS_EX_END))
    {
        LOG_DBG("Sending WebSockets binary response (%zu bytes)", _response_size);
        _destination->queue_frame(std::span<const uint8_t>(_response_buffer.data(), _response_size), session_id);
        publish_network_traffic(opendeck::signaling::SignalDirection::Out);
        _response_size = 0;
    }
}
