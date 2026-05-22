/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/websockets/hwa/hw/hwa_hw.h"

namespace opendeck::protocol::websockets
{
    /**
     * @brief Zephyr-backed WebSockets platform hooks.
     */
    class HwaHw : public opendeck::websockets::HwaHw
    {
        public:
        HwaHw();
    };
}    // namespace opendeck::protocol::websockets
