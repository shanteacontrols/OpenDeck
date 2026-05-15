/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/encoders/encoders.h"
#include "firmware/src/io/digital/encoders/filter_hw.h"
#include "firmware/src/io/digital/encoders/hwa_hw.h"
#include "firmware/src/io/digital/switches/deps.h"
#include "firmware/src/database/builder.h"

namespace opendeck::io::encoders
{
    /**
     * @brief Convenience builder that wires the hardware encoder subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the encoder builder around shared database and switch HWA instances.
         *
         * @param database Database administrator used for configuration access.
         * @param switches_hwa Switch HWA used to derive encoder pair state.
         */
        Builder(database::Admin&   database,
                io::switches::Hwa& switches_hwa)
            : _database(database)
            , _hwa(switches_hwa)
            , _mapper(_database)
            , _instance(_hwa, _filter, _database, _mapper)
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
        Mapper   _mapper;
        Encoders _instance;
    };
}    // namespace opendeck::io::encoders
