/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "io/base.h"

namespace opendeck::io::i2c
{
    /**
     * @brief Stub I2C subsystem used when no I2C implementation is available.
     */
    class I2c : public io::Base
    {
        public:
        I2c() = default;

        /**
         * @brief Reports that the stub I2C subsystem cannot initialize.
         *
         * @return Always `false`.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Deinitializes the stub I2C subsystem.
         */
        void deinit() override
        {
        }
    };
}    // namespace opendeck::io::i2c
