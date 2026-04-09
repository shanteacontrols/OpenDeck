/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "i2c.h"
#include "hwa_test.h"
#include "database/builder_test.h"

namespace io::i2c
{
    /**
     * @brief Test builder that exposes the I2C subsystem with a test backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the I2C test builder.
         *
         * @param database Unused database handle kept for builder-interface consistency.
         */
        Builder([[maybe_unused]] database::Admin& database)
        {}

        /**
         * @brief Returns the constructed I2C subsystem instance.
         *
         * @return Test-backed I2C subsystem instance.
         */
        I2c& instance()
        {
            return _instance;
        }

        private:
        HwaTest _hwa;
        I2c     _instance;
    };
}    // namespace io::i2c
