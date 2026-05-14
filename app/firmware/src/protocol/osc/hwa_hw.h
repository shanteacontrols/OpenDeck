/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace opendeck::protocol::osc
{
    /**
     * @brief Zephyr socket backend used by OSC on hardware targets.
     */
    class HwaHw : public Hwa
    {
        public:
        int open_socket(int family, int type, int proto) override
        {
            return zsock_socket(family, type, proto);
        }

        int bind(int sock, const sockaddr* addr, socklen_t addr_len) override
        {
            return zsock_bind(sock, addr, addr_len);
        }

        ssize_t send(int             sock,
                     const void*     buffer,
                     size_t          len,
                     int             flags,
                     const sockaddr* dest,
                     socklen_t       addrlen) override
        {
            return zsock_sendto(sock, buffer, len, flags, dest, addrlen);
        }

        ssize_t receive(int        sock,
                        void*      buffer,
                        size_t     size,
                        int        flags,
                        sockaddr*  sender,
                        socklen_t* sender_len) override
        {
            return zsock_recvfrom(sock, buffer, size, flags, sender, sender_len);
        }

        int close(int sock) override
        {
            return zsock_close(sock);
        }
    };
}    // namespace opendeck::protocol::osc
