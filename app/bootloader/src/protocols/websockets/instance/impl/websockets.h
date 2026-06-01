/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/protocols/websockets/instance/impl/websockets.h"
#include "common/src/protocols/websockets/shared/buffers.h"
#include "common/src/protocols/websockets/shared/firmware_upload.h"

namespace opendeck::bootloader::protocols::websockets
{
    /**
     * @brief Bootloader WebSockets endpoint for network DFU.
     */
    class WebSockets : public opendeck::common::protocols::websockets::BaseWebSockets
    {
        public:
        /**
         * @brief Constructs the endpoint around platform hooks.
         *
         * @param hwa Platform hooks used for WebSocket I/O.
         */
        explicit WebSockets(opendeck::common::protocols::websockets::Hwa& hwa)
            : opendeck::common::protocols::websockets::BaseWebSockets(hwa)
        {}

        ~WebSockets() override
        {
            deinit();
        }

        private:
        using WebSocketsBuffers = opendeck::common::protocols::websockets::Buffers<
            opendeck::common::protocols::websockets::FIRMWARE_UPLOAD_FRAME_SIZE,
            opendeck::common::protocols::websockets::FIRMWARE_UPLOAD_ACK_SIZE,
            4>;

        opendeck::common::protocols::websockets::BuffersBase& buffers() override
        {
            return _buffers;
        }

        WebSocketsBuffers _buffers;
    };
}    // namespace opendeck::bootloader::protocols::websockets
