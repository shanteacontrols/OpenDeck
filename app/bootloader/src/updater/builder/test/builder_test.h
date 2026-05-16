/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/updater/hwa/test/hwa_test.h"
#include "bootloader/src/updater/instance/impl/updater.h"

namespace opendeck::updater
{
    /**
     * @brief Convenience builder that wires the test updater backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the test updater builder.
         */
        Builder()
            : _instance(_hwa)
        {}

        /**
         * @brief Returns the constructed updater instance.
         *
         * @return Test-backed updater instance.
         */
        Updater& instance()
        {
            return _instance;
        }

        HwaTest _hwa;

        private:
        Updater _instance;
    };
}    // namespace opendeck::updater
