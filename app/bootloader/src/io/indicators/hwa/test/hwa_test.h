/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/io/indicators/instance/impl/deps.h"

namespace opendeck::bootloader::io::indicators
{
    /**
     * @brief Test indicator hardware hooks.
     */
    class HwaTest : public Hwa
    {
        public:
        bool init() override
        {
            _init_called = true;
            return _init_result;
        }

        void on() override
        {
            _on_called = true;
        }

        void off() override
        {
            _off_called = true;
        }

        bool _init_result = true;
        bool _init_called = false;
        bool _on_called   = false;
        bool _off_called  = false;
    };
}    // namespace opendeck::bootloader::io::indicators
