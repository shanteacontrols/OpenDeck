/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/direct_update_writer/instance/impl/direct_update_writer.h"
#include "bootloader/src/websockets/hwa/hw/hwa_hw.h"
#include "bootloader/src/websockets/instance/impl/websockets.h"

namespace opendeck::bootloader::websockets
{
    /**
     * @brief Convenience builder that wires the hardware bootloader WebSockets endpoint.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the bootloader WebSockets builder.
         *
         * @param direct_update_writer Writer that installs validated firmware payloads.
         */
        explicit Builder(direct_update_writer::DirectUpdateWriter& direct_update_writer)
            : _instance(_hwa, direct_update_writer)
        {}

        /**
         * @brief Returns the bootloader WebSockets endpoint.
         *
         * @return Bootloader WebSockets endpoint.
         */
        WebSockets& instance()
        {
            return _instance;
        }

        private:
        HwaHw      _hwa;
        WebSockets _instance;
    };
}    // namespace opendeck::bootloader::websockets
