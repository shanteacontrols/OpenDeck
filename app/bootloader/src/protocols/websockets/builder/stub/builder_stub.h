/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"
#include "bootloader/src/protocols/websockets/instance/stub/websockets_stub.h"

namespace opendeck::bootloader::protocols::websockets
{
    /**
     * @brief Convenience builder for targets without WebSocket DFU.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the stub WebSockets builder.
         *
         * @param direct_update_writer Direct-update writer dependency kept for the shared builder shape.
         */
        explicit Builder([[maybe_unused]] bootloader::dfu::direct_update_writer::DirectUpdateWriter& direct_update_writer)
        {}

        /**
         * @brief Returns the stub WebSockets endpoint.
         *
         * @return Stub WebSockets endpoint.
         */
        WebSockets& instance()
        {
            return _instance;
        }

        private:
        WebSockets _instance;
    };
}    // namespace opendeck::bootloader::protocols::websockets
