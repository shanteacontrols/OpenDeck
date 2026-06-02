/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/instance/impl/i2c.h"
#include "firmware/src/io/i2c/hwa/test/hwa_test.h"
#include "firmware/src/io/i2c/peripherals/builder/builder.h"
#include "firmware/src/database/builder/test/builder_test.h"

namespace opendeck::firmware::io::i2c
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
        explicit Builder(database::Admin& database)
            : _peripherals(_hwa, database)
            , _instance(_hwa)
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

        HwaTest _hwa;

        private:
        BuilderPeripherals _peripherals;
        I2c                _instance;
    };
}    // namespace opendeck::firmware::io::i2c
