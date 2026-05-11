/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "io/common/common.h"
#include "database/database.h"
#include "io/analog/drivers/count.h"

#include <array>
#include <cstdint>

namespace opendeck::io::analog
{
    /**
     * @brief Frame of raw analog samples, indexed by analog input.
     */
    using Frame = std::array<uint16_t, OPENDECK_ANALOG_PHYSICAL_COUNT>;

    /**
     * @brief Physical-channel scan mask used to skip disabled analog inputs.
     */
    using ScanMask = std::array<bool, OPENDECK_ANALOG_PHYSICAL_COUNT>;

    /**
     * @brief Flattened collection of analog-capable inputs.
     */
    class Collection : public io::common::BaseCollection<OPENDECK_ANALOG_LOGICAL_COUNT>
    {
        public:
        /**
         * @brief Prevents instantiation of this compile-time analog collection descriptor.
         */
        Collection() = delete;
    };

    /**
     * @brief Group indices used with `Collection`.
     */
    enum
    {
        GroupAnalogInputs,
    };

    /**
     * @brief Selects the action emitted by an analog input.
     */
    enum class Type : uint8_t
    {
        PotentiometerControlChange,
        PotentiometerNote,
        Fsr,
        Button,
        Nrpn7Bit,
        Nrpn14Bit,
        PitchBend,
        ControlChange14Bit,
        Reserved,
        Count
    };

    /**
     * @brief Selects how FSR pressure should be interpreted.
     */
    enum class PressureType : uint8_t
    {
        Velocity,
        Aftertouch
    };

    /**
     * @brief ADC profile used by the analog filter and drivers.
     */
    struct AdcConfig
    {
        static constexpr uint16_t ADC_MIN_VALUE               = 0;
        static constexpr uint16_t ADC_MAX_VALUE               = 4095;
        static constexpr uint16_t FSR_MIN_VALUE               = 160;
        static constexpr uint16_t FSR_MAX_VALUE               = 1360;
        static constexpr uint16_t AFTERTOUCH_MAX_VALUE        = 2400;
        static constexpr uint16_t DIGITAL_VALUE_THRESHOLD_ON  = 3000;
        static constexpr uint16_t DIGITAL_VALUE_THRESHOLD_OFF = 1000;
    };
}    // namespace opendeck::io::analog
