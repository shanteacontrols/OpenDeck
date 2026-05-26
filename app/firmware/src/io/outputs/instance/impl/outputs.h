/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/shared/deps.h"
#include "firmware/src/database/instance/impl/database.h"
#include "firmware/src/protocol/midi/instance/impl/midi.h"
#include "firmware/src/io/shared/common.h"
#include "firmware/src/system/config.h"
#include "firmware/src/io/base.h"
#include "firmware/src/threads.h"

#include "zlibs/utils/misc/mutex.h"

#include <zephyr/kernel.h>

#include <array>
#include <atomic>
#include <optional>

namespace opendeck::io::outputs
{
    /**
     * @brief Manages output state, pulsing, and refresh scheduling.
     */
    class Outputs : public io::Base
    {
        public:
        /**
         * @brief Constructs an output controller bound to hardware and database services.
         *
         * @param hwa Hardware abstraction used to update outputs.
         * @param database Database interface used to read output configuration.
         */
        Outputs(Hwa&      hwa,
                Database& database);

        /**
         * @brief Stops the output worker thread and releases runtime resources.
         */
        ~Outputs() override;

        /**
         * @brief Initializes the output subsystem and starts its worker thread.
         *
         * @return `true` if initialization completed successfully, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Stops the output subsystem worker thread.
         */
        void deinit() override;

        /**
         * @brief Returns the number of refreshable logical outputs.
         */
        size_t refreshable_components() const override;

        /**
         * @brief Republishes the current output state for a staged subset of logical outputs.
         *
         * @param start_index First output index to refresh.
         * @param count Number of output indices to refresh.
         */
        void force_refresh(size_t start_index, size_t count) override;

        /**
         * @brief Returns the configured pulse speed for the specified output.
         *
         * @param index output index to query.
         *
         * @return Pulse speed assigned to the output.
         */
        PulseSpeed pulse_speed(size_t index);

        /**
         * @brief Turns all outputs off.
         */
        void set_all_off();

        private:
        /**
         * @brief Bit positions used to encode per-output runtime state.
         */
        enum class OutputBit : uint8_t
        {
            Active,
            PulseOn,
            State
        };

        static constexpr size_t  TOTAL_PULSE_SPEEDS                         = 4;
        static constexpr uint8_t OUTPUT_PULSE_TIMER_MODE_CHECK_TIME         = 50;
        static constexpr size_t  STORAGE_SIZE                               = Collection::size() ? Collection::size() : 1;
        static constexpr uint8_t PULSE_RESET_MIDI_CLOCK[TOTAL_PULSE_SPEEDS] = {
            48,
            24,
            12,
            255,    // no pulsing
        };

        static constexpr uint8_t PULSE_RESET_TIMER[TOTAL_PULSE_SPEEDS] = {
            20,
            10,
            5,
            0,
        };

        static constexpr protocol::midi::MessageType CONTROL_TYPE_TO_MIDI_MESSAGE[static_cast<uint8_t>(ControlType::Count)] = {
            protocol::midi::MessageType::NoteOn,           // MIDI_IN_NOTE_SINGLE_VAL,
            protocol::midi::MessageType::NoteOn,           // LOCAL_NOTE_SINGLE_VAL,
            protocol::midi::MessageType::ControlChange,    // MIDI_IN_CC_SINGLE_VAL,
            protocol::midi::MessageType::ControlChange,    // LOCAL_CC_SINGLE_VAL,
            protocol::midi::MessageType::ProgramChange,    // PC_SINGLE_VAL,
            protocol::midi::MessageType::ProgramChange,    // Preset,
            protocol::midi::MessageType::NoteOn,           // MIDI_IN_NOTE_MULTI_VAL,
            protocol::midi::MessageType::NoteOn,           // LOCAL_NOTE_MULTI_VAL,
            protocol::midi::MessageType::ControlChange,    // MIDI_IN_CC_MULTI_VAL,
            protocol::midi::MessageType::ControlChange,    // LOCAL_CC_MULTI_VAL,
            protocol::midi::MessageType::Invalid,          // Static
        };

        Hwa&                      _hwa;
        Database&                 _database;
        zlibs::utils::misc::Mutex _state_mutex;

