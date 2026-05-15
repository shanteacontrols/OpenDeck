/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/indicators/deps.h"
#include "firmware/src/io/base.h"
#include "firmware/src/signaling/signaling.h"

#include "zlibs/utils/misc/kwork_delayable.h"

namespace opendeck::io::indicators
{
    /**
     * @brief Top-level subsystem that drives transport traffic and status indicators.
     */
    class Indicators : public io::Base
    {
        public:
        /** @brief Timeout used for transient traffic indications. */
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
        zlibs::utils::misc::KworkDelayable _network_in_off_work;
        zlibs::utils::misc::KworkDelayable _network_out_off_work;
        bool                               _factory_reset_in_progress = false;
        bool                               _invert                    = false;
        bool                               _network_up                = false;

        /**
         * @brief Handles one traffic notification.
         *
         * @param signal Transport traffic event to visualize.
         */
        void on_traffic(const signaling::TrafficSignal& signal);

        /**
         * @brief Handles a network identity update.
         *
         * @param identity Current advertised network identity.
         */
        void on_network_identity(const signaling::NetworkIdentitySignal& identity);

        /**
         * @brief Applies the idle state to one indicator.
         *
         * @param type Indicator to update.
         */
        void set_idle(Type type);

        /**
         * @brief Applies the active traffic state to one indicator.
         *
         * @param type Indicator to update.
         */
        void set_active(Type type);

        /**
         * @brief Schedules automatic return to idle state for the selected indicator.
         *
         * @param type Indicator to return to idle later.
         */
        void schedule_idle(Type type);

        /**
         * @brief Cancels all pending indicator idle work.
         */
        void cancel_idle_work();

        /**
         * @brief Applies the remembered network status indicator state.
         */
        void apply_network_up_state();

        /**
         * @brief Maps transport and direction to a concrete indicator output.
         *
         * @param transport Transport that carried the traffic.
         * @param direction Direction of the traffic.
         *
         * @return Indicator type that represents the traffic source.
         */
        Type indicator_type(signaling::TrafficTransport transport, signaling::SignalDirection direction);

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
}    // namespace opendeck::io::indicators
