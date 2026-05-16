/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/base.h"
#include "firmware/src/database/builder/builder.h"

namespace opendeck::io::digital
{
    /**
     * @brief Stub digital subsystem used when no digital-input backend is enabled.
     */
    class DigitalStub : public io::Base
    {
        public:
        /**
         * @brief Reports that the stub digital subsystem cannot initialize.
         *
         * @return Always `false`.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Deinitializes the stub digital subsystem.
         */
        void deinit() override
        {
        }
    };

    /**
     * @brief Stub builder that exposes a no-op digital subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the stub digital builder.
         *
         * @param database Unused database handle kept for builder-interface consistency.
         */
        explicit Builder([[maybe_unused]] database::Admin& database)
        {}

        /**
         * @brief Returns the constructed stub digital subsystem instance.
         *
         * @return Stub-backed digital subsystem instance.
         */
        DigitalStub& instance()
        {
            return _instance;
        }

        private:
        DigitalStub _instance;
    };
}    // namespace opendeck::io::digital
