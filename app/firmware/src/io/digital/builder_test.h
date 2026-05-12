/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "io/base.h"
#include "io/digital/switches/builder.h"
#include "io/digital/encoders/builder.h"
#include "database/builder_test.h"

namespace opendeck::io::digital
{
    /**
     * @brief Test-only digital subsystem wrapper used by the test builder.
     */
    class DigitalTest : public io::Base
    {
        public:
        /**
         * @brief Constructs the test wrapper around switch and encoder subsystems.
         *
         * @param switches Switch subsystem instance.
         * @param encoders Encoder subsystem instance.
         */
        DigitalTest(io::switches::Switches& switches,
                    io::encoders::Encoders& encoders)
            : _switches(switches)
            , _encoders(encoders)
        {}

        /**
         * @brief Initializes the wrapped switch and encoder subsystems.
         *
         * @return `true` if both subsystems initialized successfully, otherwise `false`.
         */
        bool init() override
        {
            return _switches.init() && _encoders.init();
        }

        /**
         * @brief Deinitializes the test wrapper.
         */
        void deinit() override
        {
        }

        /**
         * @brief Forwards queued state changes to child subsystems.
         */
        void process_state_changes()
        {
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_SWITCHES
            _switches.process_state_changes();
#endif
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS
            _encoders.process_state_changes();
#endif
        }

        /**
         * @brief Forces child subsystems to refresh their output state.
         */
        void force_refresh_all()
        {
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_SWITCHES
            _switches.force_refresh(0, _switches.refreshable_components());
#endif
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS
            _encoders.force_refresh(0, _encoders.refreshable_components());
#endif
        }

        private:
        io::switches::Switches& _switches;
        io::encoders::Encoders& _encoders;
    };

    /**
     * @brief Test builder that wires switch and encoder test subsystems together.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the digital test builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _builderSwitches(database)
            , _builderEncoders(database)
            , _instance(_builderSwitches.instance(), _builderEncoders.instance())
        {}

        /**
         * @brief Returns the constructed digital test wrapper.
         *
         * @return Test digital subsystem instance.
         */
        DigitalTest& instance()
        {
            return _instance;
        }

        io::switches::Builder _builderSwitches;
        io::encoders::Builder _builderEncoders;
        DigitalTest           _instance;
    };
}    // namespace opendeck::io::digital
