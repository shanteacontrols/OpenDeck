/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/database/instance/impl/database.h"

#include <zephyr/net/socket.h>

#include <cstddef>
#include <sys/types.h>

namespace opendeck::protocol::osc
{
    /**
     * @brief Database view used by the OSC subsystem for global settings.
     */
    using Database = database::User<database::Config::Section::Global>;

    /**
     * @brief Network access used by the OSC subsystem.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Opens one socket.
         *
         * @param family Socket address family.
         * @param type Socket type.
         * @param proto Socket protocol.
         *
         * @return Socket descriptor on success, otherwise a negative value.
         */
        virtual int open_socket(int family, int type, int proto) = 0;

        /**
         * @brief Binds one socket to a local address.
         *
         * @param sock Socket descriptor.
         * @param addr Local address.
         * @param addr_len Local address size.
         *
         * @return `0` on success, otherwise a negative value.
         */
        virtual int bind(int sock, const sockaddr* addr, socklen_t addr_len) = 0;

        /**
         * @brief Sends one datagram.
         *
         * @param sock Socket descriptor.
         * @param buffer Datagram payload.
         * @param len Datagram payload size.
         * @param flags Send flags.
         * @param dest Destination address.
         * @param addrlen Destination address size.
         *
         * @return Number of bytes sent, otherwise a negative value.
         */
        virtual ssize_t send(int             sock,
                             const void*     buffer,
                             size_t          len,
                             int             flags,
                             const sockaddr* dest,
                             socklen_t       addrlen) = 0;

        /**
         * @brief Receives one datagram.
         *
         * @param sock Socket descriptor.
         * @param buffer Receive buffer.
         * @param size Receive buffer size.
         * @param flags Receive flags.
         * @param sender Sender address populated on success.
         * @param sender_len Sender address size.
         *
         * @return Number of bytes received, otherwise a negative value.
         */
        virtual ssize_t receive(int        sock,
                                void*      buffer,
                                size_t     size,
                                int        flags,
                                sockaddr*  sender,
                                socklen_t* sender_len) = 0;

        /**
         * @brief Closes one socket.
         *
         * @param sock Socket descriptor.
         *
         * @return `0` on success, otherwise a negative value.
         */
        virtual int close(int sock) = 0;
    };
}    // namespace opendeck::protocol::osc
