/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"
#include "bootloader/src/protocols/websockets/handler/builder/builder.h"
#include "bootloader/src/protocols/websockets/hwa/hw/hwa_hw.h"
#include "bootloader/src/protocols/websockets/instance/impl/websockets.h"

namespace opendeck::bootloader::protocols::websockets
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
        explicit Builder(bootloader::dfu::direct_update_writer::DirectUpdateWriter& direct_update_writer)
            : _handlers(direct_update_writer)
            , _instance(_hwa)
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
        handler::Builder _handlers;
        HwaHw            _hwa;
        WebSockets       _instance;
    };
}    // namespace opendeck::bootloader::protocols::websockets
