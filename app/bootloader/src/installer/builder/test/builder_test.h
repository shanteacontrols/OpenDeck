/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/installer/hwa/test/hwa_test.h"
#include "bootloader/src/installer/instance/impl/installer.h"

namespace opendeck::installer
{
    /**
     * @brief Convenience builder that wires the test installer backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the test installer builder.
         */
        Builder()
            : _instance(_hwa)
        {}

        /**
         * @brief Returns the constructed installer instance.
         *
         * @return Test-backed installer instance.
         */
        Installer& instance()
        {
            return _instance;
        }

        HwaTest _hwa;

        private:
        Installer _instance;
    };
}    // namespace opendeck::installer
