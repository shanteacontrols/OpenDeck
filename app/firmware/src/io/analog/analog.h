/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "mapper.h"
#include "signaling/signaling.h"
#include "system/config.h"
#include "threads.h"
#include "io/base.h"
#include "protocol/midi/midi.h"

#include "zlibs/utils/misc/mutex.h"

#include <optional>

namespace opendeck::io::analog
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
               Mapper&   mapper,
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
        Hwa&                      _hwa;
        Filter&                   _filter;
        Mapper&                   _mapper;
        Database&                 _database;
        zlibs::utils::misc::Mutex _hwa_mutex;
        zlibs::utils::misc::Mutex _state_mutex;
        threads::AnalogThread     _thread;

        /**
         * @brief Processes one sampled frame containing the current values for all analog inputs.
         *
         * @param frame Sampled analog frame to process.
         */
        void process_frame(const Frame& frame);

        /**
         * @brief Builds the runtime filter descriptor for one analog input.
         *
         * @param index Analog input index to describe.
         * @param filter_descriptor Output storage for the populated filter descriptor.
         */
        void fill_filter_descriptor(size_t index, Filter::Descriptor& filter_descriptor);

        /**
         * @brief Processes one analog reading and emits any resulting action.
         *
         * @param index Analog input index being processed.
         * @param value Raw sampled analog value.
         */
        void process_reading(size_t index, uint16_t value);

        /**
         * @brief Rebuilds the physical scan mask from enabled logical analog inputs.
         */
        void update_scan_mask();

        /**
         * @brief Resets cached runtime state for one analog input.
         *
         * @param index Analog input index to reset.
         */
        void reset(size_t index);

        /**
         * @brief Publishes one mapped analog result.
         *
         * @param result Mapped analog result to publish.
         * @param ignore_freeze When `true`, allows refresh-triggered output while the subsystem is frozen.
         */
        void publish_result(const Mapper::Result& result, bool ignore_freeze = false);

        /**
         * @brief Publishes one analog input configured as a digital switch.
         *
         * @param result Mapped analog result containing the switch-style value to publish.
         * @param ignore_freeze When `true`, allows refresh-triggered output while the subsystem is frozen.
         */
        void publish_switch(const Mapper::Result& result, bool ignore_freeze = false);

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
}    // namespace opendeck::io::analog
