/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

namespace io::touchscreen
{
    /**
     * @brief Stub touchscreen backend that reports no hardware support.
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
         * @brief Reports that the stub backend cannot deinitialize hardware.
         *
         * @return Always `false`.
         */
        bool deinit() override
        {
            return false;
        }

        /**
         * @brief Rejects all write requests.
         *
         * @param value Byte that would be transmitted.
         *
         * @return Always `false`.
         */
        bool write([[maybe_unused]] uint8_t value) override
        {
            return false;
        }

        /**
         * @brief Returns no received data.
         *
         * @return Always `std::nullopt`.
         */
        std::optional<uint8_t> read() override
        {
            return {};
        }

        /**
         * @brief Reports that no allocatable interface is owned by the stub backend.
         *
         * @param interface Interface to query.
         *
         * @return Always `false`.
         */
        bool allocated([[maybe_unused]] io::common::Allocatable::Interface interface) override
        {
            return false;
        }
    };
}    // namespace io::touchscreen
