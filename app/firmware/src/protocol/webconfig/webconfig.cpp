/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_WEBCONFIG

#include "firmware/src/protocol/webconfig/webconfig.h"
#include "firmware/src/protocol/webconfig/common.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/system/common.h"

#include "zlibs/utils/midi/midi.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <cerrno>
#include <span>

using namespace opendeck::protocol::webconfig;

namespace zmisc = zlibs::utils::misc;

namespace
{
    LOG_MODULE_REGISTER(webconfig, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

WebConfig::WebConfig(Hwa& hwa, staged_update::StagedUpdate& staged_update)
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
    , _firmware_upload_handler(staged_update)
    , _reboot_work([this]
                   {
                       _hwa.reboot_to_bootloader();
                   },
                   []
                   {
                       return threads::SystemWorkqueue::handle();
                   })
{
    k_sem_init(&_client_wakeup, 0, 1);

    signaling::subscribe<signaling::ConfigResponseSignal>(
        [this](const signaling::ConfigResponseSignal& response)
        {
            if (response.transport != signaling::ConfigTransport::WebConfig)
            {
                return;
            }

            send_response_packet(response.packet);
        });

    signaling::subscribe<signaling::OscSignal>(
        [this](const signaling::OscSignal& event)
        {
            send_osc_packet(event.packet);
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
    _shutdown = false;
    _client_thread.run();

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
    log_endpoint();

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
    _hwa.stop_server();
    return true;
}

int WebConfig::accept_client(int socket)
{
    const int active_socket = _client_socket.load();

    if (active_socket >= 0)
    {
        LOG_WRN("Replacing active WebConfig client");
        close_client(active_socket);
    }

    _client_socket.store(socket);
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

        const int sock = _client_socket.load();

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
                if (_client_socket.load() != sock)
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
                publish_config_request(frame);
            }
            else
            {
                handle_command_frame(frame);
            }
        }

        const bool active_client_disconnected = _client_socket.load() == sock;

        close_client(sock);

        if (active_client_disconnected)
        {
            signaling::publish(signaling::ConfigDisconnectSignal{
                .transport = signaling::ConfigTransport::WebConfig,
            });
        }

        LOG_INF("WebConfig client disconnected");
    }
}

void WebConfig::close_client()
{
    const int sock = _client_socket.exchange(-1);

    if (sock >= 0)
    {
        _hwa.unregister(sock);
    }

    _response_size = 0;
}

void WebConfig::close_client(int socket)
{
    int expected = socket;

    if (!_client_socket.compare_exchange_strong(expected, -1))
    {
        return;
    }

    _hwa.unregister(socket);
    _response_size = 0;
}

void WebConfig::publish_config_request(std::span<const uint8_t> data)
{
    signaling::publish(signaling::ConfigRequestSignal{
        .transport = signaling::ConfigTransport::WebConfig,
        .data      = data,
    });
}

void WebConfig::handle_network_identity(const signaling::NetworkIdentitySignal& identity)
{
    _network_identity          = identity;
    _network_identity_received = true;
    log_endpoint();
}

void WebConfig::log_endpoint() const
{
    if (!_server_running || !_network_identity_received)
    {
        return;
    }

    [[maybe_unused]] const auto network_name = _network_identity.name();
    const auto                  ip_address   = _network_identity.ipv4_address();

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

void WebConfig::handle_command_frame(std::span<const uint8_t> data)
{
    if (data.empty())
    {
        LOG_WRN_ONCE("Ignoring empty WebConfig command frame");
        return;
    }

#ifndef CONFIG_PROJECT_TARGET_SUPPORT_STAGED_UPDATE
    if (static_cast<FirmwareUploadCommand>(data.front()) == FirmwareUploadCommand::Begin)
    {
        send_binary_frame(FirmwareUploadHandler::make_ack(
            FirmwareUploadCommand::Begin,
            FirmwareUploadStatus::Unsupported,
            0));
        return;
    }
#endif

    const auto response = _firmware_upload_handler.handle(data);

    if (!response)
    {
        LOG_WRN_ONCE("Ignoring unsupported WebConfig command frame");
        return;
    }

    send_binary_frame(*response);

    if (_firmware_upload_handler.take_reboot_request())
    {
        schedule_firmware_reboot();
    }
}

void WebConfig::schedule_firmware_reboot()
{
    LOG_INF("Staged firmware upload complete, rebooting to apply update");
    _reboot_work.reschedule(sys::REBOOT_DELAY_MS);
}

void WebConfig::send_response_packet(const midi_ump& packet)
{
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
        flush_response();
    }
}

void WebConfig::send_osc_packet(std::span<const uint8_t> packet)
{
    if (packet.empty())
    {
        return;
    }

    send_binary_frame(packet);
}

void WebConfig::send_binary_frame(std::span<const uint8_t> data)
{
    const zmisc::LockGuard lock(_send_mutex);
    const int              sock = _client_socket.load();

    if ((sock < 0) || data.empty())
    {
        return;
    }

    const int sent = _hwa.send(sock, data);

    if (sent < 0)
    {
        LOG_ERR("WebConfig send failed: %d", sent);
        return;
    }

    signaling::publish(signaling::TrafficSignal{
        .transport = signaling::TrafficTransport::Network,
        .direction = signaling::SignalDirection::Out,
    });
}

void WebConfig::flush_response()
{
    if (_response_size == 0)
    {
        return;
    }

    send_binary_frame(std::span<const uint8_t>(_response_buffer.data(), _response_size));

    _response_size = 0;
}

#endif
