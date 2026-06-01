/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"
#include "bootloader/src/protocols/webusb/instance/stub/webusb_stub.h"

namespace opendeck::bootloader::protocols::webusb
{
    /**
     * @brief Convenience builder that wires the stub WebUSB endpoint.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the stub WebUSB builder.
         *
         * @param direct_update_writer Direct-update writer dependency kept for the shared builder shape.
         */
        explicit Builder([[maybe_unused]] bootloader::dfu::direct_update_writer::DirectUpdateWriter& direct_update_writer)
        {}

        /**
         * @brief Returns the stub WebUSB endpoint.
         *
         * @return Stub WebUSB endpoint.
         */
        WebUsb& instance()
        {
            return _instance;
        }

        private:
        WebUsb _instance;
    };
}    // namespace opendeck::bootloader::protocols::webusb