        std::array<uint8_t, STORAGE_SIZE> _output_state                      = {};
        std::array<uint8_t, STORAGE_SIZE> _level                             = {};
        std::array<uint8_t, STORAGE_SIZE> _pulse_timer                       = {};
        PulseMode                         _output_pulse_mode                 = PulseMode::Timer;
        const uint8_t*                    _pulse_reset_array_ptr             = nullptr;
        uint8_t                           _pulse_counter[TOTAL_PULSE_SPEEDS] = {};
        bool                              _pulse_state[TOTAL_PULSE_SPEEDS]   = {};
        uint32_t                          _last_output_pulse_update_time     = 0;

        threads::OutputsThread _thread;
        std::atomic<bool>      _force_refresh_pending = { false };
        struct k_sem           _update_semaphore      = {};

        /**
         * @brief Turns every output fully on.
         */
        void set_all_on();

        /**
         * @brief Forces every output into a non-pulsing on state.
         */
        void set_all_static_on();

        /**
         * @brief Queues an output refresh request for the worker thread.
         *
         * @param force_refresh `true` to force a full output refresh, otherwise `false`.
         */
        void request_update(bool force_refresh);

        /**
         * @brief Waits for a queued output refresh request.
         *
         * @return `true` if a queued update was received, otherwise `false`.
         */
        bool wait_for_update();

        /**
         * @brief Processes one queued or periodic output update cycle.
         *
         * @param force_refresh `true` to force a full output refresh, otherwise `false`.
         */
        void process_update(bool force_refresh);

        /**
         * @brief Updates the pulse speed assigned to one output.
         *
         * @param index output index to update.
         * @param state Pulse speed to apply.
         */
        void set_pulse_speed(size_t index, PulseSpeed state);

        /**
         * @brief Selects the timing source used for output pulsing.
         *
         * @param pulse_mode Pulse timing source to activate.
         */
        void set_pulse_mode(PulseMode pulse_mode);

        /**
         * @brief Clears all pulse counters and synchronized pulse states.
         */
        void reset_pulsing();

        /**
         * @brief Updates one logical state bit for an output.
         *
         * @param index output index to update.
         * @param bit Bit field to modify.
         * @param state Bit value to store.
         */
        void update_bit(size_t index, OutputBit bit, bool state);

        /**
         * @brief Returns one logical state bit for an output.
         *
         * @param index output index to query.
         * @param bit Bit field to read.
         *
         * @return `true` if the requested bit is set, otherwise `false`.
         */
        bool bit(size_t index, OutputBit bit);

        /**
         * @brief Clears cached runtime state for one output.
         *
         * @param index output index to reset.
         */
        void reset_state(size_t index);

        /**
         * @brief Converts a stored numeric value into a pulse speed.
         *
         * @param value Stored pulse-speed value.
         *
         * @return Pulse speed corresponding to the stored value.
         */
        PulseSpeed value_to_pulse_speed(uint8_t value);

        /**
         * @brief Converts a stored numeric value into a level percentage.
         *
         * @param value Stored level value.
         *
         * @return uint8_t level corresponding to the stored value.
         */
        uint8_t value_to_level(uint8_t value);

        /**
         * @brief Plays the startup output animation.
         */
        void start_up_animation();

        /**
         * @brief Applies an incoming MIDI event to the output state machine.
         *
         * @param message Decoded MIDI message to process.
         * @param direction Traffic direction associated with the message.
         */
        void midi_to_state(const protocol::midi::Message& message, signaling::SignalDirection direction);

        /**
         * @brief Updates internal preset-controlled outputs for the selected preset.
         *
         * @param preset Active preset index.
         */
        void internal_preset_to_state(uint8_t preset);

        /**
         * @brief Sets the logical level for one output.
         *
         * Updates cached state, then writes the level.
         *
         * @param index output index to update.
         * @param level Logical level percentage to store and write.
         */
        void set_level(size_t index, uint8_t level);

        /**
         * @brief Writes an effective level for one output.
         *
         * Publishes and writes the level without changing cached state.
         *
         * @param index output index to update.
         * @param level Effective level percentage to write.
         */
        void write_level(size_t index, uint8_t level);

        /**
         * @brief Serves SysEx configuration reads for the output block.
         *
         * @param section output configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Outputs section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the output block.
         *
         * @param section output configuration section being written.
         * @param index Parameter index within the section.
         * @param value Value to store.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::Outputs section, size_t index, uint16_t value);

        /**
         * @brief Performs shutdown work for the controller.
         */
        void shutdown();
    };
}    // namespace opendeck::io::outputs
