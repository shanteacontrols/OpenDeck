/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/encoders/deps.h"
#include "firmware/src/io/base.h"
#include "firmware/src/io/digital/encoders/mapper.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/system/config.h"

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
                 Database& database,
                 Mapper&   mapper);

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
        static constexpr uint32_t ENCODERS_SPEED_TIMEOUT_MS = 140;

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

        Hwa&      _hwa;
        Filter&   _filter;
        Database& _database;
        Mapper&   _mapper;

        std::array<uint8_t, Collection::size()> _encoder_speed = {};

        /**
         * @brief Processes one sampled encoder reading and emits any resulting action.
         *
         * @param index Encoder index being processed.
         * @param pair_value Current A/B pin state packed into bits 0 and 1.
         * @param sample_time Timestamp of the sample in milliseconds.
         */
        void process_reading(size_t index, uint8_t pair_value, uint32_t sample_time);

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
