/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "io/base.h"

namespace opendeck::io::switches
{
    /**
     * @brief Stub switch subsystem used when no switch implementation is available.
     */
    class Switches : public io::Base
    {
        public:
        /**
         * @brief Constructs the stub switch subsystem.
         *
         * @param hwa Stub hardware backend.
         * @param filter Stub filter backend.
         * @param database Database view used by the subsystem.
         */
        Switches(Hwa&      hwa,
                 Filter&   filter,
                 Database& database)
        {}

        /**
         * @brief Reports that the stub switch subsystem cannot initialize.
         *
         * @return Always `false`.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Deinitializes the stub switch subsystem.
         */
        void deinit() override
        {
        }

        /**
         * @brief Resets no state because the stub subsystem keeps none.
         *
         * @param index Switch index to reset.
         */
        void reset(size_t index)
        {
        }
    };
}    // namespace opendeck::io::switches
