/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "io/base.h"
#include "io/digital/buttons/builder.h"
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
         * @brief Constructs the test wrapper around button and encoder subsystems.
         *
         * @param buttons Button subsystem instance.
         * @param encoders Encoder subsystem instance.
         */
        DigitalTest(io::buttons::Buttons&   buttons,
                    io::encoders::Encoders& encoders)
            : _buttons(buttons)
            , _encoders(encoders)
        {}

        /**
         * @brief Initializes the wrapped button and encoder subsystems.
         *
         * @return `true` if both subsystems initialized successfully, otherwise `false`.
         */
        bool init() override
        {
            return _buttons.init() && _encoders.init();
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
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BUTTONS
            _buttons.process_state_changes();
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
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BUTTONS
            _buttons.force_refresh(0, _buttons.refreshable_components());
#endif
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS
            _encoders.force_refresh(0, _encoders.refreshable_components());
#endif
        }

        private:
        io::buttons::Buttons&   _buttons;
        io::encoders::Encoders& _encoders;
    };

    /**
     * @brief Test builder that wires button and encoder test subsystems together.
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
            : _builderButtons(database)
            , _builderEncoders(database)
            , _instance(_builderButtons.instance(), _builderEncoders.instance())
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

        io::buttons::Builder  _builderButtons;
        io::encoders::Builder _builderEncoders;
        DigitalTest           _instance;
    };
}    // namespace opendeck::io::digital
