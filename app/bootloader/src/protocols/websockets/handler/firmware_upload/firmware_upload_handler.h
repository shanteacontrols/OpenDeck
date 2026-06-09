/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"
#include "common/src/dfu/upload/instance/impl/upload.h"
#include "common/src/protocols/websockets/handler/handler.h"

namespace opendeck::bootloader::protocols::websockets::firmware_upload
{
    /**
     * @brief Handles bootloader-side WebSockets DFU directly into the application slot.
     *
     * Unlike the firmware-side handler, the bootloader is already running in the update environment, so it
     * installs the validated firmware image directly and does not need to request a reboot into another mode.
     */
    class FirmwareUploadHandler : public opendeck::common::protocols::websockets::Handler
    {
        public:
        /**
         * @brief Constructs the firmware-upload handler around direct update storage.
         *
         * @param direct_update_writer Writer that installs validated firmware payloads.
         */
        explicit FirmwareUploadHandler(bootloader::dfu::direct_update_writer::DirectUpdateWriter& direct_update_writer);

        /**
         * @brief Handles one firmware-upload frame.
         *
         * @param data       Frame payload bytes.
         * @param session_id Unused bootloader WebSockets session id.
         *
         * @return Serialized DFU ACK when the firmware-upload frame was handled, otherwise `std::nullopt`.
         */
        std::optional<Response> handle_frame(std::span<const uint8_t> data, uint32_t session_id) override;

        /**
         * @brief Aborts any active firmware upload.
         *
         * @param session_id Unused bootloader WebSockets session id.
         */
        void on_close_session(uint32_t session_id) override;

        private:
        opendeck::common::dfu::upload::Upload _firmware_upload;
    };
}    // namespace opendeck::bootloader::protocols::websockets::firmware_upload
