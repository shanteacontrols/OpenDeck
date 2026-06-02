/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/shared/deps.h"
#include "firmware/src/io/base.h"
#include "firmware/src/io/shared/common.h"
#include "firmware/src/threads.h"

#include <optional>
#include <vector>

namespace opendeck::firmware::io::i2c
{
    /**
     * @brief Top-level I2C subsystem that owns and updates registered peripherals.
     */
    class I2c : public io::Base
    {
        public:
        /**
         * @brief Constructs the I2C subsystem.
         *
         * @param hwa Hardware abstraction used for bus init and device probing.
         */
        explicit I2c(HwaBase& hwa);

        ~I2c() override;

        /**
         * @brief Initializes the subsystem and registered peripherals.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Deinitializes the subsystem.
         */
        void deinit() override;

        /**
         * @brief Registers one peripheral for subsystem-managed updates.
         *
         * @param instance Peripheral instance to register.
         */
        static void register_peripheral(Peripheral* instance);

        private:
        static constexpr int64_t DEVICE_PROBE_INTERVAL_MS = 1000;

        struct PeripheralState
        {
            Peripheral* instance      = nullptr;
            bool        initialized   = false;
            size_t      address_index = 0;
            int64_t     next_probe_ms = 0;
        };

        static inline std::vector<PeripheralState> peripherals = {};
        HwaBase&                                   _hwa;
        threads::I2cThread                         _thread;

        /**
         * @brief Updates every registered peripheral.
         */
        void update_peripherals();

        /**
         * @brief Probes a registered peripheral's candidate addresses.
         *
         * @param peripheral Peripheral to probe.
         *
         * @return Responding address index, or empty if none responds.
         */
        std::optional<size_t> find_address(Peripheral& peripheral);

        /**
         * @brief Returns one candidate address by index.
         *
         * @param peripheral Peripheral containing the address list.
         * @param address_index Index into the peripheral address list.
         *
         * @return Candidate 7-bit I2C address.
         */
        static uint8_t address_at(Peripheral& peripheral, size_t address_index);

        /**
         * @brief Updates one peripheral state slot.
         *
         * @param state State to update.
         * @param now_ms Current uptime in milliseconds.
         */
        void update_peripheral(PeripheralState& state, int64_t now_ms);

        /**
         * @brief Stops subsystem activity and releases runtime resources.
         */
        void shutdown();
    };
}    // namespace opendeck::firmware::io::i2c
