/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "io/base.h"

namespace io::buttons
{
    /**
     * @brief Stub button subsystem used when no button implementation is available.
     */
    class Buttons : public io::Base
    {
        public:
        /**
         * @brief Constructs the stub button subsystem.
         *
         * @param hwa Stub hardware backend.
         * @param filter Stub filter backend.
         * @param database Database view used by the subsystem.
         */
        Buttons(Hwa&      hwa,
                Filter&   filter,
                Database& database)
        {}

        /**
         * @brief Reports that the stub button subsystem cannot initialize.
         *
         * @return Always `false`.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Deinitializes the stub button subsystem.
         */
        void deinit() override
        {
        }

        /**
         * @brief Resets no state because the stub subsystem keeps none.
         *
         * @param index Button index to reset.
         */
        void reset(size_t index)
        {
        }
    };
}    // namespace io::buttons
