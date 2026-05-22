/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace opendeck::bootloader::mdns
{
    /**
     * @brief Stub bootloader mDNS discovery used when network discovery is disabled.
     */
    class MdnsStub
    {
        public:
        /**
         * @brief Treats disabled mDNS as initialized.
         *
         * @return Always `true`.
         */
        bool init()
        {
            return true;
        }
    };
}    // namespace opendeck::bootloader::mdns
