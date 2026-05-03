/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace opendeck::io::encoders
{
    /**
     * @brief Stub encoder subsystem used when no encoder implementation is available.
     */
    class Encoders : public io::Base
    {
        public:
        /**
         * @brief Constructs the stub encoder subsystem.
         *
         * @param hwa Stub hardware backend.
         * @param filter Stub filter backend.
         * @param database Database view used by the subsystem.
         */
        Encoders(Hwa&      hwa,
                 Filter&   filter,
                 Database& database)
        {}

        /**
         * @brief Reports that the stub encoder subsystem cannot initialize.
         *
         * @return Always `false`.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Deinitializes the stub encoder subsystem.
         */
        void deinit() override
        {
        }
    };
}    // namespace opendeck::io::encoders
