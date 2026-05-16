/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/midi/deps.h"

#include <gmock/gmock.h>

namespace opendeck::protocol::midi
{
    /**
     * @brief Helper that records and decodes MIDI messages written by test backends.
     */
    class WrittenMessageLog
    {
        public:
        /**
         * @brief Appends one UMP packet after decoding it into a logical message.
         *
         * @param packet UMP packet to decode and store.
         */
        void push(const midi_ump& packet)
        {
            _decoded.push_back(decode_message(packet));
        }

        /**
         * @brief Feeds one MIDI 1.0 byte into the internal parser.
         *
         * @param data MIDI byte to parse.
         */
        void push(uint8_t data)
        {
            if (const auto packet = _parser.parse(data, 0); packet.has_value())
            {
                push(packet.value());
            }
        }

        /**
         * @brief Returns the decoded written-message log.
         *
         * @return Mutable vector of decoded messages.
         */
        std::vector<Message>& written_messages()
        {
            return _decoded;
        }

        /**
         * @brief Counts logged messages that carry MIDI channel data.
         *
         * @return Number of logged channel messages.
         */
        size_t total_written_channel_messages() const
        {
            size_t count = 0;

            for (const auto& message : _decoded)
            {
                if (is_channel_message(message.type))
                {
                    count++;
                }
            }

            return count;
        }

        /**
         * @brief Clears both decoded messages and parser state.
         */
        void clear()
        {
            _decoded.clear();
            _parser.reset();
        }

        private:
        zlibs::utils::midi::Midi1ByteToUmpParser _parser  = {};
        std::vector<Message>                     _decoded = {};
    };

    /**
     * @brief Test USB MIDI backend backed by in-memory packet queues.
     */
    class HwaUsbTest : public HwaUsb
    {
        public:
        /**
         * @brief Constructs the USB test backend and initializes its poll signal.
         */
        HwaUsbTest()
        {
            k_poll_signal_init(&_data_available_signal);
        }

        /**
         * @brief Returns the poll signal used for USB input readiness.
         *
         * @return USB data-available poll signal.
         */
        k_poll_signal* data_available_signal() override
        {
            return &_data_available_signal;
        }

        /**
         * @brief Returns whether the USB test backend is ready for packet exchange.
         *
         * @return Always `true`.
         */
        bool ready() override
        {
            return true;
        }

        /**
         * @brief Initializes the backend, clears buffered packets, and signals readiness.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            clear();
            k_poll_signal_reset(&_data_available_signal);

            return true;
        }

        /**
         * @brief Deinitializes the backend and clears buffered packets.
         *
         * @return Always `true`.
         */
        bool deinit() override
        {
            clear();
            k_poll_signal_reset(&_data_available_signal);
            return true;
        }

        /**
         * @brief Records one written UMP packet.
         *
         * @param packet Packet written by the MIDI subsystem.
         *
         * @return Always `true`.
         */
        bool write(const midi_ump& packet) override
        {
            _writePackets.push_back(packet);
            _writeParser.push(packet);
            return true;
        }

        /**
         * @brief Returns and removes the next queued USB input packet.
         *
         * @return Queued packet, or `std::nullopt` when none is available.
         */
        std::optional<midi_ump> read() override
        {
            if (_readPackets.empty())
            {
                return {};
            }

            const auto packet = _readPackets.front();
            _readPackets.erase(_readPackets.begin());
            return packet;
        }

        /**
         * @brief Clears queued packets and decoded write logs.
         */
        void clear()
        {
            _readPackets.clear();
            _writePackets.clear();
            _writeParser.clear();
        }

        std::vector<midi_ump> _readPackets  = {};
        std::vector<midi_ump> _writePackets = {};
        WrittenMessageLog     _writeParser;

        private:
        k_poll_signal _data_available_signal = {};
    };

