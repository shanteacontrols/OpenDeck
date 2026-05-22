/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/protocols/websockets/instance/impl/websockets.h"

#include <zephyr/logging/log.h>

#include <cerrno>
#include <cstring>

using namespace opendeck;
using namespace opendeck::bootloader::protocols::websockets;

namespace
{
    LOG_MODULE_REGISTER(opendeck_bootloader_websockets, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}

WebSockets::WebSockets(opendeck::common::protocols::websockets::Hwa& hwa, bootloader::dfu::direct_update_writer::DirectUpdateWriter& direct_update_writer)
    : _hwa(hwa)
    , _firmware_upload(direct_update_writer)
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
{
    k_sem_init(&_client_wakeup, 0, 1);
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
        LOG_WRN("Bootloader WebSockets HTTP server was already running");
    }

    if ((status < 0) && (status != -EALREADY))
    {
        LOG_ERR("Failed to start bootloader WebSockets HTTP server: %d", status);
        return false;
    }

    _server_running = true;
    _client_thread.run();

    LOG_INF("Bootloader WebSockets DFU endpoint started");

    return true;
}

bool WebSockets::deinit()
{
    return stop();
}

bool WebSockets::stop()
{
    if (!_server_running)
    {
        return true;
    }

    _server_running = false;
    _client_thread.destroy();
    _hwa.stop_server();
    return true;
}

int WebSockets::accept_client(const int socket)
{
    {
        const zlibs::utils::misc::LockGuard lock(_client_state_lock);
        const int                           active_socket = _client_socket.exchange(socket);

        if (active_socket >= 0)
        {
            LOG_WRN("Replacing active bootloader WebSockets client");
            _hwa.unregister(active_socket);
        }
    }

    LOG_INF("Accepted bootloader WebSockets client");
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

        int sock = -1;

        {
            const zlibs::utils::misc::LockGuard lock(_client_state_lock);
            sock = _client_socket.load();
        }

        if (sock < 0)
        {
            continue;
        }

        while (!_shutdown)
        {
            opendeck::common::protocols::websockets::FrameInfo frame_info = {};
            const int                                          received   = _hwa.receive(sock, _rx_buffer, frame_info);

            if (received == -ENOTCONN)
            {
                LOG_INF("Bootloader WebSockets peer disconnected");
                break;
            }

            if (received < 0)
            {
                LOG_ERR("Bootloader WebSockets receive failed: %d", received);
                break;
            }

            if (frame_info.close)
            {
                LOG_INF("Bootloader WebSockets close frame received");
                break;
            }

            if (!frame_info.binary)
            {
                LOG_WRN_ONCE("Ignoring non-binary bootloader WebSockets frame");
                continue;
            }

            if (frame_info.remaining != 0U)
            {
                LOG_WRN_ONCE("Ignoring fragmented bootloader WebSockets frame");
                continue;
            }

            const auto frame    = std::span<const uint8_t>(_rx_buffer.data(), static_cast<size_t>(received));
            const auto response = handle_command_frame(frame);

            if (!response)
            {
                LOG_WRN_ONCE("Ignoring unsupported bootloader WebSockets command frame");
                continue;
            }

            const int sent = _hwa.send(sock, *response);

            if (sent < 0)
            {
                LOG_ERR("Bootloader WebSockets send failed: %d", sent);
                break;
            }
        }

        close_client();
        _firmware_upload.abort();
        LOG_INF("Bootloader WebSockets client disconnected");
    }
}

void WebSockets::close_client()
{
    const zlibs::utils::misc::LockGuard lock(_client_state_lock);
    const int                           sock = _client_socket.exchange(-1);

    if (sock >= 0)
    {
        _hwa.unregister(sock);
    }
}

std::optional<opendeck::common::protocols::websockets::FirmwareUploadAck> WebSockets::handle_command_frame(std::span<const uint8_t> data)
{
    const auto response = _firmware_upload.handle(data);

    if (response && response->finished)
    {
        LOG_INF("Bootloader network DFU upload complete");
    }

    if (!response)
    {
        return std::nullopt;
    }

    return response->response;
}
