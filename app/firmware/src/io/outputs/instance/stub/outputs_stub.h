/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/shared/deps.h"
#include "firmware/src/io/base.h"

namespace opendeck::firmware::io::outputs
{
    /**
     * @brief Stub output subsystem used when no output implementation is available.
     */
    class Outputs : public io::Base
    {
        public:
        /**
         * @brief Constructs the stub output subsystem.
         *
         * @param hwa Stub hardware backend.
         * @param database Database view used by the subsystem.
         */
        Outputs([[maybe_unused]] Hwa&      hwa,
                [[maybe_unused]] Database& database)
        {}

        /**
         * @brief Reports that the stub output subsystem cannot initialize.
         *
         * @return Always `false`.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Deinitializes the stub output subsystem.
         */
        void deinit() override
        {
        }
    };
}    // namespace opendeck::firmware::io::outputs
