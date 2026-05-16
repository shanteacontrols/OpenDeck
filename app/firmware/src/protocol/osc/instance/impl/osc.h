/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/osc/shared/common.h"
#include "firmware/src/protocol/osc/shared/deps.h"
#include "firmware/src/protocol/osc/packet/packet.h"
#include "firmware/src/protocol/base.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/system/config.h"
#include "firmware/src/threads.h"

#include "zlibs/utils/misc/mutex.h"
#include "zlibs/utils/misc/ring_buffer.h"

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>

#include <atomic>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace opendeck::protocol::osc
{
    /**
     * @brief OSC protocol backend.
     */
    class Osc : public protocol::Base
    {
        public:
        /**
         * @brief Constructs the OSC protocol backend.
         *
         * @param hwa Network backend used to exchange UDP packets.
         * @param database Database interface used to read and update OSC settings.
         */
        Osc(Hwa& hwa, Database& database);
        ~Osc() override = default;

        bool init() override;
        bool deinit() override;

        private:
        /**
         * @brief Queued OSC transmit event generated from a protocol-neutral IO event.
         */
        struct TxEvent
        {
            std::string_view path             = {};
            size_t           component_index  = 0;
            int32_t          value            = 0;
            bool             float_value      = false;
            float            normalized_value = 0.0F;
            bool             control          = false;
            bool             shutdown         = false;
        };

        Hwa&                                                          _hwa;
        Database&                                                     _database;
        zlibs::utils::misc::RingBuffer<TX_QUEUE_SIZE, false, TxEvent> _queue = {};
        zlibs::utils::misc::Mutex                                     _queue_lock;
        k_sem                                                         _send_wakeup               = {};
        k_sem                                                         _read_wakeup               = {};
        std::atomic<int>                                              _listen_sock               = -1;
        std::atomic<bool>                                             _initialized               = false;
        std::atomic<bool>                                             _shutdown                  = false;
        std::atomic<bool>                                             _network_identity_received = false;
        signaling::NetworkIdentitySignal                              _network_identity          = {};
        threads::OscReadThread                                        _read_thread;
        threads::OscSendThread                                        _send_thread;

        /**
         * @brief Converts a raw IO signal into a queued OSC event.
         *
         * @param event Raw IO signal published by an IO component.
         *
         * @return `true` when the signal was queued or intentionally ignored.
         */
        bool enqueue_input(const signaling::OscIoSignal& event);

        /**
         * @brief Returns whether OSC output is enabled in the database.
         *
         * @return `true` when OSC is enabled, otherwise `false`.
         */
        bool enabled();

        /**
         * @brief Returns whether mDNS has published a network identity.
         *
         * @return `true` after mDNS publishes the first identity signal.
         */
        bool has_network_identity() const;

        /**
         * @brief Main receive loop that blocks on incoming OSC UDP packets.
         */
        void read_loop();

        /**
         * @brief Main send loop that blocks on queued outbound OSC events.
         */
        void send_loop();

        /**
         * @brief Sends one queued event as an OSC integer message.
         *
         * @param event TX event to encode and transmit.
         *
         * @param sock Open UDP socket used for sending.
         *
         * @return `true` when the OSC packet was sent.
         */
        bool send_event(const TxEvent& event, int sock);

        /**
         * @brief Encodes and sends an OSC message with one 32-bit integer argument.
         *
         * @param address OSC address to send.
         * @param value Integer argument value.
         * @param sock Open UDP socket used for sending.
         *
         * @return `true` when the OSC packet was sent.
         */
        bool send_int(OscIndexedAddress address, int32_t value, int sock);

        /**
         * @brief Encodes and sends an OSC message with one normalized float argument.
         *
         * @param address OSC address to send.
         * @param value Normalized float argument value.
         * @param sock Open UDP socket used for sending.
         *
         * @return `true` when the OSC packet was sent.
         */
        bool send_normalized_float(OscIndexedAddress address, float value, int sock);

        /**
         * @brief Encodes and sends an OSC discovery response to one sender.
         *
         * @param sender Endpoint that sent the discovery request.
         * @param sock UDP socket used for the response.
         *
         * @return `true` when the discovery response was sent.
         */
        bool send_discovery_response(const sockaddr_in& sender, int sock);

        /**
         * @brief Sends the OSC device information packet to the configured destination.
         *
         * @return `true` when the announcement was sent.
         */
        bool send_discovery_announcement();

        /**
         * @brief Sends one pre-encoded OSC packet to the configured destination.
         *
         * @param packet Packet bytes to send.
         * @param sock Open UDP socket used for sending.
         *
         * @return `true` when the UDP datagram was sent.
         */
        bool send_packet(std::span<const uint8_t> packet, int sock);

        /**
         * @brief Sends one pre-encoded OSC packet to an explicit endpoint.
         *
         * @param packet Packet bytes to send.
         * @param dest Destination endpoint.
         * @param sock Open UDP socket used for sending.
         *
         * @return `true` when the UDP datagram was sent.
         */
        bool send_packet_to(std::span<const uint8_t> packet, const sockaddr_in& dest, int sock);

        /**
         * @brief Builds the configured OSC destination endpoint.
         *
         * @param dest Endpoint populated on success.
         *
         * @return `true` when the database contains a valid destination.
         */
        bool destination(sockaddr_in& dest);

        /**
         * @brief Opens the UDP socket used for outbound OSC packets.
         *
         * @return Socket descriptor on success, otherwise a negative value.
         */
        int open_send_socket();

        /**
         * @brief Opens the UDP socket used for incoming OSC packets.
         *
         * @return Socket descriptor on success, otherwise a negative value.
         */
        int open_listen_socket();

        /**
         * @brief Receives one pending OSC packet and dispatches supported commands.
         *
         * @param listen_sock Bound UDP listen socket.
         *
         * @return `true` when a packet was received and handled.
         */
        bool receive_packet(int listen_sock);

        /**
         * @brief Returns whether an inbound OSC sender is accepted by current settings.
         *
         * @param sender Sender endpoint reported by the UDP socket.
         *
         * @return `true` when packets from the sender should be processed.
         */
        bool sender_allowed(const sockaddr_in& sender);

        /**
         * @brief Closes the active OSC listen socket and wakes the receive loop.
         */
        void close_listen_socket();

        /**
         * @brief Dispatches one decoded OSC message.
         *
         * @param address OSC address pattern.
         * @param value Integer argument value.
         *
         * @return `true` when the message matched a supported command.
         */
        bool handle_message(std::string_view address, int32_t value);

        /**
         * @brief Serves SysEx configuration reads for OSC settings.
         *
         * @param section Global configuration section being read.
         * @param index Setting index within the section.
         * @param value Output value populated on success.
         *
         * @return Configuration status byte, or `std::nullopt` when the request
         *         does not belong to the OSC subsystem.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Global section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for OSC settings.
         *
         * @param section Global configuration section being written.
         * @param index Setting index within the section.
         * @param value New value requested by the caller.
         *
         * @return Configuration status byte, or `std::nullopt` when the request
         *         does not belong to the OSC subsystem.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::Global section, size_t index, uint16_t value);
    };
}    // namespace opendeck::protocol::osc
