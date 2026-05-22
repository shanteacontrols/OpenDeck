/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_WEBCONFIG

#include "firmware/src/protocol/webconfig/instance/impl/webconfig.h"
#include "firmware/src/protocol/webconfig/shared/common.h"
#include "firmware/src/protocol/osc/packet/packet.h"
#include "firmware/src/signaling/signaling.h"

#include "zlibs/utils/midi/midi.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <cerrno>
#include <optional>
#include <span>

using namespace opendeck::protocol::webconfig;

namespace zmisc = zlibs::utils::misc;

namespace
{
    LOG_MODULE_REGISTER(webconfig, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

WebConfig::WebConfig(Hwa& hwa, staged_update_writer::StagedUpdateWriter& staged_update_writer)
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
    , _firmware_upload_handler(staged_update_writer)
{
    k_sem_init(&_client_wakeup, 0, 1);
    k_sem_init(&_tx_wakeup, 0, 1);

    signaling::subscribe<signaling::ConfigResponseSignal>(
        [this](const signaling::ConfigResponseSignal& response)
        {
            if (response.transport != signaling::ConfigTransport::WebConfig)
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

WebConfig::~WebConfig()
{
    stop();
}

bool WebConfig::init()
{
    _shutdown        = false;
    const int status = _hwa.start_server(*this);

    if (status == -EALREADY)
    {
        LOG_WRN("WebConfig HTTP server was already running");
    }

    if ((status < 0) && (status != -EALREADY))
    {
        LOG_ERR("Failed to start WebConfig HTTP server: %d", status);
        return false;
    }

    _server_running = true;
    _client_thread.run();
    _tx_thread.run();

    return true;
}

bool WebConfig::deinit()
{
    return stop();
}

bool WebConfig::stop()
{
    _server_running = false;
    _client_thread.destroy();
    _tx_thread.destroy();
    _hwa.stop_server();
    return true;
}

int WebConfig::accept_client(int socket)
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
        LOG_WRN("Replacing active WebConfig client");
        close_client(active_socket, active_generation);
    }

    {
        const zmisc::LockGuard lock(_client_state_lock);
        ++_client_generation;
        _client_socket.store(socket);
    }

    LOG_INF("Accepted WebConfig client");
    k_sem_give(&_client_wakeup);

    return 0;
}

void WebConfig::client_loop()
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
            FrameInfo frame_info = {};
            const int received   = _hwa.receive(sock, _rx_buffer, frame_info);

            if (received == -ENOTCONN)
            {
                LOG_INF("WebConfig peer disconnected");
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
                    LOG_INF("WebConfig client superseded");
                }
                else
                {
                    LOG_ERR("WebConfig receive failed: %d", received);
                }

                break;
            }

            if (frame_info.close)
            {
                LOG_INF("WebConfig close frame received");
                break;
            }

            if (!frame_info.binary)
            {
                LOG_WRN_ONCE("Ignoring non-binary WebConfig frame");
                continue;
            }

            if (frame_info.remaining != 0U)
            {
                LOG_WRN_ONCE("Ignoring fragmented WebConfig frame");
                continue;
            }

            LOG_DBG("Received WebConfig binary frame (%d bytes)", received);

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
                .transport  = signaling::ConfigTransport::WebConfig,
                .session_id = client_generation,
            });
        }

        LOG_INF("WebConfig client disconnected");
    }
}

void WebConfig::tx_loop()
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

void WebConfig::close_client()
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

bool WebConfig::close_client(int socket, uint32_t generation)
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

void WebConfig::clear_tx()
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

void WebConfig::publish_config_request(std::span<const uint8_t> data, uint32_t session_id)
{
    if (data.empty())
    {
        LOG_WRN_ONCE("Ignoring empty WebConfig SysEx frame");
        return;
    }

    if (data.size() > signaling::ConfigRequestSignal::DATA_SIZE)
    {
        LOG_WRN_ONCE("Ignoring oversized WebConfig SysEx frame");
        return;
    }

    signaling::ConfigRequestSignal request(signaling::ConfigTransport::WebConfig, data, session_id);

    signaling::publish(request);
}

