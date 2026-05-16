/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/webconfig/hwa/hw/hwa_hw.h"
#include "firmware/src/mcu/builder/builder.h"
#include "firmware/src/staged_update/builder/builder.h"
#include "firmware/src/protocol/webconfig/instance/impl/webconfig.h"

namespace opendeck::protocol::webconfig
{
    /**
     * @brief Convenience builder that wires the WebSocket configuration endpoint.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs WebConfig with a private MCU services backend.
         */
        Builder()
            : _hwa(_default_mcu_builder.instance())
            , _instance(_hwa, _firmware_builder.instance())
        {}

        /**
         * @brief Constructs WebConfig with shared MCU services.
         *
         * @param mcu MCU services shared by firmware subsystems.
         */
        explicit Builder(mcu::Hwa& mcu)
            : _hwa(mcu)
            , _instance(_hwa, _firmware_builder.instance())
        {}

        /**
         * @brief Returns the configured WebConfig protocol instance.
         *
         * @return WebConfig protocol instance.
         */
        WebConfig& instance()
        {
            return _instance;
        }

        private:
        mcu::Builder           _default_mcu_builder;
        staged_update::Builder _firmware_builder;
        HwaHw                  _hwa;
        WebConfig              _instance;
    };
}    // namespace opendeck::protocol::webconfig
