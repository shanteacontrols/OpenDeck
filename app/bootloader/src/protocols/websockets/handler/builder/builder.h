/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"
#include "bootloader/src/protocols/websockets/handler/firmware_upload/firmware_upload_handler.h"

namespace opendeck::bootloader::protocols::websockets::handler
{
    /**
     * @brief Builder that instantiates bootloader WebSockets handlers.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs WebSockets handlers around the direct-update writer.
         *
         * @param direct_update_writer Writer that installs validated firmware payloads.
         */
        explicit Builder(bootloader::dfu::direct_update_writer::DirectUpdateWriter& direct_update_writer)
            : _firmware_upload(direct_update_writer)
        {}

        private:
        firmware_upload::FirmwareUploadHandler _firmware_upload;
    };
}    // namespace opendeck::bootloader::protocols::websockets::handler
