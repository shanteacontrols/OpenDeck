/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/protocols/websockets/handler/handler.h"
#include "firmware/src/dfu/staged_update_writer/builder/builder.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_CONFIG_INTERFACE_WEBSOCKETS
#include "firmware/src/protocol/websockets/handler/sysex_config/sysex_config_handler.h"
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_STAGED_UPDATE
#include "firmware/src/protocol/websockets/handler/firmware_upload/firmware_upload_handler.h"
#endif

namespace opendeck::firmware::protocol::websockets::handler
{
    /**
     * @brief Builder that instantiates WebSockets handlers.
     */
    class Builder
    {
        public:
        Builder() = default;

        private:
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_CONFIG_INTERFACE_WEBSOCKETS
        websockets::sysex_config::SysexConfigHandler _sysex_config;
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_STAGED_UPDATE
        firmware::dfu::staged_update_writer::Builder       _staged_update_writer;
        websockets::firmware_upload::FirmwareUploadHandler _firmware_upload{ _staged_update_writer.instance() };
#endif
    };
}    // namespace opendeck::firmware::protocol::websockets::handler
