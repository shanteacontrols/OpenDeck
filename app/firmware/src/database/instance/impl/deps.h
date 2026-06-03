/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/lessdb/lessdb.h"

namespace opendeck::firmware::database
{
    /**
     * @brief Hardware abstraction interface for database storage access.
     */
    class Hwa : public zlibs::utils::lessdb::Hwa
    {
        public:
        /**
         * @brief Returns whether a valid factory snapshot is provisioned.
         *
         * @return `true` when the factory snapshot can be restored.
         */
        virtual bool has_factory_snapshot() = 0;

        /**
         * @brief Stores the current initialized database contents as a factory snapshot.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool store_factory_snapshot() = 0;

        /**
         * @brief Restores the active database contents from the factory snapshot.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool restore_factory_snapshot() = 0;
    };
}    // namespace opendeck::firmware::database
