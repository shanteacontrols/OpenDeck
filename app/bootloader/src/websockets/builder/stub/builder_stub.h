/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/direct_update_writer/instance/impl/direct_update_writer.h"
#include "bootloader/src/websockets/instance/stub/websockets_stub.h"

namespace opendeck::bootloader::websockets
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
        explicit Builder([[maybe_unused]] direct_update_writer::DirectUpdateWriter& direct_update_writer)
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
}    // namespace opendeck::bootloader::websockets
