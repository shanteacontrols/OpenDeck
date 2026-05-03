/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "io/base.h"
#include "signaling/signaling.h"
#include "global/midi_program.h"
#include "system/config.h"
#include "protocol/midi/midi.h"

#include <optional>

namespace opendeck::io::encoders
{
    /**
     * @brief Reads encoder movement, tracks per-encoder state, and publishes the configured actions.
     */
    class Encoders : public io::Base
    {
        public:
        /**
         * @brief Constructs an encoder controller bound to hardware, filtering, and database services.
         *
         * @param hwa Hardware abstraction used to sample encoder states.
         * @param filter Direction filter used to debounce encoder movement.
         * @param database Database interface used to read encoder configuration.
         */
        Encoders(Hwa&      hwa,
                 Filter&   filter,
                 Database& database);

        /**
         * @brief Shuts the controller down.
         */
        ~Encoders() override;

        /**
         * @brief Resets all encoder runtime state to their configured defaults.
         *
         * @return Always `true`.
         */
        bool init() override;

        /**
         * @brief Shuts the controller down.
         */
        void deinit() override;

        /**
         * @brief Returns the number of refreshable encoders.
         */
        size_t refreshable_components() const override;

        /**
         * @brief Republishes the current output state for a staged subset of encoders.
         *
         * @param start_index First encoder index to refresh.
         * @param count Number of encoder indices to refresh.
         */
        void force_refresh(size_t start_index, size_t count) override;

        /**
         * @brief Samples every enabled encoder and processes any detected movement.
         */
        void process_state_changes();

        /**
         * @brief Resets cached runtime state for a single encoder.
         *
         * @param index Encoder index to reset.
         */
        void reset(size_t index);

        private:
        /**
         * @brief Runtime description of an encoder action resolved from the database.
         */
        struct Descriptor
        {
            Type                  type   = Type::ControlChange7fh01h;
            signaling::MidiSignal signal = {};
        };

        using ValueIncDecMidi7Bit  = util::IncDec<uint8_t, 0, protocol::midi::MAX_VALUE_7BIT>;
        using ValueIncDecMidi14Bit = util::IncDec<uint16_t, 0, protocol::midi::MAX_VALUE_14BIT>;

        static constexpr uint32_t ENCODERS_SPEED_TIMEOUT_MS = 140;
        static constexpr int8_t   ENCODER_LOOK_UP_TABLE[16] = {
            0,     // 0000
            1,     // 0001
            -1,    // 0010
            0,     // 0011
            -1,    // 0100
            0,     // 0101
            0,     // 0110
            1,     // 0111
            1,     // 1000
            0,     // 1001
            0,     // 1010
            -1,    // 1011
            0,     // 1100
            -1,    // 1101
            1,     // 1110
            0      // 1111
        };

        static constexpr uint8_t ENCODER_SPEED_CHANGE[static_cast<uint8_t>(Acceleration::Count)] = {
            0,    // acceleration disabled
            1,
            2,
            3
        };

        static constexpr uint8_t ENCODER_ACCELERATION_STEP_INC[static_cast<uint8_t>(Acceleration::Count)] = {
            0,    // acceleration disabled
            5,
            10,
            100
        };

        static constexpr uint8_t VAL_CONTROL_CHANGE_7FH01H[3] = {
            0,      // Stopped
            127,    // Ccw
            1       // Cw
        };

        static constexpr uint8_t VAL_CONTROL_CHANGE_3FH41H[3] = {
            0,     // Stopped
            63,    // Ccw
            65     // Cw
        };

        static constexpr uint8_t VAL_CONTROL_CHANGE_41H01H[3] = {
            0,     // Stopped
            65,    // Ccw
            1      // Cw
        };

        static constexpr std::array<protocol::midi::MessageType, static_cast<uint8_t>(Type::Count)> INTERNAL_MSG_TO_MIDI_TYPE = {
            protocol::midi::MessageType::ControlChange,         // ControlChange7fh01h
            protocol::midi::MessageType::ControlChange,         // ControlChange3fh41h
            protocol::midi::MessageType::ProgramChange,         // ProgramChange
            protocol::midi::MessageType::ControlChange,         // ControlChange
            protocol::midi::MessageType::Invalid,               // PresetChange
            protocol::midi::MessageType::PitchBend,             // PitchBend
            protocol::midi::MessageType::Nrpn7Bit,              // Nrpn7Bit
            protocol::midi::MessageType::Nrpn14Bit,             // Nrpn14Bit
            protocol::midi::MessageType::ControlChange14Bit,    // ControlChange14Bit
            protocol::midi::MessageType::ControlChange,         // Reserved
            protocol::midi::MessageType::Invalid,               // BpmChange
            protocol::midi::MessageType::NoteOn,                // SingleNoteVariableVal
            protocol::midi::MessageType::NoteOn,                // SingleNoteFixedValBothDir
            protocol::midi::MessageType::NoteOn,                // SingleNoteFixedValOneDir0OtherDir
            protocol::midi::MessageType::NoteOn,                // TwoNoteFixedValBothDir
        };

        Hwa&      _hwa;
        Filter&   _filter;
        Database& _database;

        std::array<int16_t, Collection::size()> _value          = {};
        std::array<uint8_t, Collection::size()> _encoder_speed  = {};
        std::array<uint8_t, Collection::size()> _encoder_data   = {};
        std::array<int8_t, Collection::size()>  _encoder_pulses = {};

        /**
         * @brief Builds the runtime descriptor for an encoder in the specified direction.
         *
         * @param index Encoder index to describe.
         * @param position Movement direction used to resolve direction-dependent settings.
         * @param descriptor Output storage for the populated descriptor.
         */
        void fill_descriptor(size_t index, Position position, Descriptor& descriptor);

        /**
         * @brief Decodes one sampled encoder pin pair into a logical movement step.
         *
         * @param index Encoder index being sampled.
         * @param pair_state Current A/B pin state packed into bits 0 and 1.
         *
         * @return Decoded encoder direction, or `Position::Stopped` when no full step was detected.
         */
        Position read(size_t index, uint8_t pair_state);

        /**
         * @brief Processes one sampled encoder reading and emits any resulting action.
         *
         * @param index Encoder index being processed.
         * @param pair_value Current A/B pin state packed into bits 0 and 1.
         * @param sample_time Timestamp of the sample in milliseconds.
         */
        void process_reading(size_t index, uint8_t pair_value, uint32_t sample_time);

        /**
         * @brief Publishes the action configured for a detected encoder movement.
         *
         * @param index Encoder index being processed.
         * @param position Detected movement direction.
         * @param descriptor Runtime descriptor describing the configured action.
         * @param ignore_freeze When `true`, allows refresh-triggered output while the subsystem is frozen.
         */
        void send_message(size_t index, Position position, Descriptor& descriptor, bool ignore_freeze = false);

        /**
         * @brief Stores a remotely synchronized value for an encoder.
         *
         * @param index Encoder index to update.
         * @param value New cached value.
         */
        void set_value(size_t index, uint16_t value);

        /**
         * @brief Serves SysEx configuration reads for the encoder block.
         *
         * @param section Encoder configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Encoder section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the encoder block.
         *
         * @param section Encoder configuration section being written.
         * @param index Parameter index within the section.
         * @param value Value to store.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::Encoder section, size_t index, uint16_t value);

        /**
         * @brief Performs shutdown work for the controller.
         */
        void shutdown();
    };
}    // namespace opendeck::io::encoders
