/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/upload/instance/impl/upload.h"
#include "common/src/protocols/websockets/handler/handler.h"
#include "firmware/src/dfu/staged_update_writer/instance/impl/staged_update_writer.h"

namespace opendeck::firmware::protocol::websockets::firmware_upload
{
    /**
     * @brief Handles firmware-side WebSockets DFU into staged-update storage.
     *
     * The running firmware cannot install the new image directly, so this handler stores the uploaded
     * `dfu.bin` into staged DFU storage and requests a reboot into the bootloader once the upload finishes.
     */
    class FirmwareUploadHandler : public opendeck::common::protocols::websockets::Handler
    {
        public:
        /**
         * @brief Constructs the firmware-upload handler around staged-update storage.
         *
         * @param staged_update_writer Writer that stores validated firmware payloads in staged DFU storage.
         */
        explicit FirmwareUploadHandler(firmware::dfu::staged_update_writer::StagedUpdateWriter& staged_update_writer);

        /**
         * @brief Handles one firmware-upload frame.
         *
         * On upload start this handler closes the active WebSockets SysEx configuration session so the
         * firmware-upload flow owns the transport until completion or abort.
         *
         * @param data       Frame payload bytes.
         * @param session_id Active WebSockets session id.
         *
         * @return Serialized DFU ACK when the firmware-upload frame was handled, otherwise `std::nullopt`.
         */
        std::optional<Response> handle_frame(std::span<const uint8_t> data, uint32_t session_id) override;

        private:
        opendeck::common::dfu::upload::Upload _firmware_upload;
    };
}    // namespace opendeck::firmware::protocol::websockets::firmware_upload
