/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "io/common/common.h"
#include "database/database.h"
#include "io/analog/drivers/count.h"

#include <array>
#include <concepts>
#include <cstdint>

namespace io::analog
{
    /**
     * @brief Frame of raw analog samples, indexed by analog input.
     */
    using Frame = std::array<uint16_t, OPENDECK_ANALOG_PHYSICAL_COUNT>;

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
     * @brief ADC resolution identifiers used by analog backends.
     */
    inline constexpr uint8_t ADC_RESOLUTION_10_BIT = 10;
    inline constexpr uint8_t ADC_RESOLUTION_12_BIT = 12;

    /**
     * @brief Concept describing one compile-time ADC profile.
     */
    template<typename T>
    concept AdcConfigType = requires {
        { T::ADC_MIN_VALUE } -> std::convertible_to<uint16_t>;
        { T::ADC_MAX_VALUE } -> std::convertible_to<uint16_t>;
        { T::FSR_MIN_VALUE } -> std::convertible_to<uint16_t>;
        { T::FSR_MAX_VALUE } -> std::convertible_to<uint16_t>;
        { T::AFTERTOUCH_MAX_VALUE } -> std::convertible_to<uint16_t>;
        { T::DIGITAL_VALUE_THRESHOLD_ON } -> std::convertible_to<uint16_t>;
        { T::DIGITAL_VALUE_THRESHOLD_OFF } -> std::convertible_to<uint16_t>;
    };

    /**
     * @brief 10-bit ADC profile used by analog filter and drivers.
     */
    struct AdcConfig10Bit
    {
        static constexpr uint16_t ADC_MIN_VALUE               = 0;
        static constexpr uint16_t ADC_MAX_VALUE               = 1023;
        static constexpr uint16_t FSR_MIN_VALUE               = 40;
        static constexpr uint16_t FSR_MAX_VALUE               = 340;
        static constexpr uint16_t AFTERTOUCH_MAX_VALUE        = 600;
        static constexpr uint16_t DIGITAL_VALUE_THRESHOLD_ON  = 800;
        static constexpr uint16_t DIGITAL_VALUE_THRESHOLD_OFF = 200;
    };

    /**
     * @brief 12-bit ADC profile used by analog filter and drivers.
     */
    struct AdcConfig12Bit
    {
        static constexpr uint16_t ADC_MIN_VALUE               = 0;
        static constexpr uint16_t ADC_MAX_VALUE               = 4095;
        static constexpr uint16_t FSR_MIN_VALUE               = 160;
        static constexpr uint16_t FSR_MAX_VALUE               = 1360;
        static constexpr uint16_t AFTERTOUCH_MAX_VALUE        = 2400;
        static constexpr uint16_t DIGITAL_VALUE_THRESHOLD_ON  = 3000;
        static constexpr uint16_t DIGITAL_VALUE_THRESHOLD_OFF = 1000;
    };

    /**
     * @brief Compile-time mapping from ADC resolution to one supported ADC profile type.
     *
     * Supported resolutions are limited to 10-bit and 12-bit.
     *
     * @tparam Bits ADC resolution in bits.
     */
    template<uint8_t Bits>
    struct AdcConfigFor
    {
        static_assert(Bits == ADC_RESOLUTION_10_BIT || Bits == ADC_RESOLUTION_12_BIT,
                      "Only 10-bit and 12-bit ADC profiles are supported.");

        using Type = void;
    };

    template<>
    struct AdcConfigFor<ADC_RESOLUTION_10_BIT>
    {
        using Type = AdcConfig10Bit;
    };

    template<>
    struct AdcConfigFor<ADC_RESOLUTION_12_BIT>
    {
        using Type = AdcConfig12Bit;
    };

    template<uint8_t Bits>
    using AdcConfigForT = typename AdcConfigFor<Bits>::Type;
}    // namespace io::analog
