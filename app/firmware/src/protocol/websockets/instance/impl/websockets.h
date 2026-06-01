/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/protocols/websockets/instance/impl/websockets.h"
#include "common/src/protocols/websockets/shared/firmware_upload.h"
#include "firmware/src/protocol/base.h"
#include "firmware/src/protocol/osc/shared/packet.h"
#include "firmware/src/signaling/signaling.h"

#include <algorithm>

namespace opendeck::protocol::websockets
{
    /**
     * @brief Firmware protocol wrapper for the shared WebSockets endpoint.
     */
    class WebSockets : public protocol::Base, public opendeck::common::protocols::websockets::BaseWebSockets
    {
        public:
        explicit WebSockets(opendeck::common::protocols::websockets::Hwa& hwa)
            : opendeck::common::protocols::websockets::BaseWebSockets(hwa)
        {}

        ~WebSockets() override
        {
            deinit();
        }

        bool init() override
        {
            return opendeck::common::protocols::websockets::BaseWebSockets::init();
        }

        bool deinit() override
        {
            return opendeck::common::protocols::websockets::BaseWebSockets::deinit();
        }

        private:
        using WebSocketsBuffers = opendeck::common::protocols::websockets::Buffers<
            opendeck::common::protocols::websockets::FIRMWARE_UPLOAD_FRAME_SIZE,
            std::max(opendeck::firmware::signaling::ConfigRequestSignal::DATA_SIZE, opendeck::protocol::osc::PACKET_BUFFER_SIZE),
            32>;

        opendeck::common::protocols::websockets::BuffersBase& buffers() override
        {
            return _buffers;
        }

        WebSocketsBuffers _buffers;
    };
}    // namespace opendeck::protocol::websockets
