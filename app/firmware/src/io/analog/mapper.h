/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "signaling/signaling.h"

#include <array>
#include <optional>

namespace opendeck::io::analog
{
    /**
     * @brief Maps stable analog positions into protocol-facing OSC and MIDI values.
     */
    class Mapper
    {
        public:
        /**
         * @brief Cached mapped output value used for duplicate suppression and refresh replay.
         */
        struct Value
        {
            bool     fsr_pressed = false;
            uint16_t value       = 0;
            uint16_t max_value   = 0;
            bool     valid       = false;
        };

        /**
         * @brief Result of one mapped continuous analog update.
         */
        struct Result
        {
            std::optional<signaling::OscIoSignal>  osc  = {};
            std::optional<signaling::MidiIoSignal> midi = {};
        };

        explicit Mapper(Database& database)
            : _database(database)
        {}

        /**
         * @brief Returns one mapped result for the current analog position.
         *
         * @param index Analog input index being processed.
         * @param position Stable physical position produced by the filter.
         *
         * @return Mapped protocol result when the value changed, otherwise empty.
         */
        std::optional<Result> result(size_t index, uint16_t position);

        /**
         * @brief Rebuilds the last published result for one analog input.
         *
         * @param index Analog input index to query.
         *
         * @return Reconstructed result when one exists, otherwise empty.
         */
        std::optional<Result> last_result(size_t index) const;

        /**
         * @brief Clears cached duplicate-suppression state for one analog input.
         *
         * @param index Analog input index to reset.
         */
        void reset(size_t index);

        private:
        struct DatabaseInfo
        {
            Type                        type            = Type::PotentiometerControlChange;
            bool                        inverted        = false;
            uint16_t                    lower_limit     = 0;
            uint16_t                    upper_limit     = 0;
            size_t                      component_index = 0;
            uint8_t                     channel         = 0;
            uint16_t                    midi_index      = 0;
            protocol::midi::MessageType midi_message    = protocol::midi::MessageType::Invalid;
            uint16_t                    midi_max_value  = protocol::midi::MAX_VALUE_7BIT;
        };

        Database&                             _database;
        std::array<Value, Collection::size()> _last_value = {};

        /**
         * @brief Builds one button-style passthrough result from the received logical value.
         *
         * @param info Runtime mapping configuration.
         * @param value Filtered button value to forward.
         *
         * @return Ready-to-publish button result.
         */
        Result button_result(const DatabaseInfo& info, uint16_t value) const;

        /**
         * @brief Reads runtime mapping configuration for one analog input from the database.
         *
         * @param index Analog input index to query.
         *
         * @return Database-backed runtime info for the selected analog input.
         */
        DatabaseInfo read_database_info(size_t index) const;

        /**
         * @brief Fills one MIDI signal from the mapped analog value.
         *
         * @param signal Output storage for the MIDI signal.
         * @param info Runtime mapping configuration.
         * @param value Effective mapped analog value.
         * @param message MIDI message type to publish.
         */
        void fill_midi_signal(signaling::MidiIoSignal& signal, const DatabaseInfo& info, uint16_t value, protocol::midi::MessageType message) const;

        /**
         * @brief Fills one OSC signal from the analog position and runtime mapping configuration.
         *
         * @param position Stable physical position produced by the filter.
         * @param info Runtime mapping configuration.
         * @param signal Output storage for the OSC signal.
         */
        void fill_osc_signal(uint16_t position, const DatabaseInfo& info, signaling::OscIoSignal& signal) const;

        /**
         * @brief Computes the effective mapped analog value for one stable position.
         *
         * @param position Stable physical position produced by the filter.
         * @param info Runtime mapping configuration.
         *
         * @return Mapped integer value in the descriptor-selected output range.
         */
        uint16_t compute_value(uint16_t position, const DatabaseInfo& info) const;

        /**
         * @brief Resolves the effective MIDI index for one analog mapping configuration.
         *
         * @param info Runtime mapping configuration.
         *
         * @return Effective MIDI index after 7-bit/14-bit adjustment.
         */
        uint16_t midi_index(const DatabaseInfo& info) const;

        /**
         * @brief Resolves the effective output range for one analog mapping configuration.
         *
         * @param info Runtime mapping configuration.
         *
         * @return Maximum integer output value for the configured analog type.
         */
        uint16_t output_max_value(const DatabaseInfo& info) const;

        /**
         * @brief Checks whether one analog type uses the 7-bit MIDI value range.
         *
         * @param type Analog input type to check.
         *
         * @return `true` if the type uses the 7-bit range, otherwise `false`.
         */
        bool is_7bit_type(Type type) const;
    };
}    // namespace opendeck::io::analog
