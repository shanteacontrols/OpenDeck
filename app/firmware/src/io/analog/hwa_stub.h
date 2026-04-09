/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace io::analog
{
    /**
     * @brief Stub analog hardware adapter used when no backend is available.
     */
    class HwaStub : public Hwa
    {
        public:
        HwaStub() = default;

        /**
         * @brief Reports that the stub backend cannot initialize hardware.
         *
         * @return Always `false`.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Deinitializes the stub backend.
         */
        void deinit() override
        {
        }

        /**
         * @brief Returns no sampled frame.
         *
         * @return Always `std::nullopt`.
         */
        std::optional<Frame> read() override
        {
            return {};
        }
    };
}    // namespace io::analog
