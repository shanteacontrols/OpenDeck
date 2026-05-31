/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/base.h"

namespace opendeck::io::indicators
{
    /**
     * @brief Stub indicator subsystem used when traffic indicators are not enabled.
     */
    class Indicators : public io::Base
    {
        public:
        bool init() override
        {
            return false;
        }

        void deinit() override
        {}
    };
}    // namespace opendeck::io::indicators
