/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "protocol/midi/midi.h"
#include "global/midi_program.h"
#include "system/config.h"
#include "io/base.h"

#include <optional>

namespace io::buttons
{
    /**
     * @brief Samples buttons, tracks latching state, and publishes the configured actions.
     */
    class Buttons : public io::Base
    {
        public:
        /**
         * @brief Constructs a button controller bound to hardware, filtering, and database services.
         *
         * @param hwa Hardware abstraction used to sample button states.
         * @param filter Debounce filter used to validate state changes.
         * @param database Database interface used to read button configuration.
         */
        Buttons(Hwa&      hwa,
                Filter&   filter,
                Database& database);

        ~Buttons() override = default;

        /**
         * @brief Initializes button hardware and clears cached runtime state.
         *
         * @return `true` if the hardware initialized successfully, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Deinitializes the button controller.
         */
        void deinit() override;

        /**
         * @brief Returns the number of refreshable digital button inputs.
         */
        size_t refreshable_components() const override;

        /**
         * @brief Republishes the current output state for a staged subset of digital buttons.
         *
         * @param start_index First button index to refresh.
         * @param count Number of button indices to refresh.
         */
        void force_refresh(size_t start_index, size_t count) override;

        /**
         * @brief Samples digital buttons and processes any filtered state changes.
         */
        void process_state_changes();

        /**
         * @brief Clears cached pressed and latching state for one button.
         *
         * @param index Button index to reset.
         */
        void reset(size_t index);

        private:
        /**
         * @brief Runtime description of a button action resolved from the database.
         */
        struct Descriptor
        {
            Type                  type         = Type::Momentary;
            MessageType           message_type = MessageType::Note;
            messaging::MidiSignal signal       = {};
        };

        using ValueIncDecMidi7Bit = util::IncDec<uint8_t, 0, protocol::midi::MAX_VALUE_7BIT>;

        static constexpr size_t STATE_STORAGE_DIVISOR = 8;

        static constexpr std::array<protocol::midi::MessageType, static_cast<uint8_t>(MessageType::Count)> INTERNAL_MSG_TO_MIDI_TYPE = {
            protocol::midi::MessageType::NoteOn,                      // Note
            protocol::midi::MessageType::ProgramChange,               // ProgramChange
            protocol::midi::MessageType::ControlChange,               // ControlChange
            protocol::midi::MessageType::ControlChange,               // ControlChangeReset
            protocol::midi::MessageType::MmcStop,                     // MmcStop
            protocol::midi::MessageType::MmcPlay,                     // MmcPlay
            protocol::midi::MessageType::MmcRecordStart,              // MmcRecord - modified to stop when needed
            protocol::midi::MessageType::MmcPause,                    // MmcPause
            protocol::midi::MessageType::SysRealTimeClock,            // RealTimeClock
            protocol::midi::MessageType::SysRealTimeStart,            // RealTimeStart
            protocol::midi::MessageType::SysRealTimeContinue,         // RealTimeContinue
            protocol::midi::MessageType::SysRealTimeStop,             // RealTimeStop
            protocol::midi::MessageType::SysRealTimeActiveSensing,    // RealTimeActiveSensing
            protocol::midi::MessageType::SysRealTimeSystemReset,      // RealTimeSystemReset
            protocol::midi::MessageType::ProgramChange,               // ProgramChangeInc
            protocol::midi::MessageType::ProgramChange,               // ProgramChangeDec
            protocol::midi::MessageType::Invalid,                     // None
            protocol::midi::MessageType::Invalid,                     // PresetChange
            protocol::midi::MessageType::NoteOn,                      // MultiValIncResetNote
            protocol::midi::MessageType::NoteOn,                      // MultiValIncDecNote
            protocol::midi::MessageType::ControlChange,               // MultiValIncResetCc
            protocol::midi::MessageType::ControlChange,               // MultiValIncDecCc
            protocol::midi::MessageType::NoteOn,                      // NoteOffOnly
            protocol::midi::MessageType::ControlChange,               // ControlChange0Only
            protocol::midi::MessageType::Invalid,                     // Reserved
            protocol::midi::MessageType::Invalid,                     // ProgramChangeOffsetInc
            protocol::midi::MessageType::Invalid,                     // ProgramChangeOffsetDec
            protocol::midi::MessageType::Invalid,                     // BpmInc
            protocol::midi::MessageType::Invalid,                     // BpmDec
            protocol::midi::MessageType::MmcPlay,                     // MmcPlayStop - modified to stop when needed
        };

        Hwa&      _hwa;
        Filter&   _filter;
        Database& _database;
        uint8_t   _button_pressed[Collection::size() / STATE_STORAGE_DIVISOR + 1]      = {};
        uint8_t   _last_latching_state[Collection::size() / STATE_STORAGE_DIVISOR + 1] = {};
        uint8_t   _inc_dec_value[Collection::size()]                                   = {};

        /**
         * @brief Returns the cached pressed state for a button.
         *
         * @param index Button index to query.
         *
         * @return `true` if the cached state is pressed, otherwise `false`.
         */
        bool cached_state(size_t index);

        /**
         * @brief Reads the current physical state for a button when it is not claimed by an encoder.
         *
         * @param index Button index to read.
         *
         * @return Physical button state, or `std::nullopt` when the index is unavailable for button use.
         */
        std::optional<bool> state(size_t index);

        /**
         * @brief Builds the runtime descriptor for a button.
         *
         * @param index Button index to describe.
         * @param descriptor Output storage for the populated descriptor.
         */
        void fill_descriptor(size_t index, Descriptor& descriptor);

        /**
         * @brief Processes one button state change and emits any configured action.
         *
         * @param index Button index being processed.
         * @param reading New sampled state.
         * @param descriptor Runtime descriptor describing the configured action.
         */
        void process_button(size_t index, bool reading, Descriptor& descriptor);

        /**
         * @brief Publishes the action configured for a button event.
         *
         * @param index Button index being processed.
         * @param state Logical button state to publish.
         * @param descriptor Runtime descriptor describing the configured action.
         * @param ignore_freeze When `true`, allows refresh-triggered output while the subsystem is frozen.
         */
        void send_message(size_t index, bool state, Descriptor& descriptor, bool ignore_freeze = false);

        /**
         * @brief Stores the cached physical pressed state for a button.
         *
         * @param index Button index to update.
         * @param state Cached pressed state to store.
         */
        void set_state(size_t index, bool state);

        /**
         * @brief Stores the cached logical latching state for a button.
         *
         * @param index Button index to update.
         * @param state Cached latching state to store.
         */
        void set_latching_state(size_t index, bool state);

        /**
         * @brief Returns the cached logical latching state for a button.
         *
         * @param index Button index to query.
         *
         * @return `true` if the button is latched on, otherwise `false`.
         */
        bool latching_state(size_t index);

        /**
         * @brief Serves SysEx configuration reads for the button block.
         *
         * @param section Button configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Button section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the button block.
         *
         * @param section Button configuration section being written.
         * @param index Parameter index within the section.
         * @param value Value to store.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::Button section, size_t index, uint16_t value);
    };
}    // namespace io::buttons
