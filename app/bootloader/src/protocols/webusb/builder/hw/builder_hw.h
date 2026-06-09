/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"
#include "bootloader/src/protocols/webusb/hwa/hw/webusb_hw.h"
#include "bootloader/src/protocols/webusb/instance/impl/webusb.h"

namespace opendeck::bootloader::protocols::webusb
{
    /**
     * @brief Convenience builder that wires the hardware WebUSB endpoint.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the hardware WebUSB builder.
         *
         * @param direct_update_writer Direct-update writer that receives incoming DFU bytes.
         */
        explicit Builder(bootloader::dfu::direct_update_writer::DirectUpdateWriter& direct_update_writer)
            : _instance(_hwa, direct_update_writer)
        {}

        /**
         * @brief Returns the hardware WebUSB endpoint.
         *
         * @return Hardware WebUSB endpoint.
         */
        WebUsb& instance()
        {
            return _instance;
        }

        private:
        WebUsbHw _hwa;
        WebUsb   _instance;
    };
}    // namespace opendeck::bootloader::protocols::webusb
