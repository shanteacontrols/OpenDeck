/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"
#include "fw_selector/common.h"
#include "io/base.h"
#include "protocol/base.h"
#include "database/database.h"

#include <array>
#include <span>

namespace opendeck::sys
{
    /**
     * @brief Collection of top-level I/O subsystem instances indexed by `io::Io`.
     */
    using IoCollection = std::array<io::Base*, static_cast<size_t>(io::Io::Count)>;

    /**
     * @brief Collection of protocol subsystem instances indexed by `protocol::Protocol`.
     */
    using ProtocolCollection = std::array<protocol::Base*, static_cast<size_t>(protocol::Protocol::Count)>;

    /**
     * @brief Hardware abstraction used by the system coordinator.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Initializes the system-level hardware backend.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Reboots into the requested firmware target.
         *
         * @param type Firmware target to reboot into.
         */
        virtual void reboot(fw_selector::FwType type) = 0;

        /**
         * @brief Reads the hardware serial number bytes.
         *
         * @return View into HWA-owned serial number bytes, or an empty span when unavailable.
         */
        virtual std::span<uint8_t> serial_number() = 0;

        /**
         * @brief Returns the registered top-level I/O subsystems.
         *
         * @return Array of I/O subsystem pointers.
         */
        virtual IoCollection& io() = 0;

        /**
         * @brief Returns the registered protocol subsystems.
         *
         * @return Array of protocol subsystem pointers.
         */
        virtual ProtocolCollection& protocol() = 0;

        /**
         * @brief Returns the shared database administrator.
         *
         * @return Database administrator instance.
         */
        virtual database::Admin& database() = 0;
    };
}    // namespace opendeck::sys
