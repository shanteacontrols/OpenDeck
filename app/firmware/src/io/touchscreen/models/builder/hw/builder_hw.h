/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/touchscreen/shared/deps.h"
#include "firmware/src/io/touchscreen/instance/impl/models/nextion/nextion.h"

namespace opendeck::io::touchscreen
{
    /**
     * @brief Registers the touchscreen model implementations available on hardware builds.
     */
    class ModelsBuilder
    {
        public:
        /**
         * @brief Constructs and registers all supported touchscreen models.
         *
         * @param hwa Hardware backend shared by model instances.
         */
        explicit ModelsBuilder(Hwa& hwa)
            : _nextion(hwa)
        {}

        private:
        Nextion _nextion;
    };
}    // namespace opendeck::io::touchscreen
