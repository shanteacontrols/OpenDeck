/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/osc/instance/impl/common.h"
#include "firmware/src/protocol/osc/shared/common.h"
#include "firmware/src/protocol/osc/instance/impl/deps.h"
#include "firmware/src/protocol/osc/packet/packet.h"
#include "firmware/src/protocol/base.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/system/shared/config.h"
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

namespace opendeck::firmware::protocol::osc
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
            PacketBuffer packet   = {};
            size_t       size     = 0;
            bool         control  = false;
            bool         shutdown = false;
        };

        /**
         * @brief Cached OSC destination endpoint and sender state.
         */
        struct Destination
        {
            sockaddr_in endpoint    = {};
            uint32_t    retry_at_ms = 0;
        };

        Hwa&                                                          _hwa;
        Database&                                                     _database;
        zlibs::utils::misc::RingBuffer<TX_QUEUE_SIZE, false, TxEvent> _queue = {};
        zlibs::utils::misc::Mutex                                     _queue_lock;
        zlibs::utils::misc::Mutex                                     _destination_lock;
        k_sem                                                         _send_wakeup               = {};
        k_sem                                                         _read_wakeup               = {};
        std::atomic<int>                                              _listen_sock               = -1;
        std::atomic<bool>                                             _initialized               = false;
        std::atomic<bool>                                             _shutdown                  = false;
        std::atomic<bool>                                             _network_identity_received = false;
        opendeck::firmware::signaling::NetworkIdentitySignal          _network_identity          = {};
        std::array<std::optional<Destination>, DESTINATION_COUNT>     _destinations              = {};
        threads::OscReadThread                                        _read_thread;
        threads::OscSendThread                                        _send_thread;

        /**
         * @brief Converts a signal into an outbound OSC packet.
         *
         * @param event Signal to publish over OSC.
         *
         * @return `true` when the signal was queued, dropped, or intentionally ignored.
         */
        template<typename Signal>
        bool enqueue(const Signal& event);

        /**
         * @brief Queues one already encoded OSC packet for outbound transport.
         *
         * @param packet Encoded OSC packet bytes.
         * @param size Number of bytes to send from the packet buffer.
         *
         * @return `true` when the packet was queued or intentionally dropped.
         */
        bool enqueue_packet(const PacketBuffer& packet, size_t size);

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
         * @brief Encodes and sends one queued OSC IO event.
         *
         * @param event TX event to encode and transmit.
         * @param sock Open UDP socket used for sending.
         *
         * @return `true` when the OSC packet was sent.
         */
        bool send_event(const TxEvent& event, int sock);

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
         * @brief Builds one OSC destination from persisted configuration.
         *
         * This reads the destination IP octets and port from the database. It does
         * not read or update the runtime destination cache.
         *
         * @param index Zero-based destination slot.
         *
         * @return Destination built from configuration, or empty when index is out of range.
         */
        std::optional<Destination> read_destination_config(size_t index) const;

        /**
         * @brief Returns one cached OSC destination.
         *
         * This reads the runtime cache populated by refresh_destinations(); it does
         * not touch the database.
         *
         * @param index Zero-based destination slot.
         *
         * @return Cached destination, or empty when index is out of range.
         */
        std::optional<Destination> destination(size_t index);

        /**
         * @brief Refreshes cached OSC destination endpoints from the database.
         */
        void refresh_destinations();

        /**
         * @brief Checks whether at least one destination can currently be sent to.
         *
         * @param now Current uptime in milliseconds.
         *
         * @return `true` if at least one configured destination is not in retry backoff.
         */
        bool has_sendable_destination(uint32_t now);

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
         * @brief Clears queued outbound OSC packets after destination configuration changes.
         */
        void reset_send_state();

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
}    // namespace opendeck::firmware::protocol::osc