    /**
     * @brief Test serial MIDI backend backed by in-memory byte queues.
     */
    class HwaSerialTest : public HwaSerial
    {
        public:
        /**
         * @brief Constructs the serial test backend and initializes its poll signal.
         */
        HwaSerialTest()
        {
            k_poll_signal_init(&_data_available_signal);
        }

        /**
         * @brief Returns the poll signal used for serial input readiness.
         *
         * @return Serial data-available poll signal.
         */
        k_poll_signal* data_available_signal() override
        {
            return &_data_available_signal;
        }

        MOCK_METHOD0(init, bool());
        MOCK_METHOD0(deinit, bool());
        MOCK_METHOD1(set_loopback, bool(bool state));

        /**
         * @brief Records one written serial MIDI byte.
         *
         * @param data MIDI byte written by the subsystem.
         *
         * @return Always `true`.
         */
        bool write(uint8_t data) override
        {
            _writePackets.push_back(data);
            _writeParser.push(data);
            return true;
        }

        /**
         * @brief Returns and removes the next queued serial MIDI byte.
         *
         * @return Queued byte, or `std::nullopt` when none is available.
         */
        std::optional<uint8_t> read() override
        {
            if (_readPackets.empty())
            {
                return {};
            }

            const auto data = _readPackets.front();
            _readPackets.erase(_readPackets.begin());
            return data;
        }

        /**
         * @brief Returns whether the requested interface is considered allocated.
         *
         * @param interface Interface to query.
         *
         * @return `true` only for UART when loopback is enabled, otherwise `false`.
         */
        bool allocated(io::common::Allocatable::Interface interface) override
        {
            return interface == io::common::Allocatable::Interface::Uart ? _loopbackEnabled : false;
        }

        /**
         * @brief Clears queued bytes, decoded write logs, and loopback state.
         */
        void clear()
        {
            _readPackets.clear();
            _writePackets.clear();
            _writeParser.clear();
            _loopbackEnabled = false;
        }

        std::vector<uint8_t> _readPackets     = {};
        std::vector<uint8_t> _writePackets    = {};
        bool                 _loopbackEnabled = false;
        WrittenMessageLog    _writeParser;

        private:
        k_poll_signal _data_available_signal = {};
    };

    /**
     * @brief Test BLE MIDI backend backed by in-memory packet queues.
     */
    class HwaBleTest : public HwaBle
    {
        public:
        /**
         * @brief Constructs the BLE test backend and initializes its poll signal.
         */
        HwaBleTest()
        {
            k_poll_signal_init(&_data_available_signal);
        }

        /**
         * @brief Returns the poll signal used for BLE input readiness.
         *
         * @return BLE data-available poll signal.
         */
        k_poll_signal* data_available_signal() override
        {
            return &_data_available_signal;
        }

        /**
         * @brief Returns whether the BLE test backend is ready for packet transmission.
         *
         * @return Current ready state of the test BLE backend.
         */
        bool ready() override
        {
            return _ready;
        }

        MOCK_METHOD0(init, bool());
        MOCK_METHOD0(deinit, bool());

        /**
         * @brief Records one written BLE MIDI packet.
         *
         * @param packet BLE packet written by the subsystem.
         *
         * @return Always `true`.
         */
        bool write(BlePacket& packet) override
        {
            _writePackets.push_back(packet);
            return true;
        }

        /**
         * @brief Returns and removes the next queued BLE input packet.
         *
         * @return Queued packet, or `std::nullopt` when none is available.
         */
        std::optional<BlePacket> read() override
        {
            if (_readPackets.empty())
            {
                return {};
            }

            const auto packet = _readPackets.front();
            _readPackets.erase(_readPackets.begin());
            return packet;
        }

        void clear()
        {
            _readPackets.clear();
            _writePackets.clear();
        }

        std::vector<BlePacket> _readPackets  = {};
        std::vector<BlePacket> _writePackets = {};
        WrittenMessageLog      _writeParser;
        bool                   _ready = true;

        private:
        k_poll_signal _data_available_signal = {};
    };
}    // namespace opendeck::protocol::midi
