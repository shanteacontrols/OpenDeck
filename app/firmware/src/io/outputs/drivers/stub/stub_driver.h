/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/shared/deps.h"

namespace opendeck::io::outputs
{
    /**
     * @brief No-op output driver used for touchscreen-only targets.
     */
    class Driver : public Hwa
    {
        public:
        void set_level(size_t, uint8_t) override
        {
        }
    };
}    // namespace opendeck::io::outputs
