/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/osc/shared/deps.h"

#include "zlibs/utils/misc/mutex.h"

#include <zephyr/kernel.h>

#include <algorithm>
#include <deque>
#include <span>
#include <vector>

namespace opendeck::protocol::osc
{
    /**
     * @brief In-memory socket backend used by OSC tests.
     */
    class HwaTest : public Hwa
    {
        public:
        HwaTest()
        {
            k_sem_init(&_receive_wakeup, 0, 1);
        }

        struct SentPacket
        {
            int                  sock = -1;
            std::vector<uint8_t> data = {};
            sockaddr_in          dest = {};
        };

        struct ReceivedPacket
        {
            std::vector<uint8_t> data   = {};
            sockaddr_in          sender = {};
        };

        int open_socket([[maybe_unused]] int family, [[maybe_unused]] int type, [[maybe_unused]] int proto) override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);

            if (!_socket_result)
            {
                return -1;
            }

            return _next_sock++;
        }

        int bind(int sock, const sockaddr* addr, socklen_t addr_len) override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);

            _bound_sock = sock;

            if ((addr != nullptr) && (addr_len == sizeof(sockaddr_in)))
            {
                _bound_addr = *reinterpret_cast<const sockaddr_in*>(addr);
            }

            return _bind_result ? 0 : -1;
        }

        ssize_t send(int                  sock,
                     const void*          buffer,
                     size_t               size,
                     [[maybe_unused]] int flags,
                     const sockaddr*      dest,
                     socklen_t            dest_len) override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);

            if (!_send_result || (buffer == nullptr) || (dest == nullptr) || (dest_len != sizeof(sockaddr_in)))
            {
                return -1;
            }

            auto bytes = std::span<const uint8_t>(static_cast<const uint8_t*>(buffer), size);

            _sent_packets.push_back({
                .sock = sock,
                .data = std::vector<uint8_t>(bytes.begin(), bytes.end()),
                .dest = *reinterpret_cast<const sockaddr_in*>(dest),
            });

            return static_cast<ssize_t>(size);
        }

        ssize_t receive(int                  sock,
                        void*                buffer,
                        size_t               size,
                        [[maybe_unused]] int flags,
                        sockaddr*            sender,
                        socklen_t*           sender_len) override
        {
            if ((buffer == nullptr) || (sender == nullptr) || (sender_len == nullptr) || (*sender_len < sizeof(sockaddr_in)))
            {
                return -1;
            }

            while (true)
            {
                {
                    const zlibs::utils::misc::LockGuard lock(_mutex);

                    _received_sock = sock;

                    if (!_recv_result || socket_closed(sock))
                    {
                        return -1;
                    }

                    if (!_received_packets.empty())
                    {
                        auto packet = _received_packets.front();
                        _received_packets.pop_front();

                        const size_t copied = std::min(size, packet.data.size());
                        std::copy(packet.data.begin(), packet.data.begin() + copied, static_cast<uint8_t*>(buffer));

                        *reinterpret_cast<sockaddr_in*>(sender) = packet.sender;
                        *sender_len                             = sizeof(sockaddr_in);

                        return static_cast<ssize_t>(copied);
                    }
                }

                k_sem_take(&_receive_wakeup, K_FOREVER);
            }
        }

        int close(int sock) override
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _closed_socks.push_back(sock);
            k_sem_give(&_receive_wakeup);
            return 0;
        }

        void push_received(std::span<const uint8_t> data, const sockaddr_in& sender)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);

            _received_packets.push_back({
                .data   = std::vector<uint8_t>(data.begin(), data.end()),
                .sender = sender,
            });

            k_sem_give(&_receive_wakeup);
        }

        void clear_sent_packets()
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _sent_packets.clear();
        }

        void set_socket_result(bool result)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _socket_result = result;
        }

        void set_bind_result(bool result)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _bind_result = result;
        }

        void set_send_result(bool result)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _send_result = result;
        }

        void set_recv_result(bool result)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _recv_result = result;
        }

        std::vector<SentPacket> sent_packets() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _sent_packets;
        }

        size_t sent_packet_count() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _sent_packets.size();
        }

        std::vector<int> closed_socks() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _closed_socks;
        }

        int bound_sock() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _bound_sock;
        }

        sockaddr_in bound_addr() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _bound_addr;
        }

        int received_sock() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _received_sock;
        }

        private:
        bool socket_closed(int sock) const
        {
            return std::find(_closed_socks.begin(), _closed_socks.end(), sock) != _closed_socks.end();
        }

        mutable zlibs::utils::misc::Mutex _mutex;
        k_sem                             _receive_wakeup   = {};
        std::deque<ReceivedPacket>        _received_packets = {};
        std::vector<SentPacket>           _sent_packets     = {};
        std::vector<int>                  _closed_socks     = {};
        sockaddr_in                       _bound_addr       = {};
        int                               _bound_sock       = -1;
        int                               _received_sock    = -1;
        int                               _next_sock        = 1;
        bool                              _socket_result    = true;
        bool                              _bind_result      = true;
        bool                              _send_result      = true;
        bool                              _recv_result      = true;
    };
}    // namespace opendeck::protocol::osc
