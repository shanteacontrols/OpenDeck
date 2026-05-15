/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/webconfig/common.h"
#include "firmware/src/protocol/webconfig/deps.h"
#include "firmware/src/protocol/webconfig/firmware_upload/firmware_upload.h"
#include "firmware/src/protocol/base.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/staged_update/staged_update.h"
#include "firmware/src/threads.h"

#include "zlibs/utils/midi/midi.h"
#include "zlibs/utils/misc/kwork_delayable.h"
#include "zlibs/utils/misc/mutex.h"

#include <zephyr/kernel.h>

#include <array>
#include <atomic>
#include <span>

namespace opendeck::protocol::webconfig
{
    /**
     * @brief Browser-facing WebSocket configuration endpoint.
     */
    class WebConfig : public protocol::Base
    {
        public:
        WebConfig(Hwa& hwa, staged_update::StagedUpdate& staged_update);
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
        Hwa&                                   _hwa;
        k_sem                                  _client_wakeup             = {};
        std::atomic<int>                       _client_socket             = -1;
        std::atomic<bool>                      _shutdown                  = false;
        std::atomic<bool>                      _server_running            = false;
        std::atomic<bool>                      _network_identity_received = false;
        threads::WebConfigThread               _client_thread;
        zlibs::utils::misc::Mutex              _send_mutex;
        std::array<uint8_t, FRAME_BUFFER_SIZE> _rx_buffer        = {};
        std::array<uint8_t, FRAME_BUFFER_SIZE> _response_buffer  = {};
        signaling::NetworkIdentitySignal       _network_identity = {};
        size_t                                 _response_size    = 0;
        FirmwareUploadHandler                  _firmware_upload_handler;
        zlibs::utils::misc::KworkDelayable     _reboot_work;

        /**
         * @brief Worker loop that forwards binary WebSocket frames to the config parser.
         */
        void client_loop();

        /**
         * @brief Stops the WebConfig server and client worker.
         */
        bool stop();

        /**
         * @brief Closes the active WebSocket client, if one is connected.
         */
        void close_client();

        /**
         * @brief Closes a specific WebSocket client if it is still active.
         *
         * @param socket Socket descriptor expected to be the active client.
         */
        void close_client(int socket);

        /**
         * @brief Publishes one binary WebSocket frame as a SysEx configuration request.
         *
         * @param data Frame payload bytes received from the active client.
         */
        void publish_config_request(std::span<const uint8_t> data);

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
         */
        void handle_command_frame(std::span<const uint8_t> data);

        /**
         * @brief Schedules a reboot so the bootloader can consume staged firmware.
         */
        void schedule_firmware_reboot();

        /**
         * @brief Serializes and sends one SysEx configuration response packet.
         *
         * @param packet UMP response packet emitted by SysExConf.
         */
        void send_response_packet(const midi_ump& packet);

        /**
         * @brief Sends one OSC packet to the browser as raw OSC bytes.
         *
         * @param packet OSC packet bytes observed by the OSC protocol.
         */
        void send_osc_packet(std::span<const uint8_t> packet);

        /**
         * @brief Sends one binary WebSocket frame to the active client.
         *
         * @param data Complete frame payload.
         */
        void send_binary_frame(std::span<const uint8_t> data);

        /**
         * @brief Sends and clears the accumulated SysEx response frame.
         */
        void flush_response();
    };
}    // namespace opendeck::protocol::webconfig
