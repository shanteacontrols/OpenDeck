/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/touchscreen/shared/common.h"
#include "firmware/src/io/common/shared/common.h"
#include "firmware/src/database/instance/impl/database.h"

#include <optional>

namespace opendeck::io::touchscreen
{
    /**
     * @brief Database view used by the touchscreen subsystem.
     */
    using Database = database::User<database::Config::Section::Touchscreen>;

    /**
     * @brief Hardware abstraction used by the touchscreen subsystem.
     */
    class Hwa : public io::common::Allocatable
    {
        public:
        ~Hwa() override = default;

        /**
         * @brief Initializes the touchscreen backend.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Deinitializes the touchscreen backend.
         *
         * @return `true` if deinitialization succeeded, otherwise `false`.
         */
        virtual bool deinit() = 0;

        /**
         * @brief Writes one byte to the touchscreen transport.
         *
         * @param value Byte to transmit.
         *
         * @return `true` if the byte was written, otherwise `false`.
         */
        virtual bool write(uint8_t value) = 0;

        /**
         * @brief Reads one byte from the touchscreen transport.
         *
         * @return Received byte, or `std::nullopt` when none is available.
         */
        virtual std::optional<uint8_t> read() = 0;

        /**
         * @brief Reports that no allocatable interface is owned by default.
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
}    // namespace opendeck::io::touchscreen
