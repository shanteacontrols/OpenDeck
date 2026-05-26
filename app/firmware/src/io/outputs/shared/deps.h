/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/shared/common.h"
#include "firmware/src/database/instance/impl/database.h"

namespace opendeck::io::outputs
{
    /**
     * @brief Database view used by the output subsystem.
     */
    using Database = database::User<database::Config::Section::Outputs,
                                    database::Config::Section::Global>;

    /**
     * @brief Hardware abstraction used by the output subsystem.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Sets the level of one output.
         *
         * @param index output index to update.
         * @param level Output level percentage in the range [0, 100].
         */
        virtual void set_level(size_t index, uint8_t level) = 0;
    };
}    // namespace opendeck::io::outputs
