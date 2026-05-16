/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/switches/shared/deps.h"
#include "firmware/src/protocol/midi/instance/impl/midi.h"
#include "firmware/src/signaling/signaling.h"

#include <array>
#include <optional>

namespace opendeck::io::switches
{
    /**
     * @brief Maps logical switch states into OSC, MIDI, and system-facing outputs.
     */
    class Mapper
    {
        public:
        /**
         * @brief Result of one mapped switch update.
         */
        struct Result
        {
            std::optional<signaling::OscIoSignal>  osc    = {};
            std::optional<signaling::MidiIoSignal> midi   = {};
            std::optional<signaling::SystemSignal> system = {};
        };

        explicit Mapper(Database& database)
            : _database(database)
        {}

        /**
         * @brief Resolves the effective switch behavior type for one message type.
         *
         * Some MIDI message types override the configured switch type for MIDI behavior,
         * while OSC should continue to follow the configured type.
         *
         * @param message_type Configured switch message type.
         * @param configured_type Configured switch behavior type.
         *
         * @return Effective behavior type for that message type.
         */
        static Type message_to_type(MessageType message_type, Type configured_type);

        /**
         * @brief Returns one mapped result for the current logical switch state.
         *
         * @param index Switch input index being processed.
         * @param reading Current filtered logical switch state.
         *
         * @return Mapped protocol result when the switch state changed, otherwise empty.
         */
        std::optional<Result> result(size_t index, bool reading);

        /**
         * @brief Rebuilds the last published result for one switch input.
         *
         * @param index Switch input index to query.
         *
         * @return Reconstructed result when one exists, otherwise empty.
         */
        std::optional<Result> refresh_result(size_t index);

        /**
         * @brief Clears cached mapper state for one switch input.
         *
         * @param index Switch input index to reset.
         */
        void reset(size_t index);

        private:
        struct DatabaseInfo
        {
            Type                        type            = Type::Momentary;
            MessageType                 message_type    = MessageType::Note;
            size_t                      component_index = 0;
            uint8_t                     channel         = 0;
            uint16_t                    midi_index      = 0;
            uint16_t                    midi_value      = 0;
            protocol::midi::MessageType midi_message    = protocol::midi::MessageType::Invalid;
        };

        Database&                               _database;
        std::array<bool, Collection::size()>    _pressed_state       = {};
        std::array<uint8_t, Collection::size()> _inc_dec_value       = {};
        std::array<bool, Collection::size()>    _osc_latching_state  = {};
        std::array<bool, Collection::size()>    _midi_latching_state = {};

        /**
         * @brief Reads runtime mapping configuration for one switch from the database.
         *
         * @param index Switch input index to query.
         *
         * @return Database-backed runtime info for the selected switch.
         */
        DatabaseInfo read_database_info(size_t index) const;

        /**
         * @brief Builds one mapped output result for the provided logical switch state.
         *
         * @param index Switch input index being processed.
         * @param reading Current filtered logical switch state.
         * @param info Runtime mapping configuration.
         * @param mutate `true` to update cached latching/inc-dec state, `false` to only rebuild output.
         * @param refresh `true` when reconstructing the last published state for refresh replay.
         *
         * @return Mapped result when at least one protocol output should be published, otherwise empty.
         */
        std::optional<Result> build_result(size_t index, bool reading, const DatabaseInfo& info, bool mutate, bool refresh);

        /**
         * @brief Fills one MIDI signal from switch mapping configuration.
         *
         * @param signal Output storage for the MIDI signal.
         * @param info Runtime mapping configuration.
         */
        void fill_midi_signal(signaling::MidiIoSignal& signal, const DatabaseInfo& info) const;

        /**
         * @brief Fills one OSC signal from the effective logical switch state.
         *
         * @param result Output storage for the mapped result.
         * @param index Switch input index being processed.
         * @param state Effective OSC-facing switch state after momentary/latching handling.
         */
        void fill_osc_signal(Result& result, size_t index, bool state) const;
    };
}    // namespace opendeck::io::switches
