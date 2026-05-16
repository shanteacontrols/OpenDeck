/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/updater/hwa/hw/hwa_hw.h"
#include "bootloader/src/updater/updater.h"

namespace opendeck::updater
{
    /**
     * @brief Convenience builder that wires the hardware updater backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the updater builder.
         *
         * @param cleanup_callback Callback passed to the hardware backend.
         */
        explicit Builder(CleanupCallback cleanup_callback)
            : _hwa(cleanup_callback)
            , _instance(_hwa)
        {}

        /**
         * @brief Returns the constructed updater instance.
         *
         * @return Hardware-backed updater instance.
         */
        Updater& instance()
        {
            return _instance;
        }

        private:
        HwaHw   _hwa;
        Updater _instance;
    };
}    // namespace opendeck::updater
