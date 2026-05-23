/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"
#include "bootloader/src/protocols/webusb/shared/deps.h"
#include "common/src/dfu/dfu_stream/instance/impl/dfu_stream.h"

#include <span>

namespace opendeck::bootloader::protocols::webusb
{
    /**
     * @brief Bootloader WebUSB endpoint that feeds incoming DFU bytes into the direct-update writer.
     */
    class WebUsbHw : public Hwa
    {
        public:
        /**
         * @brief Constructs WebUSB around a direct-update writer instance.
         *
         * @param direct_update_writer Direct-update writer that receives incoming DFU bytes.
         */
        explicit WebUsbHw(bootloader::dfu::direct_update_writer::DirectUpdateWriter& direct_update_writer);

        /**
         * @brief Initializes bootloader WebUSB.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Deinitializes bootloader WebUSB.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool deinit() override;

        /**
         * @brief Sends one device-side status line to the host.
         *
         * @param message Null-terminated status string to send.
         */
        void status(std::string_view message) override;

        /**
         * @brief Processes one received WebUSB DFU chunk.
         *
         * @param data Raw DFU stream bytes.
         */
        void feed(std::span<const uint8_t> data);

        private:
        opendeck::common::dfu::dfu_stream::DfuStream _dfu_stream;
    };
}    // namespace opendeck::bootloader::protocols::webusb
