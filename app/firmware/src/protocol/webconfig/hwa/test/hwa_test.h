/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/webconfig/deps.h"

#include "zlibs/utils/misc/mutex.h"

#include <zephyr/kernel.h>

#include <algorithm>
#include <deque>
#include <span>
#include <vector>

namespace opendeck::protocol::webconfig
{
    /**
     * @brief In-memory WebConfig backend used by tests.
     */
    class HwaTest : public Hwa
    {
        public:
        /**
         * @brief WebSocket frame queued for the receive side of the test backend.
         */
        struct ReceivedFrame
        {
            std::vector<uint8_t> data = {};
            FrameInfo            info = {};
            int                  ret  = 0;
        };

        /**
         * @brief WebSocket frame captured from the send side of the test backend.
         */
        struct SentFrame
        {
            int                  socket = -1;
            std::vector<uint8_t> data   = {};
        };

        HwaTest()
        {
            k_sem_init(&_receive_wakeup, 0, 1);
        }

        int start_server(WebConfig& endpoint) override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _endpoint = &endpoint;
            return _start_result;
        }

        void stop_server() override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _server_stopped = true;
        }

        int receive(int socket, std::span<uint8_t> buffer, FrameInfo& info) override
        {
            while (true)
            {
                {
                    const zlibs::utils::misc::LockGuard lock(_mutex);

                    if (socket_closed(socket))
                    {
                        return -ENOTCONN;
                    }

                    if (!_received_frames.empty())
                    {
                        auto frame = _received_frames.front();
                        _received_frames.pop_front();

                        if (frame.ret < 0)
                        {
                            return frame.ret;
                        }

                        const size_t copied = std::min(buffer.size(), frame.data.size());
                        std::copy(frame.data.begin(), frame.data.begin() + copied, buffer.begin());
                        info = frame.info;

                        return static_cast<int>(copied);
                    }
                }

                k_sem_take(&_receive_wakeup, K_FOREVER);
            }
        }

        int send(int socket, std::span<const uint8_t> data) override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);

            if (_send_result < 0)
            {
                return _send_result;
            }

            _sent_frames.push_back({
                .socket = socket,
                .data   = std::vector<uint8_t>(data.begin(), data.end()),
            });

            return static_cast<int>(data.size());
        }

        void unregister(int socket) override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _closed_sockets.push_back(socket);
            k_sem_give(&_receive_wakeup);
        }

        void reboot_to_bootloader() override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _reboot_requested = true;
        }

        /**
         * @brief Queues one binary frame for the WebConfig receive loop.
         *
         * @param data Frame payload bytes.
         */
        void push_frame(std::span<const uint8_t> data)
        {
            push_frame(data, FrameInfo{
                                 .binary    = true,
                                 .close     = false,
                                 .remaining = 0,
                             });
        }

        /**
         * @brief Queues one frame for the WebConfig receive loop.
         *
         * @param data Frame payload bytes.
         * @param info Metadata returned with the frame.
         */
        void push_frame(std::span<const uint8_t> data,
                        FrameInfo                info)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _received_frames.push_back({
                .data = std::vector<uint8_t>(data.begin(), data.end()),
                .info = info,
                .ret  = static_cast<int>(data.size()),
            });
            k_sem_give(&_receive_wakeup);
        }

        /**
         * @brief Queues one receive error for the WebConfig receive loop.
         *
         * @param error Negative errno value returned by receive().
         */
        void push_receive_error(int error)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _received_frames.push_back({
                .data = {},
                .info = {},
                .ret  = error,
            });
            k_sem_give(&_receive_wakeup);
        }

        /**
         * @brief Sets the result returned by start_server().
         *
         * @param result Return value to use.
         */
        void set_start_result(int result)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _start_result = result;
        }

        /**
         * @brief Sets the result returned by send().
         *
         * @param result Return value to use.
         */
        void set_send_result(int result)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _send_result = result;
        }

        /**
         * @brief Returns frames captured from send().
         *
         * @return Captured sent frames.
         */
        std::vector<SentFrame> sent_frames() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _sent_frames;
        }

        /**
         * @brief Returns how many frames were sent through this backend.
         *
         * @return Number of captured sent frames.
         */
        size_t sent_frame_count() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _sent_frames.size();
        }

        /**
         * @brief Returns sockets closed through unregister().
         *
         * @return Closed socket descriptors.
         */
        std::vector<int> closed_sockets() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _closed_sockets;
        }

        /**
         * @brief Reports whether stop_server() was called.
         *
         * @return True after stop_server() runs.
         */
        bool server_stopped() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _server_stopped;
        }

        /**
         * @brief Reports whether a bootloader reboot was requested.
         *
         * @return True after reboot_to_bootloader() runs.
         */
        bool reboot_requested() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _reboot_requested;
        }

        private:
        mutable zlibs::utils::misc::Mutex _mutex;
        k_sem                             _receive_wakeup   = {};
        WebConfig*                        _endpoint         = nullptr;
        std::deque<ReceivedFrame>         _received_frames  = {};
        std::vector<SentFrame>            _sent_frames      = {};
        std::vector<int>                  _closed_sockets   = {};
        int                               _start_result     = 0;
        int                               _send_result      = 0;
        bool                              _server_stopped   = false;
        bool                              _reboot_requested = false;

        /**
         * @brief Checks whether the test backend already closed a socket.
         *
         * @param socket Socket descriptor to check.
         *
         * @return True when the socket was closed through unregister().
         */
        bool socket_closed(int socket) const
        {
            return std::find(_closed_sockets.begin(), _closed_sockets.end(), socket) != _closed_sockets.end();
        }
    };
}    // namespace opendeck::protocol::webconfig