void WebConfig::handle_network_identity(const signaling::NetworkIdentitySignal& identity)
{
    {
        const zmisc::LockGuard lock(_network_identity_lock);
        _network_identity          = identity;
        _network_identity_received = true;
    }

    log_endpoint();
}

void WebConfig::log_endpoint() const
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
        LOG_WRN("WebConfig network identity has no address: %.*s",
                static_cast<int>(network_name.size()),
                network_name.data());
        return;
    }

    LOG_INF("WebConfig endpoint: ws://%.*s/config (%.*s)",
            static_cast<int>(network_name.size()),
            network_name.data(),
            static_cast<int>(ip_address.size()),
            ip_address.data());
}

void WebConfig::handle_command_frame(std::span<const uint8_t> data, uint32_t session_id)
{
    if (data.empty())
    {
        LOG_WRN_ONCE("Ignoring empty WebConfig command frame");
        return;
    }

    const auto command = static_cast<FirmwareUploadCommand>(data.front());

    if ((command == FirmwareUploadCommand::Begin) && (data.size() == 1U))
    {
        LOG_INF("Closing WebConfig SysEx configuration session before firmware upload");

        signaling::publish(signaling::ConfigDisconnectSignal{
            .transport  = signaling::ConfigTransport::WebConfig,
            .session_id = session_id,
        });
    }

#ifndef CONFIG_PROJECT_TARGET_SUPPORT_STAGED_UPDATE
    if (command == FirmwareUploadCommand::Begin)
    {
        queue_binary_frame(FirmwareUploadHandler::make_ack(
                               FirmwareUploadCommand::Begin,
                               FirmwareUploadStatus::Unsupported,
                               0),
                           session_id);
        return;
    }
#endif

    const auto response = _firmware_upload_handler.handle(data);

    if (!response)
    {
        LOG_WRN_ONCE("Ignoring unsupported WebConfig command frame");
        return;
    }

    queue_binary_frame(*response, session_id);

    if (_firmware_upload_handler.take_reboot_request())
    {
        schedule_firmware_reboot();
    }
}

void WebConfig::schedule_firmware_reboot()
{
    LOG_INF("Staged firmware upload complete, rebooting to apply update");
    signaling::publish(signaling::SystemSignal{
        .system_event = signaling::SystemEvent::BootloaderRebootReq,
    });
}

void WebConfig::send_response_packet(const midi_ump& packet, uint32_t session_id)
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
        LOG_WRN_ONCE("Failed to serialize WebConfig response packet");
        _response_size = 0;
        return;
    }

    if ((_response_size > 0) &&
        (_response_buffer.at(_response_size - 1) == zlibs::utils::midi::SYS_EX_END))
    {
        LOG_DBG("Sending WebConfig binary response (%zu bytes)", _response_size);
        queue_binary_frame(std::span<const uint8_t>(_response_buffer.data(), _response_size), session_id);
        _response_size = 0;
    }
}

bool WebConfig::client_session_active(uint32_t session_id)
{
    const zmisc::LockGuard lock(_client_state_lock);

    return (_client_socket.load() >= 0) &&
           (_client_generation == session_id);
}

void WebConfig::mirror_osc_packet(const signaling::OscIoSignal& signal)
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

void WebConfig::queue_binary_frame(std::span<const uint8_t> data, uint32_t session_id)
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
        LOG_WRN_ONCE("Ignoring oversized WebConfig TX frame");
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
        LOG_WRN_ONCE("WebConfig TX queue full, dropping frame");
        return;
    }

    k_sem_give(&_tx_wakeup);
}

void WebConfig::process_tx_queue()
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
            LOG_ERR("WebConfig send failed: %d", sent);
            continue;
        }

        signaling::publish(signaling::TrafficSignal{
            .transport = signaling::TrafficTransport::Network,
            .direction = signaling::SignalDirection::Out,
        });
    }
}

#endif
