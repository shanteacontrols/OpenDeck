/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/webconfig/instance/stub/webconfig_stub.h"

namespace opendeck::protocol::webconfig
{
    /**
     * @brief Test builder that keeps WebConfig out of host-test network stacks.
     */
    class Builder
    {
        public:
        Builder() = default;

        /**
         * @brief Returns the test WebConfig protocol instance.
         *
         * @return Stub WebConfig protocol instance.
         */
        WebConfig& instance()
        {
            return _instance;
        }

        WebConfig _instance;
    };
}    // namespace opendeck::protocol::webconfig
