/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "touchscreen.h"
#include "hwa_hw.h"
#include "models/builder.h"
#include "database/builder.h"

namespace io::touchscreen
{
    /**
     * @brief Convenience builder that wires the hardware touchscreen subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the touchscreen builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _models(_hwa)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the constructed touchscreen subsystem instance.
         *
         * @return Hardware-backed touchscreen subsystem instance.
         */
        Touchscreen& instance()
        {
            return _instance;
        }

        private:
        Database      _database;
        HwaHw         _hwa;
        ModelsBuilder _models;
        Touchscreen   _instance;
    };
}    // namespace io::touchscreen
