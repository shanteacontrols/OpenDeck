/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "frame_store.h"
#include "drivers/driver_base.h"
#include "io/base.h"
#include "io/digital/buttons/buttons.h"
#include "io/digital/encoders/encoders.h"
#include "threads.h"

namespace opendeck::io::digital
{
    /**
     * @brief Top-level digital-input subsystem that coordinates buttons and encoders.
     */
    class Digital : public io::Base
    {
        public:
        /**
         * @brief Constructs the digital subsystem around its driver and child components.
         *
         * @param driver Low-level digital scan driver.
         * @param frame_store Shared frame cache populated from the driver.
         * @param buttons Button subsystem instance.
         * @param encoders Encoder subsystem instance.
         */
        Digital(drivers::DriverBase&    driver,
                FrameStore&             frame_store,
                io::buttons::Buttons&   buttons,
                io::encoders::Encoders& encoders);
        ~Digital() override;

        /**
         * @brief Initializes the digital subsystem and its child components.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Deinitializes the digital subsystem.
         */
        void deinit() override;

        /**
         * @brief Returns the number of refreshable digital components.
         */
        size_t refreshable_components() const override;

        /**
         * @brief Republishes a staged subset of button and encoder state.
         *
         * @param start_index First digital component index to refresh.
         * @param count Number of digital component indices to refresh.
         */
        void force_refresh(size_t start_index, size_t count) override;

        private:
        drivers::DriverBase&    _driver;
        FrameStore&             _frame_store;
        io::buttons::Buttons&   _buttons;
        io::encoders::Encoders& _encoders;
        threads::DigitalThread  _thread;

        /**
         * @brief Stops subsystem activity and releases runtime resources.
         */
        void shutdown();
    };
}    // namespace opendeck::io::digital
