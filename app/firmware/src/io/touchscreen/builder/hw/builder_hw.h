/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/touchscreen/touchscreen.h"
#include "firmware/src/io/touchscreen/hwa/hw/hwa_hw.h"
#include "firmware/src/io/touchscreen/models/builder.h"
#include "firmware/src/database/builder.h"

namespace opendeck::io::touchscreen
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
}    // namespace opendeck::io::touchscreen
