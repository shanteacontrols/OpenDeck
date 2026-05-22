/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/direct_update_writer/instance/impl/direct_update_writer.h"
#include "bootloader/src/webusb/instance/impl/webusb.h"
#include "bootloader/src/webusb/instance/stub/webusb_stub.h"

namespace opendeck::webusb
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
        explicit Builder([[maybe_unused]] direct_update_writer::DirectUpdateWriter& direct_update_writer)
            : _instance(_hwa)
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
        WebUsbStub _hwa;
        WebUsb     _instance;
    };
}    // namespace opendeck::webusb
