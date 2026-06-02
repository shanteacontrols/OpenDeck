/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/protocols/websockets/handler/handler.h"
#include "common/src/protocols/websockets/firmware_upload/firmware_upload.h"
#include "common/src/protocols/websockets/shared/firmware_upload.h"
#include "firmware/src/dfu/staged_update_writer/builder/builder.h"

namespace opendeck::firmware::protocol::websockets::firmware_upload
{
    /**
     * @brief Handles staged firmware upload WebSockets frames.
     */
    class FirmwareUploadHandler : public opendeck::common::protocols::websockets::Handler
    {
        public:
        /**
         * @brief Constructs the firmware-upload handler around staged-update storage.
         */
        FirmwareUploadHandler();

        /**
         * @brief Handles one firmware-upload frame.
         *
         * @param data       Frame payload bytes.
         * @param session_id Active WebSockets session id.
         *
         * @return ACK frame when the firmware-upload frame was handled, otherwise `std::nullopt`.
         */
        std::optional<std::span<const uint8_t>> handle_frame(std::span<const uint8_t> data, uint32_t session_id) override;

        private:
        firmware::dfu::staged_update_writer::Builder               _firmware_builder;
        opendeck::common::protocols::websockets::FirmwareUpload    _firmware_upload;
        opendeck::common::protocols::websockets::FirmwareUploadAck _response = {};
    };
}    // namespace opendeck::firmware::protocol::websockets::firmware_upload
