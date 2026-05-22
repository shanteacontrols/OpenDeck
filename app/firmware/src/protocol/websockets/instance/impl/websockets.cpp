/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS

#include "firmware/src/protocol/websockets/instance/impl/websockets.h"
#include "firmware/src/protocol/osc/packet/packet.h"
#include "firmware/src/signaling/signaling.h"

#include "zlibs/utils/midi/midi.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <cerrno>
#include <optional>
#include <span>

using namespace opendeck::protocol::websockets;

namespace zmisc = zlibs::utils::misc;

namespace
{
    LOG_MODULE_REGISTER(websockets, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

WebSockets::WebSockets(opendeck::websockets::Hwa& hwa, staged_update_writer::StagedUpdateWriter& staged_update_writer)
    : _hwa(hwa)
    , _client_thread(
          [this]()
          {
              client_loop();
          },
          [this]()
          {
              _shutdown = true;
              close_client();
              k_sem_give(&_client_wakeup);
          })
    , _tx_thread(
          [this]()
          {
              tx_loop();
          },
          [this]()
          {
              _shutdown = true;
              k_sem_give(&_tx_wakeup);
          })
    , _firmware_upload(staged_update_writer)
{
    k_sem_init(&_client_wakeup, 0, 1);
    k_sem_init(&_tx_wakeup, 0, 1);

    signaling::subscribe<signaling::ConfigResponseSignal>(
        [this](const signaling::ConfigResponseSignal& response)
        {
            if (response.transport != signaling::ConfigTransport::WebSockets)
            {
                return;
            }

            send_response_packet(response.packet, response.session_id);
        });

    signaling::subscribe<signaling::OscIoSignal>(
        [this](const signaling::OscIoSignal& event)
        {
            mirror_osc_packet(event);
        });

    signaling::subscribe<signaling::NetworkIdentitySignal>(
        [this](const signaling::NetworkIdentitySignal& identity)
        {
            handle_network_identity(identity);
        });
}

WebSockets::~WebSockets()
{
    stop();
}

bool WebSockets::init()
{
    _shutdown        = false;
    const int status = _hwa.start_server(*this);

    if (status == -EALREADY)
    {
        LOG_WRN("WebSockets HTTP server was already running");
    }

    if ((status < 0) && (status != -EALREADY))
    {
        LOG_ERR("Failed to start WebSockets HTTP server: %d", status);
        return false;
    }

    _server_running = true;
    _client_thread.run();
    _tx_thread.run();

    return true;
}

bool WebSockets::deinit()
{
    return stop();
}

bool WebSockets::stop()
{
    _server_running = false;
    _client_thread.destroy();
    _tx_thread.destroy();
    _hwa.stop_server();
    return true;
}

int WebSockets::accept_client(int socket)
{
    int      active_socket     = -1;
    uint32_t active_generation = 0;

    {
        const zmisc::LockGuard lock(_client_state_lock);
        active_socket     = _client_socket.load();
        active_generation = _client_generation;
    }

    if (active_socket >= 0)
    {
        LOG_WRN("Replacing active WebSockets client");
        close_client(active_socket, active_generation);
    }

    {
        const zmisc::LockGuard lock(_client_state_lock);
        ++_client_generation;
        _client_socket.store(socket);
    }

    LOG_INF("Accepted WebSockets client");
    k_sem_give(&_client_wakeup);

    return 0;
}

void WebSockets::client_loop()
{
    while (true)
    {
        k_sem_take(&_client_wakeup, K_FOREVER);

        if (_shutdown)
        {
            return;
        }

        int      sock              = -1;
        uint32_t client_generation = 0;

        {
            const zmisc::LockGuard lock(_client_state_lock);
            sock              = _client_socket.load();
            client_generation = _client_generation;
        }

        if (sock < 0)
        {
            continue;
        }

        while (!_shutdown)
        {
            opendeck::websockets::FrameInfo frame_info = {};
            const int                       received   = _hwa.receive(sock, _rx_buffer, frame_info);

            if (received == -ENOTCONN)
            {
                LOG_INF("WebSockets peer disconnected");
                break;
            }

            if (received < 0)
            {
                bool superseded = false;

                {
                    const zmisc::LockGuard lock(_client_state_lock);
                    superseded = (_client_socket.load() != sock) ||
                                 (_client_generation != client_generation);
                }

                if (superseded)
                {
                    LOG_INF("WebSockets client superseded");
                }
                else
                {
                    LOG_ERR("WebSockets receive failed: %d", received);
                }

                break;
            }

            if (frame_info.close)
            {
                LOG_INF("WebSockets close frame received");
                break;
            }

            if (!frame_info.binary)
            {
                LOG_WRN_ONCE("Ignoring non-binary WebSockets frame");
                continue;
            }

            if (frame_info.remaining != 0U)
            {
                LOG_WRN_ONCE("Ignoring fragmented WebSockets frame");
                continue;
            }

            LOG_DBG("Received WebSockets binary frame (%d bytes)", received);

            signaling::publish(signaling::TrafficSignal{
                .transport = signaling::TrafficTransport::Network,
                .direction = signaling::SignalDirection::In,
            });

            const auto frame = std::span<const uint8_t>(_rx_buffer.data(), static_cast<size_t>(received));

            if (!frame.empty() && (frame.front() == zlibs::utils::midi::SYS_EX_START))
            {
                publish_config_request(frame, client_generation);
            }
            else
            {
                handle_command_frame(frame, client_generation);
            }
        }

        const bool active_client_disconnected = close_client(sock, client_generation);

        if (active_client_disconnected)
        {
            signaling::publish(signaling::ConfigDisconnectSignal{
                .transport  = signaling::ConfigTransport::WebSockets,
                .session_id = client_generation,
            });
        }

        LOG_INF("WebSockets client disconnected");
    }
}

void WebSockets::tx_loop()
{
    while (true)
    {
        k_sem_take(&_tx_wakeup, K_FOREVER);

        if (_shutdown)
        {
            return;
        }

        process_tx_queue();
    }
}

void WebSockets::close_client()
{
    int sock = -1;

    {
        const zmisc::LockGuard send_lock(_send_mutex);
        const zmisc::LockGuard state_lock(_client_state_lock);

        sock = _client_socket.exchange(-1);
        ++_client_generation;

        if (sock >= 0)
        {
            _hwa.unregister(sock);
        }
    }

    clear_tx();
}

bool WebSockets::close_client(int socket, uint32_t generation)
{
    bool closed = false;

    {
        const zmisc::LockGuard send_lock(_send_mutex);
        const zmisc::LockGuard state_lock(_client_state_lock);
        int                    expected = socket;

        if ((_client_generation != generation) ||
            !_client_socket.compare_exchange_strong(expected, -1))
        {
            return false;
        }

        ++_client_generation;
        _hwa.unregister(socket);
        closed = true;
    }

    if (closed)
    {
        clear_tx();
    }

    return closed;
}

void WebSockets::clear_tx()
{
    {
        const zmisc::LockGuard lock(_response_lock);
        _response_size = 0;
    }

    {
        const zmisc::LockGuard lock(_tx_queue_lock);
        _tx_queue.reset();
    }
}

void WebSockets::publish_config_request(std::span<const uint8_t> data, uint32_t session_id)
{
    if (data.empty())
    {
        LOG_WRN_ONCE("Ignoring empty WebSockets SysEx frame");
        return;
    }

    if (data.size() > signaling::ConfigRequestSignal::DATA_SIZE)
    {
        LOG_WRN_ONCE("Ignoring oversized WebSockets SysEx frame");
        return;
    }

    signaling::ConfigRequestSignal request(signaling::ConfigTransport::WebSockets, data, session_id);

    signaling::publish(request);
}

void WebSockets::handle_network_identity(const signaling::NetworkIdentitySignal& identity)
{
    {
        const zmisc::LockGuard lock(_network_identity_lock);
        _network_identity          = identity;
        _network_identity_received = true;
    }

    log_endpoint();
}

void WebSockets::log_endpoint() const
{
    signaling::NetworkIdentitySignal identity = {};

    {
        const zmisc::LockGuard lock(_network_identity_lock);

        if (!_server_running || !_network_identity_received)
        {
            return;
        }

        identity = _network_identity;
    }

    [[maybe_unused]] const auto network_name = identity.name();
    const auto                  ip_address   = identity.ipv4_address();

    if (ip_address.empty())
    {
        LOG_WRN("WebSockets network identity has no address: %.*s",
                static_cast<int>(network_name.size()),
                network_name.data());
        return;
    }

    LOG_INF("WebSockets endpoint: ws://%.*s/config (%.*s)",
            static_cast<int>(network_name.size()),
            network_name.data(),
            static_cast<int>(ip_address.size()),
            ip_address.data());
}

void WebSockets::handle_command_frame(std::span<const uint8_t> data, uint32_t session_id)
{
    if (data.empty())
    {
        LOG_WRN_ONCE("Ignoring empty WebSockets command frame");
        return;
    }

    const auto command = static_cast<opendeck::websockets::FirmwareUploadCommand>(data.front());

    if ((command == opendeck::websockets::FirmwareUploadCommand::Begin) && (data.size() == 1U))
    {
        LOG_INF("Closing WebSockets SysEx configuration session before firmware upload");

        signaling::publish(signaling::ConfigDisconnectSignal{
            .transport  = signaling::ConfigTransport::WebSockets,
            .session_id = session_id,
        });
    }

#ifndef CONFIG_PROJECT_TARGET_SUPPORT_STAGED_UPDATE
    if (command == opendeck::websockets::FirmwareUploadCommand::Begin)
    {
        queue_binary_frame(opendeck::websockets::FirmwareUpload::make_ack(
                               opendeck::websockets::FirmwareUploadCommand::Begin,
                               opendeck::websockets::FirmwareUploadStatus::Unsupported,
                               0),
                           session_id);
        return;
    }
#endif

    const auto response = _firmware_upload.handle(data);

    if (!response)
    {
        LOG_WRN_ONCE("Ignoring unsupported WebSockets command frame");
        return;
    }

    queue_binary_frame(response->response, session_id);

    if (response->finished)
    {
        schedule_firmware_reboot();
    }
}

void WebSockets::schedule_firmware_reboot()
{
    LOG_INF("Staged firmware upload complete, rebooting to apply update");
    signaling::publish(signaling::SystemSignal{
        .system_event = signaling::SystemEvent::BootloaderRebootReq,
    });
}

void WebSockets::send_response_packet(const midi_ump& packet, uint32_t session_id)
{
    const zmisc::LockGuard lock(_response_lock);

    if (!client_session_active(session_id))
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
        queue_binary_frame(std::span<const uint8_t>(_response_buffer.data(), _response_size), session_id);
        _response_size = 0;
    }
}

bool WebSockets::client_session_active(uint32_t session_id)
{
    const zmisc::LockGuard lock(_client_state_lock);

    return (_client_socket.load() >= 0) &&
           (_client_generation == session_id);
}

void WebSockets::mirror_osc_packet(const signaling::OscIoSignal& signal)
{
    if (signal.direction != signaling::SignalDirection::Out)
    {
        return;
    }

    protocol::osc::PacketBuffer packet = {};
    const auto                  size   = protocol::osc::make_packet(packet, signal);

    if (!size)
    {
        return;
    }

    uint32_t session_id = signaling::CONFIG_SESSION_ID_DEFAULT;

    {
        const zmisc::LockGuard lock(_client_state_lock);
        session_id = _client_generation;
    }

    queue_binary_frame(std::span<const uint8_t>(packet.data(), *size), session_id);
}

void WebSockets::queue_binary_frame(std::span<const uint8_t> data, uint32_t session_id)
{
    int sock = -1;

    {
        const zmisc::LockGuard lock(_client_state_lock);

        if (_client_generation != session_id)
        {
            return;
        }

        sock = _client_socket.load();
    }

    if (_shutdown || (sock < 0) || data.empty())
    {
        return;
    }

    TxFrame frame = {
        .size              = data.size(),
        .socket            = sock,
        .client_generation = session_id,
    };

    if (data.size() > frame.data.size())
    {
        LOG_WRN_ONCE("Ignoring oversized WebSockets TX frame");
        return;
    }

    std::copy(data.begin(), data.end(), frame.data.begin());

    bool inserted = false;

    {
        const zmisc::LockGuard lock(_tx_queue_lock);
        inserted = _tx_queue.insert(frame);
    }

    if (!inserted)
    {
        LOG_WRN_ONCE("WebSockets TX queue full, dropping frame");
        return;
    }

    k_sem_give(&_tx_wakeup);
}

void WebSockets::process_tx_queue()
{
    while (true)
    {
        std::optional<TxFrame> queued = {};

        {
            const zmisc::LockGuard lock(_tx_queue_lock);
            queued = _tx_queue.remove();
        }

        if (!queued.has_value())
        {
            break;
        }

        const zmisc::LockGuard lock(_send_mutex);

        {
            const zmisc::LockGuard state_lock(_client_state_lock);

            if ((_client_socket.load() != queued->socket) ||
                (_client_generation != queued->client_generation))
            {
                continue;
            }
        }

        const auto data = std::span<const uint8_t>(queued->data.data(), queued->size);
        const int  sent = _hwa.send(queued->socket, data);

        if (sent < 0)
        {
            LOG_ERR("WebSockets send failed: %d", sent);
            continue;
        }

        signaling::publish(signaling::TrafficSignal{
            .transport = signaling::TrafficTransport::Network,
            .direction = signaling::SignalDirection::Out,
        });
    }
}

#endif
