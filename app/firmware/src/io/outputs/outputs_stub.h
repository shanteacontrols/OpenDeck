/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/deps.h"
#include "firmware/src/io/base.h"

namespace opendeck::io::outputs
{
    /**
     * @brief Stub OUTPUT subsystem used when no OUTPUT implementation is available.
     */
    class Outputs : public io::Base
    {
        public:
        /**
         * @brief Constructs the stub OUTPUT subsystem.
         *
         * @param hwa Stub hardware backend.
         * @param database Database view used by the subsystem.
         */
        Outputs([[maybe_unused]] Hwa&      hwa,
                [[maybe_unused]] Database& database)
        {}

        /**
         * @brief Reports that the stub OUTPUT subsystem cannot initialize.
         *
         * @return Always `false`.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Deinitializes the stub OUTPUT subsystem.
         */
        void deinit() override
        {
        }
    };
}    // namespace opendeck::io::outputs
