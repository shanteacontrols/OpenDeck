/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/base.h"

namespace opendeck::protocol::mdns
{
    /**
     * @brief Stub mDNS backend used when discovery support is disabled.
     */
    class Mdns : public protocol::Base
    {
        public:
        Mdns()           = default;
        ~Mdns() override = default;

        bool init() override
        {
            return true;
        }

        bool deinit() override
        {
            return true;
        }
    };
}    // namespace opendeck::protocol::mdns
