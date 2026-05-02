/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "database.h"
#include "hwa_test.h"

namespace database
{
    /**
     * @brief Test-database builder that owns the test backend and layout.
     */
    class Builder
    {
        public:
        Builder() = default;

        /**
         * @brief Returns the constructed test database administrator.
         *
         * @return Test database administrator instance.
         */
        Admin& instance()
        {
            return _instance;
        }

        HwaTest _hwa;
        Admin   _instance = Admin(_hwa);
    };
}    // namespace database
