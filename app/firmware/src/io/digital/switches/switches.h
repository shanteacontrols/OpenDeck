/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/switches/deps.h"
#include "firmware/src/io/digital/switches/mapper.h"
#include "firmware/src/protocol/midi/midi.h"
#include "firmware/src/system/config.h"
#include "firmware/src/io/base.h"

#include <optional>

namespace opendeck::io::switches
{
    /**
     * @brief Samples switches, tracks latching state, and publishes the configured actions.
     */
    class Switches : public io::Base
    {
        public:
        /**
         * @brief Constructs a switch controller bound to hardware, filtering, and database services.
         *
         * @param hwa Hardware abstraction used to sample switch states.
         * @param filter Debounce filter used to validate state changes.
         * @param database Database interface used to read switch configuration.
         */
        Switches(Hwa&      hwa,
                 Filter&   filter,
                 Mapper&   mapper,
                 Database& database);

        ~Switches() override = default;

        /**
         * @brief Initializes switch hardware and clears cached runtime state.
         *
         * @return `true` if the hardware initialized successfully, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Deinitializes the switch controller.
         */
        void deinit() override;

        /**
         * @brief Returns the number of refreshable digital switch inputs.
         */
        size_t refreshable_components() const override;

        /**
         * @brief Republishes the current output state for a staged subset of digital switches.
         *
         * @param start_index First switch index to refresh.
         * @param count Number of switch indices to refresh.
         */
        void force_refresh(size_t start_index, size_t count) override;

        /**
         * @brief Samples digital switches and processes any filtered state changes.
         */
        void process_state_changes();

        /**
         * @brief Clears cached pressed and latching state for one switch.
         *
         * @param index Switch index to reset.
         */
        void reset(size_t index);

        private:
        using Result = Mapper::Result;

        Hwa&      _hwa;
        Filter&   _filter;
        Mapper&   _mapper;
        Database& _database;

        /**
         * @brief Reads the current physical state for a switch when it is not claimed by an encoder.
         *
         * @param index Switch index to read.
         *
         * @return Physical switch state, or `std::nullopt` when the index is unavailable for switch use.
         */
        std::optional<bool> state(size_t index);

        /**
         * @brief Processes one switch state change and emits any configured action.
         *
         * @param index Switch index being processed.
         * @param reading New sampled state.
         */
        void process_switch(size_t index, bool reading);

        /**
         * @brief Publishes the action configured for a switch event.
         *
         * @param ignore_freeze When `true`, allows refresh-triggered output while the subsystem is frozen.
         */
        void publish_result(const Result& result, bool ignore_freeze = false);

        /**
         * @brief Serves SysEx configuration reads for the switch block.
         *
         * @param section Switch configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Switch section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the switch block.
         *
         * @param section Switch configuration section being written.
         * @param index Parameter index within the section.
         * @param value Value to store.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::Switch section, size_t index, uint16_t value);
    };
}    // namespace opendeck::io::switches
