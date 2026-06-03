/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/hwa/hw/hwa_hw.h"
#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"
#include "common/src/mcu/shared/deps.h"

namespace opendeck::bootloader::dfu::direct_update_writer
{
    /**
     * @brief Convenience builder that wires the hardware direct-update writer backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the direct-update writer builder.
         *
         * @param mcu Shared MCU services.
         */
        explicit Builder(opendeck::common::mcu::Hwa& mcu)
            : _hwa(mcu)
            , _instance(_hwa, _hwa)
        {}

        /**
         * @brief Returns the constructed direct-update writer instance.
         *
         * @return Hardware-backed direct-update writer instance.
         */
        DirectUpdateWriter& instance()
        {
            return _instance;
        }

        private:
        HwaHw              _hwa;
        DirectUpdateWriter _instance;
    };
}    // namespace opendeck::bootloader::dfu::direct_update_writer
