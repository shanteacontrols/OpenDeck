/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "io/base.h"
#include "messaging/messaging.h"

#include "zlibs/utils/misc/kwork_delayable.h"

namespace io::indicators
{
    /**
     * @brief Top-level subsystem that drives MIDI traffic and status indicators.
     */
    class Indicators : public io::Base
    {
        public:
        /** @brief Timeout used for transient MIDI traffic indications. */
        static constexpr uint32_t TRAFFIC_INDICATOR_TIMEOUT_MS = 50;
        /** @brief Timeout used for factory-reset flashing. */
        static constexpr uint32_t FACTORY_RESET_INDICATOR_TIMEOUT_MS = 250;
        /** @brief On-time used for startup flashes. */
        static constexpr uint32_t STARTUP_INDICATOR_TIMEOUT_MS = 150;
        /** @brief Number of startup flashes emitted during initialization. */
        static constexpr size_t STARTUP_INDICATOR_FLASH_COUNT = 3;

        /**
         * @brief Constructs the indicators subsystem.
         *
         * @param hwa Hardware abstraction used to drive indicator outputs.
         */
        explicit Indicators(Hwa& hwa);
        ~Indicators() override;

        /**
         * @brief Initializes the subsystem and subscribes to indicator-driving events.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Deinitializes the subsystem and turns indicators off.
         */
        void deinit() override;

        private:
        Hwa&                               _hwa;
        zlibs::utils::misc::KworkDelayable _usb_in_off_work;
        zlibs::utils::misc::KworkDelayable _usb_out_off_work;
        zlibs::utils::misc::KworkDelayable _din_in_off_work;
        zlibs::utils::misc::KworkDelayable _din_out_off_work;
        zlibs::utils::misc::KworkDelayable _ble_in_off_work;
        zlibs::utils::misc::KworkDelayable _ble_out_off_work;
        bool                               _factory_reset_in_progress = false;

        /**
         * @brief Handles one MIDI traffic notification.
         *
         * @param signal MIDI traffic event to visualize.
         */
        void on_traffic(const messaging::MidiTrafficSignal& signal);

        /**
         * @brief Schedules automatic turn-off for the selected indicator.
         *
         * @param type Indicator to turn off later.
         */
        void schedule_off(Type type);

        /**
         * @brief Maps transport and direction to a concrete indicator output.
         *
         * @param transport MIDI transport that carried the message.
         * @param direction Direction of the MIDI message.
         *
         * @return Indicator type that represents the traffic source.
         */
        Type indicator_type(messaging::MidiTransport transport, messaging::MidiDirection direction);

        /**
         * @brief Sets the state of all input indicators.
         *
         * @param state `true` to enable them, `false` to disable them.
         */
        void set_input_indicators(bool state);

        /**
         * @brief Sets the state of all output indicators.
         *
         * @param state `true` to enable them, `false` to disable them.
         */
        void set_output_indicators(bool state);

        /**
         * @brief Emits the startup indicator animation.
         */
        void indicate_startup();

        /**
         * @brief Stops any pending work and turns all indicators off.
         */
        void shutdown();
    };
}    // namespace io::indicators
