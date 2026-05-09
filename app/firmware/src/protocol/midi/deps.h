/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"
#include "io/common/common.h"
#include "database/database.h"

#include <zephyr/kernel.h>
#include <functional>
#include <span>

namespace opendeck::protocol::midi
{
    /**
     * @brief Database view used by the MIDI subsystem for global settings.
     */
    using Database = database::User<database::Config::Section::Global>;

    /**
     * @brief Callback invoked when the USB MIDI backend becomes ready.
     */
    using UsbReadyHandler = std::function<void()>;

    /**
     * @brief Hardware abstraction for the USB MIDI transport.
     */
    class HwaUsb : public zlibs::utils::midi::usb::Hwa
    {
        public:
        ~HwaUsb() override = default;
        using zlibs::utils::midi::Transport::write;

        /**
         * @brief Returns whether USB MIDI transport support is available.
         *
         * @return `true` if USB MIDI is supported, otherwise `false`.
         */
        virtual bool supported() = 0;

        /**
         * @brief Returns the poll signal raised when USB MIDI input becomes available.
         *
         * @return Poll signal used to wait for USB MIDI data.
         */
        virtual k_poll_signal* data_available_signal() = 0;

        /**
         * @brief Registers a callback invoked once the USB backend is ready.
         *
         * @param handler Callback to invoke on readiness.
         */
        virtual void register_on_ready_handler(UsbReadyHandler&& handler) = 0;

        /**
         * @brief Returns whether USB MIDI is ready for packet exchange.
         *
         * @return `true` when the USB MIDI function is ready for TX/RX.
         */
        virtual bool ready() = 0;

        /**
         * @brief Writes a burst of UMP packets through the USB backend.
         *
         * The default implementation forwards packets one by one through
         * `write(const midi_ump&)`.
         *
         * @param packets UMP packets to transmit.
         *
         * @return `true` if all packets were sent successfully, otherwise `false`.
         */
        virtual bool write(std::span<const midi_ump> packets)
        {
            for (const auto& packet : packets)
            {
                if (!this->write(packet))
                {
                    return false;
                }
            }

            return true;
        }
    };

    /**
     * @brief Hardware abstraction for the DIN/serial MIDI transport.
     */
    class HwaSerial : public io::common::Allocatable, public zlibs::utils::midi::serial::Hwa
    {
        public:
        ~HwaSerial() override = default;

        /**
         * @brief Returns whether serial MIDI transport support is available.
         *
         * @return `true` if serial MIDI is supported, otherwise `false`.
         */
        virtual bool supported() = 0;

        /**
         * @brief Enables or disables transport loopback.
         *
         * @param state Desired loopback state.
         *
         * @return `true` if the state was applied, otherwise `false`.
         */
        virtual bool set_loopback(bool state) = 0;

        /**
         * @brief Returns the poll signal raised when serial MIDI input becomes available.
         *
         * @return Poll signal used to wait for serial MIDI data.
         */
        virtual k_poll_signal* data_available_signal() = 0;
    };

    /**
     * @brief Hardware abstraction for the BLE MIDI transport.
     */
    class HwaBle : public zlibs::utils::midi::ble::Hwa
    {
        public:
        ~HwaBle() override = default;

        /**
         * @brief Returns whether BLE MIDI transport support is available.
         *
         * @return `true` if BLE MIDI is supported, otherwise `false`.
         */
        virtual bool supported() = 0;

        /**
         * @brief Returns the poll signal raised when BLE MIDI input becomes available.
         *
         * @return Poll signal used to wait for BLE MIDI data.
         */
        virtual k_poll_signal* data_available_signal() = 0;

        /**
         * @brief Returns whether the BLE MIDI transport is ready for packet exchange.
         *
         * @return `true` when BLE MIDI is connected and notifications are enabled.
         */
        virtual bool ready() = 0;
    };
}    // namespace opendeck::protocol::midi
