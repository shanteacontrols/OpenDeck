/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/analog/analog_stub.h"
#include "firmware/src/io/analog/filter_stub.h"
#include "firmware/src/io/analog/hwa_stub.h"
#include "firmware/src/io/analog/mapper.h"
#include "firmware/src/database/builder.h"

namespace opendeck::io::analog
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
            , _instance(_hwa, _filter, _mapper, _database)
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
        Mapper     _mapper;
        Database   _database;
        Analog     _instance;
    };
}    // namespace opendeck::io::analog
