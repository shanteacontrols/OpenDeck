/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/websockets/shared/deps.h"

#include <cerrno>

namespace opendeck::bootloader::websockets
{
    /**
     * @brief Stub bootloader WebSockets endpoint used when network DFU is disabled.
     */
    class WebSockets : public opendeck::websockets::Endpoint
    {
        public:
        WebSockets()           = default;
        ~WebSockets() override = default;

        /**
         * @brief Treats disabled WebSockets as initialized.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            return true;
        }

        int accept_client([[maybe_unused]] int socket) override
        {
            return -ENOTSUP;
        }
    };
}    // namespace opendeck::bootloader::websockets
