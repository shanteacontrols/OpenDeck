/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "encoders.h"
#include "filter_hw.h"
#include "hwa_hw.h"
#include "io/digital/buttons/deps.h"
#include "database/builder.h"

namespace opendeck::io::encoders
{
    /**
     * @brief Convenience builder that wires the hardware encoder subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the encoder builder around shared database and button HWA instances.
         *
         * @param database Database administrator used for configuration access.
         * @param buttons_hwa Button HWA used to derive encoder pair state.
         */
        Builder(database::Admin&  database,
                io::buttons::Hwa& buttons_hwa)
            : _database(database)
            , _hwa(buttons_hwa)
            , _instance(_hwa, _filter, _database)
        {}

        /**
         * @brief Returns the constructed encoder subsystem instance.
         *
         * @return Hardware-backed encoder subsystem instance.
         */
        Encoders& instance()
        {
            return _instance;
        }

        private:
        Database _database;
        HwaHw    _hwa;
        FilterHw _filter;
        Encoders _instance;
    };
}    // namespace opendeck::io::encoders
