/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/websockets/instance/impl/websockets.h"
#include "common/src/protocols/websockets/handler/handler.h"
#include "firmware/src/signaling/signaling.h"

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

WebSockets::WebSockets(opendeck::common::protocols::websockets::Hwa& hwa)
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
{
    k_sem_init(&_client_wakeup, 0, 1);
    k_sem_init(&_tx_wakeup, 0, 1);

    init_handlers();
}

WebSockets::~WebSockets()
{
    stop();
    opendeck::common::protocols::websockets::Handler::clear_handlers();
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
            opendeck::common::protocols::websockets::FrameInfo frame_info = {};
            const int                                          received   = _hwa.receive(sock, _buffers._rx_buffer, frame_info);

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

            const auto frame = std::span<const uint8_t>(_buffers._rx_buffer.data(), static_cast<size_t>(received));

            handle_command_frame(frame, client_generation);
        }

        close_client(sock, client_generation);

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
    int      sock       = -1;
    uint32_t generation = 0;

    {
        const zmisc::LockGuard send_lock(_send_mutex);
        const zmisc::LockGuard state_lock(_client_state_lock);

        sock       = _client_socket.exchange(-1);
        generation = _client_generation;
        ++_client_generation;

        if (sock >= 0)
        {
            _hwa.unregister(sock);
        }
    }

    clear_tx();

    if (sock >= 0)
    {
        close_session(generation);
    }
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
        close_session(generation);
    }

    return closed;
}

void WebSockets::close_session(uint32_t session_id)
{
    for (auto* handler : opendeck::common::protocols::websockets::Handler::handlers())
    {
        if (handler != nullptr)
        {
            handler->on_close_session(session_id);
        }
    }
}

void WebSockets::clear_tx()
{
    _buffers.reset();
}

void WebSockets::handle_command_frame(std::span<const uint8_t> data, uint32_t session_id)
{
    if (data.empty())
    {
        LOG_WRN_ONCE("Ignoring empty WebSockets command frame");
        return;
    }

    for (auto* handler : opendeck::common::protocols::websockets::Handler::handlers())
    {
        if (handler == nullptr)
        {
            continue;
        }

        const auto response = handler->handle_frame(data, session_id);

        if (response)
        {
            if (!response->empty())
            {
                queue_frame(*response, session_id);
            }

            return;
        }
    }

    LOG_WRN_ONCE("Ignoring unsupported WebSockets command frame");
}

void WebSockets::init_handlers()
{
    for (auto* handler : opendeck::common::protocols::websockets::Handler::handlers())
    {
        if (handler != nullptr)
        {
            handler->init(*this);
        }
    }
}

bool WebSockets::session_active(uint32_t session_id)
{
    const zmisc::LockGuard lock(_client_state_lock);

    return (_client_socket.load() >= 0) &&
           (_client_generation == session_id);
}

void WebSockets::queue_frame(std::span<const uint8_t> data, uint32_t session_id)
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

    Buffers::TxFrame frame = {
        .size       = data.size(),
        .socket     = sock,
        .session_id = session_id,
    };

    if (data.size() > frame.data.size())
    {
        LOG_WRN_ONCE("Ignoring oversized WebSockets TX frame");
        return;
    }

    std::copy(data.begin(), data.end(), frame.data.begin());

    if (!_buffers.insert(frame))
    {
        LOG_WRN_ONCE("WebSockets TX queue full, dropping frame");
        return;
    }

    k_sem_give(&_tx_wakeup);
}

void WebSockets::queue_frame(std::span<const uint8_t> data)
{
    uint32_t session_id = signaling::CONFIG_SESSION_ID_DEFAULT;

    {
        const zmisc::LockGuard lock(_client_state_lock);
        session_id = _client_generation;
    }

    queue_frame(data, session_id);
}

void WebSockets::process_tx_queue()
{
    while (true)
    {
        std::optional<Buffers::TxFrame> queued = {};

        queued = _buffers.remove();

        if (!queued.has_value())
        {
            break;
        }

        const zmisc::LockGuard lock(_send_mutex);

        {
            const zmisc::LockGuard state_lock(_client_state_lock);

            if ((_client_socket.load() != queued->socket) ||
                (_client_generation != queued->session_id))
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
    }
}
