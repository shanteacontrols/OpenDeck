/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/shared/deps.h"
#include "firmware/src/io/digital/drivers/remap.h"

namespace opendeck::firmware::io::digital
{
    /**
     * @brief Hardware-backed digital subsystem adapter.
     */
    class HwaHw : public Hwa
    {
        public:
        explicit HwaHw(Hwa& driver)
            : _driver(driver)
        {}

        bool init() override
        {
            return _driver.init();
        }

        std::optional<Frame> read() override
        {
            const auto driver_frame = _driver.read();

            if (!driver_frame.has_value())
            {
                return {};
            }

            Frame frame = {};

            for (size_t i = 0; i < driver_frame.value().size(); i++)
            {
                const auto logical_index = Remap::logical(i);

                if (logical_index < frame.size())
                {
                    frame[logical_index] = driver_frame.value()[i];
                }
            }

            return frame;
        }

        size_t encoder_count() const override
        {
            return _driver.encoder_count();
        }

        size_t switch_to_encoder_index(size_t index) override
        {
            return _driver.switch_to_encoder_index(index);
        }

        size_t encoder_component_from_encoder(size_t index, EncoderComponent component) override
        {
            return _driver.encoder_component_from_encoder(index, component);
        }

        private:
        Hwa& _driver;
    };
}    // namespace opendeck::firmware::io::digital
