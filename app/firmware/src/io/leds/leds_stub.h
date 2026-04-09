/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "io/base.h"

namespace io::leds
{
    /**
     * @brief Stub LED subsystem used when no LED implementation is available.
     */
    class Leds : public io::Base
    {
        public:
        /**
         * @brief Constructs the stub LED subsystem.
         *
         * @param hwa Stub hardware backend.
         * @param database Database view used by the subsystem.
         */
        Leds([[maybe_unused]] Hwa&      hwa,
             [[maybe_unused]] Database& database)
        {}

        /**
         * @brief Reports that the stub LED subsystem cannot initialize.
         *
         * @return Always `false`.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Deinitializes the stub LED subsystem.
         */
        void deinit() override
        {
        }
    };
}    // namespace io::leds
