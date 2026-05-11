/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "webconfig_stub.h"

namespace opendeck::protocol::webconfig
{
    /**
     * @brief Stub builder that wires WebSocket configuration to a no-op backend.
     */
    class Builder
    {
        public:
        Builder() = default;

        /**
         * @brief Returns the disabled WebConfig protocol instance.
         *
         * @return Stub WebConfig protocol instance.
         */
        WebConfig& instance()
        {
            return _instance;
        }

        private:
        WebConfig _instance;
    };
}    // namespace opendeck::protocol::webconfig
