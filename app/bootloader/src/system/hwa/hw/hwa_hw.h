/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/indicators/builder/builder.h"
#include "bootloader/src/installer/builder/builder.h"
#include "bootloader/src/mdns/builder/builder.h"
#include "bootloader/src/staged_update_reader/builder/builder.h"
#include "bootloader/src/system/shared/deps.h"
#include "bootloader/src/webusb/builder/builder.h"
#include "common/src/mcu/hwa/hw/hwa_hw.h"

namespace opendeck::bootloader::system
{
    /**
     * @brief Hardware-backed bootloader system services.
     */
    class HwaHw : public Hwa
    {
        public:
        HwaHw() = default;

        /**
         * @brief Installs a staged update when one is present.
         *
         * @return `true` if a staged update was consumed, otherwise `false`.
         */
        bool consume_staged_update() override
        {
            return _staged_update_reader.instance().consume(_staged_update_installer.instance());
        }

        /**
         * @brief Reboots into the main application image.
         */
        void reboot_application() override
        {
            _mcu.reboot(mcu::BootTarget::Application);
        }

        /**
         * @brief Initializes the bootloader indicators.
         */
        void init_indicators() override
        {
            _indicators.instance().init();
        }

        /**
         * @brief Initializes the WebUSB recovery transport.
         *
         * @return `true` if WebUSB is disabled or initialized successfully, otherwise `false`.
         */
        bool init_webusb() override
        {
            return _webusb.instance().init();
        }

        /**
         * @brief Initializes network discovery for recovery transports.
         *
         * @return `true` if mDNS is disabled or initialized successfully, otherwise `false`.
         */
        bool init_mdns() override
        {
            return _mdns.instance().init();
        }

        private:
        mcu::HwaHw                    _mcu;
        indicators::Builder           _indicators;
        staged_update_reader::Builder _staged_update_reader;
        installer::Builder            _staged_update_installer = installer::Builder(_mcu);
        installer::Builder            _webusb_installer        = installer::Builder(_mcu);
        webusb::Builder               _webusb                  = webusb::Builder(_webusb_installer.instance());
        bootloader::mdns::Builder     _mdns                    = bootloader::mdns::Builder(_mcu);
    };
}    // namespace opendeck::bootloader::system
