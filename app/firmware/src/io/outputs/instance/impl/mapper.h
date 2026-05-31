/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/shared/deps.h"
#include "firmware/src/protocol/midi/shared/common.h"
#include "firmware/src/signaling/signaling.h"

#include <array>
#include <optional>

namespace opendeck::io::outputs
{
    /**
     * @brief Maps incoming protocol and internal events into output actions.
     */
    class Mapper
    {
        public:
        /**
         * @brief Output action requested by one mapped event.
         */
        struct Action
        {
            enum class LevelCommand
            {
                None,
                Set,
                Write,
            };

            LevelCommand              level_command = LevelCommand::None;
            uint8_t                   level         = OUTPUT_LEVEL_MIN;
            std::optional<PulseSpeed> pulse_speed   = {};
        };

        using Result = std::array<Action, Collection::size()>;

        explicit Mapper(Database& database)
            : _database(database)
        {}

        /**
         * @brief Maps one MIDI event into output actions.
         *
         * @param message Decoded MIDI message to process.
         * @param direction Traffic direction associated with the message.
         *
         * @return Output actions requested by the MIDI event.
         */
        Result midi_result(const protocol::midi::Message& message, signaling::SignalDirection direction);

        /**
         * @brief Maps one preset change into output actions.
         *
         * @param preset Active preset index.
         *
         * @return Output actions requested by the preset change.
         */
        Result preset_result(uint8_t preset);

        /**
         * @brief Maps one OSC output write into an output action.
         *
         * @param value Incoming OSC integer value.
         *
         * @return Output action requested by the OSC event.
         */
        Action osc_result(int32_t value) const;

        private:
        Database& _database;

        /**
         * @brief Converts a MIDI value into a pulse speed.
         *
         * @param value MIDI value to convert.
         *
         * @return Pulse speed selected by the MIDI value.
         */
        PulseSpeed midi_value_to_pulse_speed(uint8_t value) const;

        /**
         * @brief Converts a MIDI value into an output level percentage.
         *
         * @param value MIDI value to convert.
         *
         * @return Output level percentage.
         */
        uint8_t midi_value_to_level(uint8_t value) const;
    };
}    // namespace opendeck::io::outputs
