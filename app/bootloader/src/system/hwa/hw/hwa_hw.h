/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/io/indicators/builder/builder.h"
#include "bootloader/src/dfu/direct_update_writer/builder/builder.h"
#include "bootloader/src/protocols/mdns/builder/builder.h"
#include "bootloader/src/dfu/staged_update_reader/builder/builder.h"
#include "bootloader/src/system/shared/deps.h"
#include "bootloader/src/protocols/webusb/builder/builder.h"
#include "bootloader/src/protocols/websockets/builder/builder.h"
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
            return _staged_update_reader.instance().consume(_staged_update_direct_update_writer.instance());
        }

        /**
         * @brief Reboots into the main application image.
         */
        void reboot_application() override
        {
            _mcu.reboot(opendeck::common::mcu::BootTarget::Application);
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
         * @brief Initializes the WebSocket network recovery transport.
         *
         * @return `true` if WebSockets are disabled or initialized successfully, otherwise `false`.
         */
        bool init_websockets() override
        {
            return _websockets.instance().init();
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
        opendeck::common::mcu::HwaHw                   _mcu;
        bootloader::io::indicators::Builder            _indicators;
        bootloader::dfu::staged_update_reader::Builder _staged_update_reader;
        bootloader::dfu::direct_update_writer::Builder _staged_update_direct_update_writer = bootloader::dfu::direct_update_writer::Builder(_mcu);
        bootloader::dfu::direct_update_writer::Builder _webusb_direct_update_writer        = bootloader::dfu::direct_update_writer::Builder(_mcu);
        bootloader::dfu::direct_update_writer::Builder _websockets_direct_update_writer    = bootloader::dfu::direct_update_writer::Builder(_mcu);
        bootloader::protocols::webusb::Builder         _webusb                             = bootloader::protocols::webusb::Builder(_webusb_direct_update_writer.instance());
        bootloader::protocols::websockets::Builder     _websockets                         = bootloader::protocols::websockets::Builder(_websockets_direct_update_writer.instance());
        bootloader::protocols::mdns::Builder           _mdns                               = bootloader::protocols::mdns::Builder(_mcu);
    };
}    // namespace opendeck::bootloader::system
