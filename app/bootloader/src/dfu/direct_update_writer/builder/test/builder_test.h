/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/hwa/test/hwa_test.h"
#include "bootloader/src/dfu/direct_update_writer/instance/impl/direct_update_writer.h"

namespace opendeck::bootloader::dfu::direct_update_writer
{
    /**
     * @brief Convenience builder that wires the test direct-update writer backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the test direct-update writer builder.
         */
        Builder()
            : _instance(_hwa, _hwa)
        {}

        /**
         * @brief Returns the constructed direct-update writer instance.
         *
         * @return Test-backed direct-update writer instance.
         */
        DirectUpdateWriter& instance()
        {
            return _instance;
        }

        HwaTest _hwa;

        private:
        DirectUpdateWriter _instance;
    };
}    // namespace opendeck::bootloader::dfu::direct_update_writer
