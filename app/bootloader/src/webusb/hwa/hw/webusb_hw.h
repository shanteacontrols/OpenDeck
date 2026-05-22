/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/installer/instance/impl/installer.h"
#include "bootloader/src/webusb/shared/deps.h"

namespace opendeck::webusb
{
    /**
     * @brief Bootloader WebUSB endpoint that feeds incoming DFU bytes into the installer.
     */
    class WebUsbHw : public Hwa
    {
        public:
        /**
         * @brief Constructs WebUSB around an installer instance.
         *
         * @param installer Installer that receives incoming DFU bytes.
         */
        explicit WebUsbHw(installer::Installer& installer);

        /**
         * @brief Initializes bootloader WebUSB.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool init();

        /**
         * @brief Deinitializes bootloader WebUSB.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool deinit();

        /**
         * @brief Sends one device-side status line to the host.
         *
         * @param message Null-terminated status string to send.
         */
        void status(std::string_view message) override;

        private:
        installer::Installer& _installer;
    };
}    // namespace opendeck::webusb
