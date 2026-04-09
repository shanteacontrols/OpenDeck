/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

#include "io/base.h"
#include "io/common/common.h"
#include "threads.h"

#include <array>

namespace io::i2c
{
    /**
     * @brief Top-level I2C subsystem that owns and updates registered peripherals.
     */
    class I2c : public io::Base
    {
        public:
        /**
         * @brief Constructs the I2C subsystem.
         */
        I2c();

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
        static constexpr size_t MAX_PERIPHERALS = 5;

        threads::I2cThread                                     _thread;
        static inline size_t                                   peripheral_counter = 0;
        static inline std::array<Peripheral*, MAX_PERIPHERALS> peripherals        = {};

        /**
         * @brief Updates every registered peripheral.
         */
        void update_peripherals();

        /**
         * @brief Stops subsystem activity and releases runtime resources.
         */
        void shutdown();
    };
}    // namespace io::i2c
