/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "i2c_stub.h"
#include "hwa_stub.h"

namespace io::i2c
{
    /**
     * @brief Stub builder that exposes the I2C subsystem with a no-op backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the I2C stub builder.
         *
         * @param database Unused database handle kept for builder-interface consistency.
         */
        explicit Builder([[maybe_unused]] database::Admin& database)
        {}

        /**
         * @brief Returns the constructed I2C subsystem instance.
         *
         * @return Stub-backed I2C subsystem instance.
         */
        I2c& instance()
        {
            return _instance;
        }

        private:
        HwaStub _hwa;
        I2c     _instance;
    };
}    // namespace io::i2c
