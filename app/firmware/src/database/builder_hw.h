/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "database.h"
#include "hwa_hw.h"

namespace opendeck::database
{
    /**
     * @brief Convenience builder that exposes the shared hardware database instance.
     */
    class Builder
    {
        public:
        Builder() = default;

        /**
         * @brief Returns the singleton hardware database administrator.
         *
         * @return Shared hardware database administrator.
         */
        static Admin& instance()
        {
            static HwaHw hwa;
            static Admin admin = Admin(hwa);

            return admin;
        }
    };
}    // namespace opendeck::database
