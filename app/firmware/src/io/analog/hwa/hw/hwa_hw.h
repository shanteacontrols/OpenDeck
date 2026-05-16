/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/analog/shared/deps.h"
#include "firmware/src/io/analog/drivers/driver_base.h"

namespace opendeck::io::analog
{
    /**
     * @brief Hardware-backed analog subsystem adapter.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Constructs the adapter around an analog driver instance.
         *
         * @param driver Driver implementation used to access hardware samples.
         */
        explicit HwaHw(DriverBase& driver)
            : _driver(driver)
        {}

        /**
         * @brief Initializes the underlying analog driver.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init() override
        {
            return _driver.init();
        }

        /**
         * @brief Deinitializes the underlying analog driver.
         */
        void deinit() override
        {
            _driver.deinit();
        }

        /**
         * @brief Returns the next sampled frame from the underlying driver.
         *
         * @return Sampled frame, or `std::nullopt` when no frame is ready.
         */
        std::optional<Frame> read() override
        {
            return _driver.read();
        }

        /**
         * @brief Updates which physical analog channels should be scanned.
         *
         * @param mask Physical-channel scan mask.
         */
        void set_scan_mask(const ScanMask& mask) override
        {
            _driver.set_scan_mask(mask);
        }

        private:
        DriverBase& _driver;
    };
}    // namespace opendeck::io::analog
