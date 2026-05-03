/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "database/database.h"
#include "protocol/midi/midi.h"
#include "io/common/common.h"
#include "system/config.h"
#include "io/base.h"
#include "threads.h"

#include "zlibs/utils/misc/mutex.h"

#include <zephyr/kernel.h>

#include <array>
#include <atomic>
#include <optional>

namespace opendeck::io::leds
{
    /**
     * @brief Manages LED state, blinking, and refresh scheduling.
     */
    class Leds : public io::Base
    {
        public:
        /**
         * @brief Constructs an LED controller bound to hardware and database services.
         *
         * @param hwa Hardware abstraction used to update LED outputs.
         * @param database Database interface used to read LED configuration.
         */
        Leds(Hwa&      hwa,
             Database& database);

        /**
         * @brief Stops the LED worker thread and releases runtime resources.
         */
        ~Leds() override;

        /**
         * @brief Initializes the LED subsystem and starts its worker thread.
         *
         * @return `true` if initialization completed successfully, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Stops the LED subsystem worker thread.
         */
        void deinit() override;

        /**
         * @brief Returns the number of refreshable logical LED outputs.
         */
        size_t refreshable_components() const override;

        /**
         * @brief Republishes the current output state for a staged subset of logical LED outputs.
         *
         * @param start_index First LED index to refresh.
         * @param count Number of LED indices to refresh.
         */
        void force_refresh(size_t start_index, size_t count) override;

        /**
         * @brief Returns the logical color configured for the specified LED.
         *
         * @param index LED index to query.
         *
         * @return Current logical LED color.
         */
        Color color(uint8_t index);

        /**
         * @brief Applies a logical color and brightness to the specified LED.
         *
         * @param index LED index to update.
         * @param color Logical color to apply.
         * @param brightness Brightness level to apply.
         */
        void set_color(uint8_t index, Color color, Brightness brightness);

        /**
         * @brief Returns the configured blink speed for the specified LED.
         *
         * @param index LED index to query.
         *
         * @return Blink speed assigned to the LED.
         */
        BlinkSpeed blink_speed(uint8_t index);

        /**
         * @brief Turns all LEDs off.
         */
        void set_all_off();

        private:
        /**
         * @brief Bit positions used to encode per-LED runtime state.
         */
        enum class LedBit : uint8_t
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
        static constexpr uint8_t LED_BLINK_TIMER_TYPE_CHECK_TIME            = 50;
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

        std::array<uint8_t, STORAGE_SIZE>    _led_state                         = {};
        std::array<Brightness, STORAGE_SIZE> _brightness                        = {};
        std::array<uint8_t, STORAGE_SIZE>    _blink_timer                       = {};
        BlinkType                            _led_blink_type                    = BlinkType::Timer;
        const uint8_t*                       _blink_reset_array_ptr             = nullptr;
        uint8_t                              _blink_counter[TOTAL_BLINK_SPEEDS] = {};
        bool                                 _blink_state[TOTAL_BLINK_SPEEDS]   = {};
        uint32_t                             _last_led_blink_update_time        = 0;

        threads::LedsThread _thread;
        std::atomic<bool>   _force_refresh_pending = { false };
        struct k_sem        _update_semaphore      = {};

        /**
         * @brief Turns every LED fully on.
         */
        void set_all_on();

        /**
         * @brief Forces every LED into a non-blinking on state.
         */
        void set_all_static_on();

        /**
         * @brief Queues an LED refresh request for the worker thread.
         *
         * @param force_refresh `true` to force a full output refresh, otherwise `false`.
         */
        void request_update(bool force_refresh);

        /**
         * @brief Waits for a queued LED refresh request.
         *
         * @return `true` if a queued update was received, otherwise `false`.
         */
        bool wait_for_update();

        /**
         * @brief Processes one queued or periodic LED update cycle.
         *
         * @param force_refresh `true` to force a full output refresh, otherwise `false`.
         */
        void process_update(bool force_refresh);

        /**
         * @brief Writes the current logical LED state to the hardware driver.
         */
        void refresh();

        /**
         * @brief Updates the blink speed assigned to one LED.
         *
         * @param index LED index to update.
         * @param state Blink speed to apply.
         * @param update_state `true` to update the active state bits immediately, otherwise `false`.
         */
        void set_blink_speed(uint8_t index, BlinkSpeed state, bool update_state = true);

        /**
         * @brief Selects the timing source used for LED blinking.
         *
         * @param blink_type Blink timing source to activate.
         */
        void set_blink_type(BlinkType blink_type);

        /**
         * @brief Clears all blink counters and synchronized blink states.
         */
        void reset_blinking();

        /**
         * @brief Updates one logical state bit for an LED.
         *
         * @param index LED index to update.
         * @param bit Bit field to modify.
         * @param state Bit value to store.
         */
        void update_bit(uint8_t index, LedBit bit, bool state);

        /**
         * @brief Returns one logical state bit for an LED.
         *
         * @param index LED index to query.
         * @param bit Bit field to read.
         *
         * @return `true` if the requested bit is set, otherwise `false`.
         */
        bool bit(uint8_t index, LedBit bit);

        /**
         * @brief Clears cached runtime state for one LED.
         *
         * @param index LED index to reset.
         */
        void reset_state(uint8_t index);

        /**
         * @brief Converts a stored numeric value into a logical LED color.
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
         * @brief Plays the startup LED animation.
         */
        void start_up_animation();

        /**
         * @brief Applies an incoming MIDI event to the LED state machine.
         *
         * @param message Decoded MIDI message to process.
         * @param direction MIDI traffic direction associated with the message.
         */
        void midi_to_state(const protocol::midi::Message& message, signaling::MidiDirection direction);

        /**
         * @brief Updates internal preset-indicator LEDs for the selected preset.
         *
         * @param preset Active preset index.
         */
        void internal_preset_to_state(uint8_t preset);

        /**
         * @brief Applies the final brightness state for one LED.
         *
         * @param index LED index to update.
         * @param brightness Brightness value to store.
         */
        void set_state(size_t index, Brightness brightness);

        /**
         * @brief Serves SysEx configuration reads for the LED block.
         *
         * @param section LED configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Leds section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the LED block.
         *
         * @param section LED configuration section being written.
         * @param index Parameter index within the section.
         * @param value Value to store.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::Leds section, size_t index, uint16_t value);

        /**
         * @brief Performs shutdown work for the controller.
         */
        void shutdown();
    };
}    // namespace opendeck::io::leds
