/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/protocols/websockets/handler/handler.h"
#include "firmware/src/signaling/signaling.h"

#include "zlibs/utils/midi/midi.h"
#include "zlibs/utils/misc/mutex.h"

namespace opendeck::protocol::websockets::sysex_config
{
    /**
     * @brief Handles SysEx configuration requests received over WebSockets.
     */
    class SysexConfigHandler : public opendeck::common::protocols::websockets::Handler
    {
        public:
        /**
         * @brief Subscribes to SysEx configuration responses.
         *
         * @param endpoint Endpoint used to queue response frames.
         */
        void init(opendeck::common::protocols::websockets::HandlerEndpoint& endpoint) override;

        /**
         * @brief Publishes a SysEx frame as a configuration request.
         *
         * @param data       WebSockets binary frame payload.
         * @param session_id Active WebSockets session id.
         *
         * @return Empty response when the frame was handled, otherwise `std::nullopt`.
         */
        std::optional<std::span<const uint8_t>> handle_frame(std::span<const uint8_t> data, uint32_t session_id) override;

        /**
         * @brief Closes the active WebSockets SysEx configuration session.
         *
         * @param session_id Closed WebSockets session id.
         */
        void on_close_session(uint32_t session_id) override;

        private:
        opendeck::common::protocols::websockets::HandlerEndpoint* _destination     = nullptr;
        signaling::ConfigRequestSignal::Data                      _response_buffer = {};
        zlibs::utils::misc::Mutex                                 _response_lock;
        size_t                                                    _response_size     = 0;
        uint32_t                                                  _active_session_id = 0;
        bool                                                      _session_active    = false;

        /**
         * @brief Serializes and queues one SysEx configuration response packet.
         *
         * @param packet UMP response packet emitted by SysExConf.
         * @param session_id WebSockets session id associated with the response.
         */
        void send_response_packet(const midi_ump& packet, uint32_t session_id);
    };
}    // namespace opendeck::protocol::websockets::sysex_config
