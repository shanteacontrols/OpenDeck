/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/protocols/websockets/hwa/hw/hwa_hw.h"

namespace opendeck::bootloader::protocols::websockets
{
    /**
     * @brief Zephyr-backed bootloader WebSockets platform hooks.
     */
    class HwaHw : public opendeck::common::protocols::websockets::HwaHw
    {
        public:
        HwaHw();
    };
}    // namespace opendeck::bootloader::protocols::websockets
