/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/websockets/hwa/hw/hwa_hw.h"
#include "firmware/src/dfu/staged_update_writer/builder/builder.h"
#include "firmware/src/protocol/websockets/instance/impl/websockets.h"

namespace opendeck::protocol::websockets
{
    /**
     * @brief Convenience builder that wires the WebSocket configuration endpoint.
     */
    class Builder
    {
        public:
        Builder()
            : _instance(_hwa, _firmware_builder.instance())
        {}

        /**
         * @brief Returns the configured WebSockets protocol instance.
         *
         * @return WebSockets protocol instance.
         */
        WebSockets& instance()
        {
            return _instance;
        }

        private:
        firmware::dfu::staged_update_writer::Builder _firmware_builder;
        HwaHw                                        _hwa;
        WebSockets                                   _instance;
    };
}    // namespace opendeck::protocol::websockets
