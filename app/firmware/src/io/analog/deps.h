/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"

#include "database/database.h"
#include "io/digital/buttons/common.h"

#include "zlibs/utils/midi/midi_common.h"

#include <optional>

namespace io::analog
{
    using Database = database::User<database::Config::Section::Analog>;

    /**
     * @brief Hardware abstraction interface used by the analog subsystem.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Initializes the analog hardware backend.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Deinitializes the analog hardware backend.
         */
        virtual void deinit() = 0;

        /**
         * @brief Returns the next sampled analog frame when available.
         *
         * @return Sampled frame, or `std::nullopt` when no frame is ready.
         */
        virtual std::optional<Frame> read() = 0;
    };

    /**
     * @brief Interface for analog input filters that transform raw readings into publishable values.
     */
    class Filter
    {
        public:
        /**
         * @brief Runtime description of an analog sample used by filter implementations.
         */
        struct Descriptor
        {
            Type     type         = Type::PotentiometerControlChange;
            uint16_t value        = 0;
            uint16_t lower_offset = 0;
            uint16_t upper_offset = 0;
            uint16_t max_value    = zlibs::utils::midi::MAX_VALUE_7BIT;
        };

        virtual ~Filter() = default;

        /**
         * @brief Filters one analog input reading in place.
         *
         * @param index Analog input index being processed.
         * @param descriptor Runtime descriptor containing the raw value and configuration.
         *
         * @return `true` when the reading should be processed further, otherwise `false`.
         */
        virtual bool is_filtered(size_t index, Descriptor& descriptor) = 0;

        /**
         * @brief Resets any filter state associated with one analog input.
         *
         * @param index Analog input index to reset.
         */
        virtual void reset(size_t index) = 0;
    };
}    // namespace io::analog
