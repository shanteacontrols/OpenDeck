/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "global/midi_program.h"
#include "protocol/midi/midi.h"
#include "signaling/signaling.h"
#include "util/incdec/inc_dec.h"

#include <array>
#include <optional>

namespace opendeck::io::encoders
{
    /**
     * @brief Maps logical encoder movement into OSC, MIDI, and system-facing outputs.
     */
    class Mapper
    {
        public:
        /**
         * @brief Result of one mapped encoder update.
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
         * @brief Returns one mapped result for the current encoder movement.
         *
         * @param index Encoder input index being processed.
         * @param position Filtered encoder movement direction.
         * @param steps Effective movement step count after acceleration handling.
         *
         * @return Mapped protocol result when the movement should publish output, otherwise empty.
         */
        std::optional<Result> result(size_t index, Position position, uint8_t steps);

        /**
         * @brief Rebuilds the last published result for one encoder input.
         *
         * @param index Encoder input index to query.
         *
         * @return Reconstructed result when one exists, otherwise empty.
         */
        std::optional<Result> last_result(size_t index) const;

        /**
         * @brief Clears cached mapper state for one encoder input.
         *
         * @param index Encoder input index to reset.
         */
        void reset(size_t index);

        /**
         * @brief Stores a remotely synchronized value for one encoder input.
         *
         * @param index Encoder input index to update.
         * @param value New cached value.
         */
        void set_value(size_t index, uint16_t value);

        private:
        using ValueIncDecMidi7Bit  = util::IncDec<uint8_t, 0, protocol::midi::MAX_VALUE_7BIT>;
        using ValueIncDecMidi14Bit = util::IncDec<uint16_t, 0, protocol::midi::MAX_VALUE_14BIT>;

        struct DatabaseInfo
        {
            Type                        type            = Type::ControlChange7fh01h;
            bool                        inverted        = false;
            size_t                      component_index = 0;
            uint8_t                     channel         = 0;
            uint16_t                    midi_index      = 0;
            uint16_t                    repeated_value  = 0;
            uint16_t                    lower_limit     = 0;
            uint16_t                    upper_limit     = 0;
            protocol::midi::MessageType midi_message    = protocol::midi::MessageType::Invalid;
        };

        static constexpr uint8_t VAL_CONTROL_CHANGE_7FH01H[3] = {
            0,
            127,
            1
        };

        static constexpr uint8_t VAL_CONTROL_CHANGE_3FH41H[3] = {
            0,
            63,
            65
        };

        static constexpr uint8_t VAL_CONTROL_CHANGE_41H01H[3] = {
            0,
            65,
            1
        };

        Database&                               _database;
        std::array<int16_t, Collection::size()> _value = {};

        /**
         * @brief Reads runtime mapping configuration for one encoder from the database.
         *
         * @param index Encoder input index to query.
         *
         * @return Database-backed runtime info for the selected encoder.
         */
        DatabaseInfo read_database_info(size_t index, Position position) const;

        /**
         * @brief Fills one MIDI signal from encoder mapping configuration.
         *
         * @param signal Output storage for the MIDI signal.
         * @param info Runtime mapping configuration.
         * @param value Effective mapped encoder value.
         */
        void fill_midi_signal(signaling::MidiIoSignal& signal, const DatabaseInfo& info, uint16_t value) const;

        /**
         * @brief Fills one OSC signal from the mapped encoder value.
         *
         * @param signal Output storage for the OSC signal.
         * @param info Runtime mapping configuration.
         * @param value Effective mapped encoder value.
         */
        void fill_osc_signal(signaling::OscIoSignal& signal, const DatabaseInfo& info, uint16_t value) const;

        /**
         * @brief Checks whether one encoder type keeps a refreshable cached value.
         *
         * @param type Encoder type to check.
         *
         * @return `true` if the type has a meaningful cached value, otherwise `false`.
         */
        bool has_refresh_value(Type type) const;
    };
}    // namespace opendeck::io::encoders
