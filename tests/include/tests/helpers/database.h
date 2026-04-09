/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "database/deps.h"

namespace tests
{
    /**
     * @brief No-op database lifecycle handler used by tests that only need a valid observer.
     */
    class NoOpDatabaseHandlers : public database::Handlers
    {
        public:
        void preset_change(uint8_t) override
        {
        }

        void factory_reset_start() override
        {
        }

        void factory_reset_done() override
        {
        }

        void initialized() override
        {
        }
    };
}    // namespace tests
