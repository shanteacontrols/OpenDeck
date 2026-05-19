/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/mcu/shared/deps.h"
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
         * @brief Constructs the test WebConfig builder with shared MCU services.
         *
         * @param mcu Unused MCU services retained for builder compatibility.
         */
        explicit Builder([[maybe_unused]] mcu::Hwa& mcu)
        {}

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
