/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/system/shared/deps.h"

namespace opendeck::bootloader::system
{
    /**
     * @brief Test system hardware hooks.
     */
    class HwaTest : public Hwa
    {
        public:
        bool consume_staged_update() override
        {
            return _consume_staged_update;
        }

        void reboot_application() override
        {
            _reboot_application_called = true;
        }

        void init_indicators() override
        {
            _init_indicators_called = true;
        }

        bool init_webusb() override
        {
            return _init_webusb_result;
        }

        bool init_mdns() override
        {
            return _init_mdns_result;
        }

        bool _consume_staged_update     = false;
        bool _reboot_application_called = false;
        bool _init_indicators_called    = false;
        bool _init_webusb_result        = true;
        bool _init_mdns_result          = true;
    };
}    // namespace opendeck::bootloader::system
