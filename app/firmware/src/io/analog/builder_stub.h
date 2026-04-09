/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "analog_stub.h"
#include "filter_stub.h"
#include "hwa_stub.h"
#include "database/builder.h"

namespace io::analog
{
    /**
     * @brief Stub builder that wires the analog subsystem to no-op backends.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the analog stub builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _filter, _database)
        {}

        /**
         * @brief Returns the constructed analog subsystem instance.
         *
         * @return Stub-backed analog subsystem instance.
         */
        Analog& instance()
        {
            return _instance;
        }

        private:
        HwaStub    _hwa;
        FilterStub _filter;
        Database   _database;
        Analog     _instance;
    };
}    // namespace io::analog
