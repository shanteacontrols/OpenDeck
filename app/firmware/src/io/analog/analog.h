/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "messaging/messaging.h"
#include "system/config.h"
#include "threads.h"
#include "io/base.h"
#include "protocol/midi/midi.h"

#include "zlibs/utils/misc/mutex.h"

#include <optional>

namespace io::analog
{
    /**
     * @brief Processes analog input frames, applies filtering, and publishes the configured actions.
     */
    class Analog : public io::Base
    {
        public:
        /**
         * @brief Constructs an analog controller bound to hardware, filtering, and database services.
         *
         * @param hwa Hardware abstraction used to read analog frames.
         * @param filter Filter used to smooth and validate analog readings.
         * @param database Database interface used to read analog configuration.
         */
        Analog(Hwa&      hwa,
               Filter&   filter,
               Database& database);

        /**
         * @brief Shuts the controller down.
         */
        ~Analog() override;

        /**
         * @brief Initializes the analog subsystem and starts its worker thread.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Stops the analog subsystem worker thread.
         */
        void deinit() override;

        /**
         * @brief Returns the number of refreshable analog inputs.
         */
        size_t refreshable_components() const override;

        /**
         * @brief Republishes the current output state for a staged subset of enabled analog inputs.
         *
         * @param start_index First analog input index to refresh.
         * @param count Number of analog input indices to refresh.
         */
        void force_refresh(size_t start_index, size_t count) override;

        private:
        /**
         * @brief Runtime description of an analog input action resolved from the database.
         */
        struct Descriptor
        {
            Type                  type         = Type::PotentiometerControlChange;
            bool                  inverted     = false;
            uint16_t              lower_limit  = 0;
            uint16_t              upper_limit  = 0;
            uint8_t               lower_offset = 0;
            uint8_t               upper_offset = 0;
            uint16_t              max_value    = zlibs::utils::midi::MAX_VALUE_7BIT;
            uint16_t              new_value    = 0;
            uint16_t              old_value    = 0;
            messaging::MidiSignal signal       = {};
        };

        static constexpr protocol::midi::MessageType INTERNAL_MSG_TO_MIDI_TYPE[static_cast<uint8_t>(Type::Count)] = {
            protocol::midi::MessageType::ControlChange,         // PotentiometerControlChange
            protocol::midi::MessageType::NoteOn,                // PotentiometerNote
            protocol::midi::MessageType::NoteOn,                // Fsr (set to note off when appropriate)
            protocol::midi::MessageType::Invalid,               // Button (let other listeners handle this)
            protocol::midi::MessageType::Nrpn7Bit,              // Nrpn7Bit
            protocol::midi::MessageType::Nrpn14Bit,             // Nrpn14Bit
            protocol::midi::MessageType::PitchBend,             // PitchBend
            protocol::midi::MessageType::ControlChange14Bit,    // ControlChange14Bit
            protocol::midi::MessageType::Invalid,               // Reserved
        };

        static constexpr size_t STATE_STORAGE_DIVISOR = 8;

        Hwa&                      _hwa;
        Filter&                   _filter;
        Database&                 _database;
        zlibs::utils::misc::Mutex _state_mutex;
        uint8_t                   _fsr_pressed[Collection::size() / STATE_STORAGE_DIVISOR + 1] = {};
        uint16_t                  _last_value[Collection::size()]                              = {};
        threads::AnalogThread     _thread;

        /**
         * @brief Processes one sampled frame containing the current values for all analog inputs.
         *
         * @param frame Sampled analog frame to process.
         */
        void process_frame(const Frame& frame);

        /**
         * @brief Builds the runtime descriptor for one analog input.
         *
         * @param index Analog input index to describe.
         * @param descriptor Output storage for the populated descriptor.
         */
        void fill_descriptor(size_t index, Descriptor& descriptor);

        /**
         * @brief Processes one analog reading and emits any resulting action.
         *
         * @param index Analog input index being processed.
         * @param value Raw sampled analog value.
         */
        void process_reading(size_t index, uint16_t value);

        /**
         * @brief Resets cached runtime state for one analog input.
         *
         * @param index Analog input index to reset.
         */
        void reset(size_t index);

        /**
         * @brief Processes a potentiometer-style reading.
         *
         * @param index Analog input index being processed.
         * @param descriptor Runtime descriptor describing the configured action.
         *
         * @return `true` when a new value should be published, otherwise `false`.
         */
        bool check_potentiometer_value(size_t index, Descriptor& descriptor);

        /**
         * @brief Processes an FSR-style reading.
         *
         * @param index Analog input index being processed.
         * @param descriptor Runtime descriptor describing the configured action.
         *
         * @return `true` when a new value should be published, otherwise `false`.
         */
        bool check_fsr_value(size_t index, Descriptor& descriptor);

        /**
         * @brief Publishes the action configured for an analog input.
         *
         * @param index Analog input index being processed.
         * @param descriptor Runtime descriptor describing the configured action.
         * @param ignore_freeze When `true`, allows refresh-triggered output while the subsystem is frozen.
         */
        void send_message(size_t index, Descriptor& descriptor, bool ignore_freeze = false);

        /**
         * @brief Stores the pressed state for an analog input configured as an FSR/button.
         *
         * @param index Analog input index to update.
         * @param state Pressed state to store.
         */
        void set_fsr_state(size_t index, bool state);

        /**
         * @brief Returns the cached pressed state for an analog input configured as an FSR/button.
         *
         * @param index Analog input index to query.
         *
         * @return `true` if the cached state is pressed, otherwise `false`.
         */
        bool fsr_state(size_t index);

        /**
         * @brief Serves SysEx configuration reads for the analog block.
         *
         * @param section Analog configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Analog section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the analog block.
         *
         * @param section Analog configuration section being written.
         * @param index Parameter index within the section.
         * @param value Value to store.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::Analog section, size_t index, uint16_t value);
    };
}    // namespace io::analog
