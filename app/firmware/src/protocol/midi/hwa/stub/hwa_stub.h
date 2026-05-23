/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/midi/shared/deps.h"

namespace opendeck::protocol::midi
{
    /**
     * @brief Stub USB MIDI backend used when USB MIDI is disabled.
     */
    class HwaUsbStub : public HwaUsb
    {
        public:
        using HwaUsb::write;

        HwaUsbStub()
        {
            k_poll_signal_init(&_data_available_signal);
        }

        /**
         * @brief Returns the USB input poll signal.
         *
         * @return Stub input poll signal.
         */
        k_poll_signal* data_available_signal() override
        {
            return &_data_available_signal;
        }

        /**
         * @brief Reports that USB MIDI is not supported.
         *
         * @return Always `false`.
         */
        bool supported() override
        {
            return false;
        }

        /**
         * @brief Reports that USB MIDI is not ready.
         *
         * @return Always `false`.
         */
        bool ready() override
        {
            return false;
        }

        /**
         * @brief Initializes the stub backend.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            k_poll_signal_reset(&_data_available_signal);
            return true;
        }

        /**
         * @brief Deinitializes the stub backend.
         *
         * @return Always `true`.
         */
        bool deinit() override
        {
            k_poll_signal_reset(&_data_available_signal);
            return true;
        }

        /**
         * @brief Drops one USB MIDI packet.
         *
         * @param packet Packet to drop.
         *
         * @return Always `false`.
         */
        bool write([[maybe_unused]] const midi_ump& packet) override
        {
            return false;
        }

        /**
         * @brief Returns no USB MIDI packet.
         *
         * @return Always `std::nullopt`.
         */
        std::optional<midi_ump> read() override
        {
            return {};
        }

        private:
        k_poll_signal _data_available_signal = {};
    };

    /**
     * @brief Stub serial/DIN MIDI backend used when DIN MIDI is disabled.
     */
    class HwaSerialStub : public HwaSerial
    {
        public:
        HwaSerialStub()
        {
            k_poll_signal_init(&_data_available_signal);
        }

        /**
         * @brief Returns the serial input poll signal.
         *
         * @return Stub input poll signal.
         */
        k_poll_signal* data_available_signal() override
        {
            return &_data_available_signal;
        }

        /**
         * @brief Reports that DIN MIDI is not supported.
         *
         * @return Always `false`.
         */
        bool supported() override
        {
            return false;
        }

        /**
         * @brief Initializes the stub backend.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            k_poll_signal_reset(&_data_available_signal);
            return true;
        }

        /**
         * @brief Deinitializes the stub backend.
         *
         * @return Always `true`.
         */
        bool deinit() override
        {
            k_poll_signal_reset(&_data_available_signal);
            return true;
        }

        /**
         * @brief Ignores loopback changes.
         *
         * @param state Requested loopback state.
         *
         * @return Always `false`.
         */
        bool set_loopback([[maybe_unused]] bool state) override
        {
            return false;
        }

        /**
         * @brief Drops one serial MIDI byte.
         *
         * @param data Byte to drop.
         *
         * @return Always `false`.
         */
        bool write([[maybe_unused]] uint8_t data) override
        {
            return false;
        }

        /**
         * @brief Returns no serial MIDI byte.
         *
         * @return Always `std::nullopt`.
         */
        std::optional<uint8_t> read() override
        {
            return {};
        }

        /**
         * @brief Reports that no interface is allocated.
         *
         * @param interface Interface to query.
         *
         * @return Always `false`.
         */
        bool allocated([[maybe_unused]] io::common::Allocatable::Interface interface) override
        {
            return false;
        }

        private:
        k_poll_signal _data_available_signal = {};
    };

    /**
     * @brief Stub BLE MIDI backend used when BLE MIDI is disabled.
     */
    class HwaBleStub : public HwaBle
    {
        public:
        HwaBleStub()
        {
            k_poll_signal_init(&_data_available_signal);
        }

        /**
         * @brief Returns the BLE input poll signal.
         *
         * @return Stub input poll signal.
         */
        k_poll_signal* data_available_signal() override
        {
            return &_data_available_signal;
        }

        /**
         * @brief Reports that BLE MIDI is not supported.
         *
         * @return Always `false`.
         */
        bool supported() override
        {
            return false;
        }

        /**
         * @brief Initializes the stub backend.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            k_poll_signal_reset(&_data_available_signal);
            return true;
        }

        /**
         * @brief Deinitializes the stub backend.
         *
         * @return Always `true`.
         */
        bool deinit() override
        {
            k_poll_signal_reset(&_data_available_signal);
            return true;
        }

        /**
         * @brief Drops one BLE MIDI packet.
         *
         * @param packet Packet to drop.
         *
         * @return Always `false`.
         */
        bool write([[maybe_unused]] BlePacket& packet) override
        {
            return false;
        }

        /**
         * @brief Returns no BLE MIDI packet.
         *
         * @return Always `std::nullopt`.
         */
        std::optional<BlePacket> read() override
        {
            return {};
        }

        /**
         * @brief Reports that BLE MIDI is not ready.
         *
         * @return Always `false`.
         */
        bool ready() override
        {
            return false;
        }

        private:
        k_poll_signal _data_available_signal = {};
    };
}    // namespace opendeck::protocol::midi
