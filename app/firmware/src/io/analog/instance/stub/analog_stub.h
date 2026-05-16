/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/analog/shared/deps.h"
#include "firmware/src/io/base.h"

namespace opendeck::io::analog
{
    /**
     * @brief Stub analog subsystem used when no analog hardware is present.
     */
    class Analog : public io::Base
    {
        public:
        /**
         * @brief Constructs the stub analog subsystem.
         *
         * @param hwa Stub hardware backend.
         * @param filter Stub filter backend.
         * @param database Database view used by the subsystem.
         */
        Analog(Hwa&      hwa,
               Filter&   filter,
               Database& database)
        {}

        /**
         * @brief Reports that the stub analog subsystem cannot initialize.
         *
         * @return Always `false`.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Deinitializes the stub analog subsystem.
         */
        void deinit() override
        {
        }
    };
}    // namespace opendeck::io::analog
