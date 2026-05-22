/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/direct_update_writer/instance/impl/direct_update_writer.h"
#include "bootloader/src/webusb/hwa/hw/webusb_hw.h"
#include "bootloader/src/webusb/instance/impl/webusb.h"

namespace opendeck::webusb
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
        explicit Builder(direct_update_writer::DirectUpdateWriter& direct_update_writer)
            : _hwa(direct_update_writer)
            , _instance(_hwa)
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
}    // namespace opendeck::webusb
