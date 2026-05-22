/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/webconfig/shared/common.h"
#include "firmware/src/protocol/webconfig/shared/deps.h"
#include "firmware/src/protocol/webconfig/firmware_upload/firmware_upload.h"
#include "firmware/src/protocol/osc/shared/packet.h"
#include "firmware/src/protocol/base.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/staged_update_writer/instance/impl/staged_update_writer.h"
#include "firmware/src/threads.h"

#include "zlibs/utils/midi/midi.h"
#include "zlibs/utils/misc/mutex.h"
#include "zlibs/utils/misc/ring_buffer.h"

#include <zephyr/kernel.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <span>

namespace opendeck::protocol::webconfig
{
    /**
     * @brief Browser-facing WebSocket configuration endpoint.
     */
    class WebConfig : public protocol::Base
    {
        public:
        WebConfig(Hwa& hwa, staged_update_writer::StagedUpdateWriter& staged_update_writer);
        ~WebConfig() override;

        bool init() override;
        bool deinit() override;

        /**
         * @brief Accepts a WebSocket client handed off by Zephyr's HTTP server.
         *
         * @param socket WebSocket socket descriptor.
         *
         * @return 0 on success, otherwise a negative errno value.
         */
        int accept_client(int socket);

        private:
        static constexpr size_t TX_QUEUE_SIZE = 32;
        static constexpr size_t TX_FRAME_SIZE = std::max(signaling::ConfigRequestSignal::DATA_SIZE,
                                                         osc::PACKET_BUFFER_SIZE);

        struct TxFrame
        {
            std::array<uint8_t, TX_FRAME_SIZE> data              = {};
            size_t                             size              = 0;
            int                                socket            = -1;
            uint32_t                           client_generation = 0;
        };

        Hwa&                                                          _hwa;
        k_sem                                                         _client_wakeup             = {};
        k_sem                                                         _tx_wakeup                 = {};
        std::atomic<int>                                              _client_socket             = -1;
        uint32_t                                                      _client_generation         = 0;
        std::atomic<bool>                                             _shutdown                  = false;
        std::atomic<bool>                                             _server_running            = false;
        bool                                                          _network_identity_received = false;
        threads::WebConfigThread                                      _client_thread;
        threads::WebConfigTxThread                                    _tx_thread;
        zlibs::utils::misc::Mutex                                     _send_mutex;
        zlibs::utils::misc::Mutex                                     _client_state_lock;
        zlibs::utils::misc::Mutex                                     _response_lock;
        mutable zlibs::utils::misc::Mutex                             _network_identity_lock;
        std::array<uint8_t, FIRMWARE_UPLOAD_FRAME_SIZE>               _rx_buffer        = {};
        signaling::ConfigRequestSignal::Data                          _response_buffer  = {};
        signaling::NetworkIdentitySignal                              _network_identity = {};
        size_t                                                        _response_size    = 0;
        FirmwareUploadHandler                                         _firmware_upload_handler;
        zlibs::utils::misc::RingBuffer<TX_QUEUE_SIZE, false, TxFrame> _tx_queue = {};
        zlibs::utils::misc::Mutex                                     _tx_queue_lock;

        /**
         * @brief Worker loop that forwards binary WebSocket frames to the config parser.
         */
        void client_loop();

        /**
         * @brief Worker loop that sends queued binary WebSocket frames.
         */
        void tx_loop();

        /**
         * @brief Stops the WebConfig server and worker threads.
         */
        bool stop();

        /**
         * @brief Closes the active WebSocket client, if one is connected.
         */
        void close_client();

        /**
         * @brief Closes a specific WebSocket client if socket and generation still match.
         *
         * @param socket Socket descriptor expected to be the active client.
         * @param generation Client generation expected to be active.
         *
         * @return True when the client was closed, otherwise false.
         */
        bool close_client(int socket, uint32_t generation);

        /**
         * @brief Drops queued and partially assembled transmit data for disconnected clients.
         */
        void clear_tx();

        /**
         * @brief Publishes one binary WebSocket frame as a SysEx configuration request.
         *
         * @param data Frame payload bytes received from the active client.
         * @param session_id Client generation associated with the received frame.
         */
        void publish_config_request(std::span<const uint8_t> data, uint32_t session_id);

        /**
         * @brief Stores the mDNS identity used for WebConfig endpoint logs.
         *
         * @param identity Network name and IPv4 address published by mDNS.
         */
        void handle_network_identity(const signaling::NetworkIdentitySignal& identity);

        /**
         * @brief Logs the reachable WebConfig endpoint after mDNS publishes it.
         */
        void log_endpoint() const;

        /**
         * @brief Processes one WebConfig command frame.
         *
         * @param data Frame payload bytes received from the active client.
         * @param session_id Client generation associated with the received frame.
         */
        void handle_command_frame(std::span<const uint8_t> data, uint32_t session_id);

        /**
         * @brief Schedules a reboot so the bootloader can consume staged firmware.
         */
        void schedule_firmware_reboot();

        /**
         * @brief Serializes and sends one SysEx configuration response packet.
         *
         * @param packet UMP response packet emitted by SysExConf.
         * @param session_id Client generation associated with the response.
         */
        void send_response_packet(const midi_ump& packet, uint32_t session_id);

        /**
         * @brief Checks whether a WebConfig client session is still active.
         *
         * @param session_id Client generation to validate.
         *
         * @return `true` if the session is still the active client.
         */
        bool client_session_active(uint32_t session_id);

        /**
         * @brief Queues the WebConfig OSC preview packet for one IO event.
         *
         * @param signal IO event that would be sent on the OSC UDP path.
         */
        void mirror_osc_packet(const signaling::OscIoSignal& signal);

        /**
         * @brief Queues one binary WebSocket frame for a specific client session.
         *
         * @param data Complete frame payload.
         * @param session_id Client generation that must still be active.
         */
        void queue_binary_frame(std::span<const uint8_t> data, uint32_t session_id);

        /**
         * @brief Sends queued binary WebSocket frames from worker context.
         */
        void process_tx_queue();
    };
}    // namespace opendeck::protocol::webconfig
