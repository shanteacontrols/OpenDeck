/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/staged_update_reader/hwa/hw/hwa_hw.h"
#include "bootloader/src/dfu/staged_update_reader/instance/impl/staged_update_reader.h"

namespace opendeck::bootloader::dfu::staged_update_reader
{
    /**
     * @brief Builder that wires staged-update reader storage to real flash.
     */
    class Builder
    {
        public:
        Builder()
            : _instance(_hwa)
        {}

        StagedUpdateReader& instance()
        {
            return _instance;
        }

        private:
        HwaHw              _hwa;
        StagedUpdateReader _instance;
    };
}    // namespace opendeck::bootloader::dfu::staged_update_reader
