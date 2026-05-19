/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/analog/shared/deps.h"
#include "firmware/src/io/analog/drivers/remap.h"

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
        explicit HwaHw(Hwa& driver)
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
         * @brief Returns the next sampled logical frame from the underlying driver.
         *
         * @return Sampled frame, or `std::nullopt` when no frame is ready.
         */
        std::optional<Frame> read() override
        {
            const auto driver_frame = _driver.read();

            if (!driver_frame.has_value())
            {
                return {};
            }

            Frame frame = {};

            for (size_t i = 0; i < Collection::size(GroupAnalogInputs); i++)
            {
                const auto physical_index = Remap::physical(i);

                if (physical_index < driver_frame.value().size())
                {
                    frame[i] = driver_frame.value()[physical_index];
                }
            }

            return frame;
        }

        /**
         * @brief Updates which physical analog channels should be scanned.
         *
         * @param mask Logical-channel scan mask.
         */
        void set_scan_mask(const ScanMask& mask) override
        {
            ScanMask physical_mask = {};

            for (size_t i = 0; i < Collection::size(GroupAnalogInputs); i++)
            {
                const auto physical_index = Remap::physical(i);

                if ((i < mask.size()) && (physical_index < physical_mask.size()))
                {
                    physical_mask[physical_index] = mask[i];
                }
            }

            _driver.set_scan_mask(physical_mask);
        }

        private:
        Hwa& _driver;
    };
}    // namespace opendeck::io::analog
