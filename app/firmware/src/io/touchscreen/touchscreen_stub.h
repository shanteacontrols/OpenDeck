/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace io::touchscreen
{
    /**
     * @brief Stub touchscreen subsystem used when no touchscreen implementation is available.
     */
    class Touchscreen : public io::Base
    {
        public:
        /**
         * @brief Constructs the stub touchscreen subsystem.
         *
         * @param hwa Stub hardware backend.
         * @param database Database view used by the subsystem.
         */
        Touchscreen([[maybe_unused]] Hwa&      hwa,
                    [[maybe_unused]] Database& database)
        {}

        /**
         * @brief Reports that the stub touchscreen subsystem cannot initialize.
         *
         * @return Always `false`.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Deinitializes the stub touchscreen subsystem.
         */
        void deinit() override
        {
        }
    };
}    // namespace io::touchscreen
