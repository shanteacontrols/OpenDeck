/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"
#include "bootloader/src/protocols/webusb/instance/impl/deps.h"
#include "bootloader/src/threads.h"
#include "common/src/dfu/dfu_stream_parser/instance/impl/dfu_stream_parser.h"

#include <span>

namespace opendeck::bootloader::protocols::webusb
{
    struct DfuRxChunk;
    struct WebUsbHwAccess;

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

        private:
        friend struct WebUsbHwAccess;

        opendeck::common::dfu::dfu_stream_parser::DfuStreamParser _dfu_stream;
        bootloader::threads::WebUsbRxThread                       _rx_thread;

        /**
         * @brief Queues one received WebUSB DFU buffer for processing.
         *
         * @param chunk Raw DFU stream buffer and owning USB class data.
         *
         * @return `true` when the buffer was queued, otherwise `false`.
         */
        bool feed(DfuRxChunk chunk);

        /**
         * @brief Processes queued WebUSB RX chunks outside the USB request callback.
         */
        void process_rx();

        /**
         * @brief Stops the WebUSB RX worker.
         */
        void stop_rx();
    };
}    // namespace opendeck::bootloader::protocols::webusb
