/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "protocol/midi/midi.h"
#include "signaling/signaling.h"

#include <array>
#include <optional>

namespace opendeck::io::buttons
{
    /**
     * @brief Maps logical button states into OSC, MIDI, and system-facing outputs.
     */
    class Mapper
    {
        public:
        /**
         * @brief Result of one mapped button update.
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
         * @brief Resolves the effective button behavior type for one message type.
         *
         * Some MIDI message types override the configured button type for MIDI behavior,
         * while OSC should continue to follow the configured type.
         *
         * @param message_type Configured button message type.
         * @param configured_type Configured button behavior type.
         *
         * @return Effective behavior type for that message type.
         */
        static Type message_to_type(MessageType message_type, Type configured_type);

        /**
         * @brief Returns one mapped result for the current logical button state.
         *
         * @param index Button input index being processed.
         * @param reading Current filtered logical button state.
         *
         * @return Mapped protocol result when the button state changed, otherwise empty.
         */
        std::optional<Result> result(size_t index, bool reading);

        /**
         * @brief Rebuilds the last published result for one button input.
         *
         * @param index Button input index to query.
         *
         * @return Reconstructed result when one exists, otherwise empty.
         */
        std::optional<Result> refresh_result(size_t index);

        /**
         * @brief Clears cached mapper state for one button input.
         *
         * @param index Button input index to reset.
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
         * @brief Reads runtime mapping configuration for one button from the database.
         *
         * @param index Button input index to query.
         *
         * @return Database-backed runtime info for the selected button.
         */
        DatabaseInfo read_database_info(size_t index) const;

        /**
         * @brief Builds one mapped output result for the provided logical button state.
         *
         * @param index Button input index being processed.
         * @param reading Current filtered logical button state.
         * @param info Runtime mapping configuration.
         * @param mutate `true` to update cached latching/inc-dec state, `false` to only rebuild output.
         * @param refresh `true` when reconstructing the last published state for refresh replay.
         *
         * @return Mapped result when at least one protocol output should be published, otherwise empty.
         */
        std::optional<Result> build_result(size_t index, bool reading, const DatabaseInfo& info, bool mutate, bool refresh);

        /**
         * @brief Fills one MIDI signal from button mapping configuration.
         *
         * @param signal Output storage for the MIDI signal.
         * @param info Runtime mapping configuration.
         */
        void fill_midi_signal(signaling::MidiIoSignal& signal, const DatabaseInfo& info) const;

        /**
         * @brief Fills one OSC signal from the effective logical button state.
         *
         * @param result Output storage for the mapped result.
         * @param index Button input index being processed.
         * @param state Effective OSC-facing button state after momentary/latching handling.
         */
        void fill_osc_signal(Result& result, size_t index, bool state) const;
    };
}    // namespace opendeck::io::buttons
