/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/outputs/deps.h"
#include "firmware/src/database/database.h"
#include "firmware/src/protocol/midi/midi.h"
#include "firmware/src/io/common/common.h"
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
     * @brief Manages OUTPUT state, blinking, and refresh scheduling.
     */
    class Outputs : public io::Base
    {
        public:
        /**
         * @brief Constructs an OUTPUT controller bound to hardware and database services.
         *
         * @param hwa Hardware abstraction used to update OUTPUT outputs.
         * @param database Database interface used to read OUTPUT configuration.
         */
        Outputs(Hwa&      hwa,
                Database& database);

        /**
         * @brief Stops the OUTPUT worker thread and releases runtime resources.
         */
        ~Outputs() override;

        /**
         * @brief Initializes the OUTPUT subsystem and starts its worker thread.
         *
         * @return `true` if initialization completed successfully, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Stops the OUTPUT subsystem worker thread.
         */
        void deinit() override;

        /**
         * @brief Returns the number of refreshable logical OUTPUT outputs.
         */
        size_t refreshable_components() const override;

        /**
         * @brief Republishes the current output state for a staged subset of logical OUTPUT outputs.
         *
         * @param start_index First OUTPUT index to refresh.
         * @param count Number of OUTPUT indices to refresh.
         */
        void force_refresh(size_t start_index, size_t count) override;

        /**
         * @brief Returns the logical color configured for the specified OUTPUT.
         *
         * @param index OUTPUT index to query.
         *
         * @return Current logical OUTPUT color.
         */
        Color color(uint8_t index);

        /**
         * @brief Applies a logical color and brightness to the specified OUTPUT.
         *
         * @param index OUTPUT index to update.
         * @param color Logical color to apply.
         * @param brightness Brightness level to apply.
         */
        void set_color(uint8_t index, Color color, Brightness brightness);

        /**
         * @brief Returns the configured blink speed for the specified OUTPUT.
         *
         * @param index OUTPUT index to query.
         *
         * @return Blink speed assigned to the OUTPUT.
         */
        BlinkSpeed blink_speed(uint8_t index);

        /**
         * @brief Turns all Outputs off.
         */
        void set_all_off();

        private:
        /**
         * @brief Bit positions used to encode per-OUTPUT runtime state.
         */
        enum class OutputBit : uint8_t
        {
            Active,
            BlinkOn,
            State,
            Rgb,
            RgbR,
            RgbG,
            RgbB
        };

        static constexpr size_t  TOTAL_BLINK_SPEEDS                         = 4;
        static constexpr size_t  TOTAL_BRIGHTNESS_VALUES                    = 4;
        static constexpr uint8_t OUTPUT_BLINK_TIMER_TYPE_CHECK_TIME         = 50;
        static constexpr size_t  STORAGE_SIZE                               = Collection::size() ? Collection::size() : 1;
        static constexpr uint8_t BLINK_RESET_MIDI_CLOCK[TOTAL_BLINK_SPEEDS] = {
            48,
            24,
            12,
            255,    // no blinking
        };

        static constexpr uint8_t BLINK_RESET_TIMER[TOTAL_BLINK_SPEEDS] = {
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

        std::array<uint8_t, STORAGE_SIZE>    _output_state                      = {};
        std::array<Brightness, STORAGE_SIZE> _brightness                        = {};
        std::array<uint8_t, STORAGE_SIZE>    _blink_timer                       = {};
        BlinkType                            _output_blink_type                 = BlinkType::Timer;
        const uint8_t*                       _blink_reset_array_ptr             = nullptr;
        uint8_t                              _blink_counter[TOTAL_BLINK_SPEEDS] = {};
        bool                                 _blink_state[TOTAL_BLINK_SPEEDS]   = {};
        uint32_t                             _last_output_blink_update_time     = 0;

        threads::OutputsThread _thread;
        std::atomic<bool>      _force_refresh_pending = { false };
        struct k_sem           _update_semaphore      = {};

        /**
         * @brief Turns every OUTPUT fully on.
         */
        void set_all_on();

        /**
         * @brief Forces every OUTPUT into a non-blinking on state.
         */
        void set_all_static_on();

        /**
         * @brief Queues an OUTPUT refresh request for the worker thread.
         *
         * @param force_refresh `true` to force a full output refresh, otherwise `false`.
         */
        void request_update(bool force_refresh);

        /**
         * @brief Waits for a queued OUTPUT refresh request.
         *
         * @return `true` if a queued update was received, otherwise `false`.
         */
        bool wait_for_update();

        /**
         * @brief Processes one queued or periodic OUTPUT update cycle.
         *
         * @param force_refresh `true` to force a full output refresh, otherwise `false`.
         */
        void process_update(bool force_refresh);

        /**
         * @brief Writes the current logical OUTPUT state to the hardware driver.
         */
        void refresh();

        /**
         * @brief Updates the blink speed assigned to one OUTPUT.
         *
         * @param index OUTPUT index to update.
         * @param state Blink speed to apply.
         * @param update_state `true` to update the active state bits immediately, otherwise `false`.
         */
        void set_blink_speed(uint8_t index, BlinkSpeed state, bool update_state = true);

        /**
         * @brief Selects the timing source used for OUTPUT blinking.
         *
         * @param blink_type Blink timing source to activate.
         */
        void set_blink_type(BlinkType blink_type);

        /**
         * @brief Clears all blink counters and synchronized blink states.
         */
        void reset_blinking();

        /**
         * @brief Updates one logical state bit for an OUTPUT.
         *
         * @param index OUTPUT index to update.
         * @param bit Bit field to modify.
         * @param state Bit value to store.
         */
        void update_bit(uint8_t index, OutputBit bit, bool state);

        /**
         * @brief Returns one logical state bit for an OUTPUT.
         *
         * @param index OUTPUT index to query.
         * @param bit Bit field to read.
         *
         * @return `true` if the requested bit is set, otherwise `false`.
         */
        bool bit(uint8_t index, OutputBit bit);

        /**
         * @brief Clears cached runtime state for one OUTPUT.
         *
         * @param index OUTPUT index to reset.
         */
        void reset_state(uint8_t index);

        /**
         * @brief Converts a stored numeric value into a logical OUTPUT color.
         *
         * @param value Stored color value.
         *
         * @return Logical color corresponding to the stored value.
         */
        Color value_to_color(uint8_t value);

        /**
         * @brief Converts a stored numeric value into a blink speed.
         *
         * @param value Stored blink-speed value.
         *
         * @return Blink speed corresponding to the stored value.
         */
        BlinkSpeed value_to_blink_speed(uint8_t value);

        /**
         * @brief Converts a stored numeric value into a brightness level.
         *
         * @param value Stored brightness value.
         *
         * @return Brightness level corresponding to the stored value.
         */
        Brightness value_to_brightness(uint8_t value);

        /**
         * @brief Plays the startup OUTPUT animation.
         */
        void start_up_animation();

        /**
         * @brief Applies an incoming MIDI event to the OUTPUT state machine.
         *
         * @param message Decoded MIDI message to process.
         * @param direction Traffic direction associated with the message.
         */
        void midi_to_state(const protocol::midi::Message& message, signaling::SignalDirection direction);

        /**
         * @brief Updates internal preset-indicator Outputs for the selected preset.
         *
         * @param preset Active preset index.
         */
        void internal_preset_to_state(uint8_t preset);

        /**
         * @brief Applies the final brightness state for one OUTPUT.
         *
         * @param index OUTPUT index to update.
         * @param brightness Brightness value to store.
         */
        void set_state(size_t index, Brightness brightness);

        /**
         * @brief Serves SysEx configuration reads for the OUTPUT block.
         *
         * @param section OUTPUT configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Outputs section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the OUTPUT block.
         *
         * @param section OUTPUT configuration section being written.
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
