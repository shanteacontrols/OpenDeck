/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/analog/instance/impl/analog.h"
#include "firmware/src/io/analog/filter/test/filter_test.h"
#include "firmware/src/io/analog/hwa/test/hwa_test.h"
#include "firmware/src/io/analog/instance/impl/frame_store.h"
#include "firmware/src/database/builder/test/builder_test.h"

namespace opendeck::firmware::io::analog
{
    /**
     * @brief Test builder that wires the analog subsystem to test backends.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the analog test builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _filter, _frame_store, _database)
        {}

        /**
         * @brief Returns the constructed analog subsystem instance.
         *
         * @return Test-backed analog subsystem instance.
         */
        Analog& instance()
        {
            return _instance;
        }

        Database   _database;
        HwaTest    _hwa;
        FilterTest _filter;
        FrameStore _frame_store;
        Analog     _instance;
    };
}    // namespace opendeck::firmware::io::analog
