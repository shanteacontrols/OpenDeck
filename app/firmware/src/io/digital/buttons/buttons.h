/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "mapper.h"
#include "protocol/midi/midi.h"
#include "system/config.h"
#include "io/base.h"

#include <optional>

namespace opendeck::io::buttons
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
                Mapper&   mapper,
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
        using Result = Mapper::Result;

        Hwa&      _hwa;
        Filter&   _filter;
        Mapper&   _mapper;
        Database& _database;

        /**
         * @brief Reads the current physical state for a button when it is not claimed by an encoder.
         *
         * @param index Button index to read.
         *
         * @return Physical button state, or `std::nullopt` when the index is unavailable for button use.
         */
        std::optional<bool> state(size_t index);

        /**
         * @brief Processes one button state change and emits any configured action.
         *
         * @param index Button index being processed.
         * @param reading New sampled state.
         */
        void process_button(size_t index, bool reading);

        /**
         * @brief Publishes the action configured for a button event.
         *
         * @param ignore_freeze When `true`, allows refresh-triggered output while the subsystem is frozen.
         */
        void publish_result(const Result& result, bool ignore_freeze = false);

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
}    // namespace opendeck::io::buttons
