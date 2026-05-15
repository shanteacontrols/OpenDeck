/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/midi/deps.h"
#include "firmware/src/io/common/common.h"
#include "firmware/src/protocol/base.h"
#include "firmware/src/database/database.h"
#include "firmware/src/system/config.h"
#include "firmware/src/protocol/base.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/threads.h"

#include "zlibs/utils/misc/kwork_delayable.h"
#include "zlibs/utils/misc/timer.h"

namespace opendeck::protocol::midi
{
    /**
     * @brief Top-level MIDI protocol subsystem coordinating all active transports.
     */
    class Midi : public protocol::Base
    {
        public:
        Midi(HwaUsb&    hwa_usb,
             HwaSerial& hwa_serial,
             HwaBle&    hwa_ble,
             Database&  database);
        ~Midi() override;

        /**
         * @brief Initializes active MIDI transports, routing, and worker state.
         *
         * @return `true` when initialization completed successfully.
         */
        bool init() override;

        /**
         * @brief Deinitializes active MIDI transports and stops background processing.
         *
         * @return `true` when deinitialization completed successfully.
         */
        bool deinit() override;

        private:
        /**
         * @brief Identifies the transport backends managed by the MIDI protocol thread.
         */
        enum class Interface : uint8_t
        {
            Usb,
            Serial,
            Ble,
            Count
        };

        HwaUsb&                                                                      _hwa_usb;
        HwaSerial&                                                                   _hwa_serial;
        HwaBle&                                                                      _hwa_ble;
        zlibs::utils::midi::usb::Usb                                                 _usb    = zlibs::utils::midi::usb::Usb(_hwa_usb);
        zlibs::utils::midi::serial::Serial                                           _serial = zlibs::utils::midi::serial::Serial(_hwa_serial);
        zlibs::utils::midi::ble::Ble                                                 _ble    = zlibs::utils::midi::ble::Ble(_hwa_ble);
        Database&                                                                    _database;
        std::array<zlibs::utils::midi::Base*, static_cast<size_t>(Interface::Count)> _midi_interface = {};
        zlibs::utils::misc::Timer                                                    _clock_timer;
        std::array<k_poll_event, static_cast<size_t>(Interface::Count)>              _poll_events = {};
        threads::MidiThread                                                          _thread;
        bool                                                                         _initialized       = false;
        bool                                                                         _standard_note_off = true;
        bool                                                                         _din_loopback      = false;
        bool                                                                         _burst_midi_active = false;
        std::array<midi_ump, USB_UMP_BURST_PACKET_COUNT>                             _usb_burst_packets = {};
        size_t                                                                       _usb_burst_count   = 0;
        size_t                                                                       _usb_burst_size    = 0;

        /**
         * @brief Reads a boolean MIDI feature flag from the global configuration section.
         *
         * @param feature MIDI setting to query.
         *
         * @return `true` when the setting is enabled, otherwise `false`.
         */
        bool is_setting_enabled(Setting feature);

        /**
         * @brief Determines whether DIN loopback should currently be enabled.
         *
         * @return `true` when the active MIDI routing requires DIN loopback.
         */
        bool is_din_loopback_required();

        /**
         * @brief Applies the desired DIN loopback state to the serial backend.
         *
         * @return `true` when the backend accepted the requested loopback state.
         */
        bool apply_din_loopback();

        /**
         * @brief Handles SysEx reads for global MIDI settings.
         *
         * @param section Global configuration section being queried.
         * @param index Setting index within the section.
         * @param value Output value populated on success.
         *
         * @return Configuration status byte, or `std::nullopt` when the request
         *         does not belong to the MIDI subsystem.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Global section, size_t index, uint16_t& value);

        /**
         * @brief Handles SysEx writes for global MIDI settings.
         *
         * @param section Global configuration section being updated.
         * @param index Setting index within the section.
         * @param value New value requested by the caller.
         *
         * @return Configuration status byte, or `std::nullopt` when the request
         *         does not belong to the MIDI subsystem.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::Global section, size_t index, uint16_t value);

        /**
         * @brief Converts and sends one MIDI 1.0 event to the enabled transports.
         *
         * @param event MIDI-interpreted IO event to transmit.
         */
        void send(const signaling::MidiIoSignal& event);

        /**
         * @brief Sends one prebuilt UMP event to the enabled transports.
         *
         * @param event UMP signal to transmit.
         */
        void send(const signaling::UmpSignal& event);

        /**
         * @brief Updates the note-off encoding mode used for outbound events.
         *
         * @param type Desired note-off representation.
         */
        void set_note_off_mode(NoteOffType type);

        /**
         * @brief Initializes and registers the USB MIDI transport.
         *
         * @return `true` when USB setup succeeded or is not required.
         */
        bool setup_usb();

        /**
         * @brief Initializes and registers the DIN MIDI transport.
         *
         * @return `true` when serial setup succeeded or is not required.
         */
        bool setup_serial();

        /**
         * @brief Initializes and registers the BLE MIDI transport.
         *
         * @return `true` when BLE setup succeeded or is not required.
         */
        bool setup_ble();

        /**
         * @brief Applies current thru-routing settings across active transports.
         *
         * @return `true` when every requested thru route is configured.
         */
        bool setup_thru();

        /**
         * @brief Main polling loop that receives incoming data from MIDI transports.
         */
        void read_loop();

        /**
         * @brief Processes one readable transport selected by poll index.
         *
         * @param interface_index Index into the polled transport table.
         */
        void read_interface(size_t interface_index);

        /**
         * @brief Maps an internal transport index to the public transport enum.
         *
         * @param index Internal transport slot index.
         *
         * @return Matching public MIDI transport identifier.
         */
        signaling::TrafficTransport interface_to_transport(size_t index) const;

        /**
         * @brief Adds one UMP packet to the pending USB burst buffer.
         *
         * @param packet UMP packet to append.
         *
         * @return `true` when the packet fit into the current burst buffer.
         */
        bool append_usb_burst_packet(const midi_ump& packet);

        /**
         * @brief Flushes the currently accumulated USB UMP burst.
         *
         * @return `true` when the burst was sent successfully or was empty.
         */
        bool flush_usb_burst();

        /**
         * @brief Shuts down active transports and stops the MIDI worker thread.
         *
         * @return `true` when shutdown completed successfully.
         */
        bool shutdown();
    };
}    // namespace opendeck::protocol::midi
