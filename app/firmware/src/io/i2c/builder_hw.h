/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "i2c.h"
#include "hwa_hw.h"
#include "peripherals/builder.h"

namespace opendeck::io::i2c
{
    /**
     * @brief Convenience builder that wires the hardware I2C subsystem and peripherals.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the I2C builder and peripheral bundle.
         *
         * @param database Database administrator used by registered peripherals.
         */
        explicit Builder(database::Admin& database)
            : _peripherals(_hwa, database)
        {}

        /**
         * @brief Returns the constructed I2C subsystem instance.
         *
         * @return Hardware-backed I2C subsystem instance.
         */
        I2c& instance()
        {
            return _instance;
        }

        private:
        HwaHw              _hwa;
        BuilderPeripherals _peripherals;
        I2c                _instance;
    };
}    // namespace opendeck::io::i2c
