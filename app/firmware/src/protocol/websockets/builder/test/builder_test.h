/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/websockets/hwa/test/hwa_test.h"
#include "firmware/src/protocol/websockets/handler/builder/builder.h"
#include "firmware/src/protocol/websockets/instance/impl/websockets.h"

namespace opendeck::protocol::websockets
{
    /**
     * @brief Test builder that wires WebSockets to test backends.
     */
    class Builder
    {
        public:
        Builder()
            : _instance(_hwa)
        {}

        /**
         * @brief Returns the test WebSockets protocol instance.
         *
         * @return Test-backed WebSockets protocol instance.
         */
        WebSockets& instance()
        {
            return _instance;
        }

        /**
         * @brief Returns the test WebSockets backend.
         *
         * @return Test WebSockets backend.
         */
        HwaTest& hwa()
        {
            return _hwa;
        }

        private:
        handler::Builder _handlers;
        HwaTest          _hwa;
        WebSockets       _instance;
    };
}    // namespace opendeck::protocol::websockets
