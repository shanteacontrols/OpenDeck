/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ADC

#include "io/analog/filter_hw.h"

#include "zlibs/utils/misc/numeric.h"

#include <initializer_list>
#include <optional>
#include <utility>
#include <vector>

using namespace opendeck::io;

namespace
{
    constexpr uint8_t ActiveAdcBits = analog::ADC_RESOLUTION_12_BIT;
    using ActiveAdcConfig           = analog::AdcConfigForT<ActiveAdcBits>;

    class AnalogFilterTest : public ::testing::Test
    {
        protected:
        static constexpr size_t   FILTER_INDEX          = 0;
        static constexpr uint16_t ADC_MIN_VALUE         = ActiveAdcConfig::ADC_MIN_VALUE;
        static constexpr uint16_t ADC_MAX_VALUE         = ActiveAdcConfig::ADC_MAX_VALUE;
        static constexpr uint16_t MID_VALUE             = (ADC_MIN_VALUE + ADC_MAX_VALUE) / 2;
        static constexpr uint16_t BUTTON_OFF_VALUE      = ActiveAdcConfig::DIGITAL_VALUE_THRESHOLD_OFF / 2;
        static constexpr uint16_t BUTTON_ON_VALUE       = (ActiveAdcConfig::DIGITAL_VALUE_THRESHOLD_ON + ActiveAdcConfig::ADC_MAX_VALUE) / 2;
        static constexpr uint16_t BUTTON_MID_VALUE      = (ActiveAdcConfig::DIGITAL_VALUE_THRESHOLD_OFF + ActiveAdcConfig::DIGITAL_VALUE_THRESHOLD_ON) / 2;
        static constexpr uint16_t FSR_MIN_VALUE         = ActiveAdcConfig::FSR_MIN_VALUE;
        static constexpr uint16_t FSR_MAX_VALUE         = ActiveAdcConfig::FSR_MAX_VALUE;
        static constexpr uint16_t MIDI_MAX_VALUE        = 127;
        static constexpr uint16_t MIDI_MAX_VALUE_14_BIT = 16383;
        static constexpr uint16_t DEFAULT_MIDI_OUT      = MIDI_MAX_VALUE / 2;
        static constexpr uint8_t  PERCENTAGE_DIVISOR    = 100;

        analog::Filter::Descriptor descriptor(uint16_t value,
                                              uint16_t max_value    = MIDI_MAX_VALUE,
                                              uint8_t  lower_offset = 0,
                                              uint8_t  upper_offset = 0) const
        {
            analog::Filter::Descriptor descriptor = {};
            descriptor.type                       = analog::Type::PotentiometerControlChange;
            descriptor.value                      = value;
            descriptor.lower_offset               = lower_offset;
            descriptor.upper_offset               = upper_offset;
            descriptor.max_value                  = max_value;
            return descriptor;
        }

        analog::Filter::Descriptor button_descriptor(uint16_t value) const
        {
            analog::Filter::Descriptor descriptor = {};
            descriptor.type                       = analog::Type::Button;
            descriptor.value                      = value;
            descriptor.lower_offset               = 0;
            descriptor.upper_offset               = 0;
            descriptor.max_value                  = 1;
            return descriptor;
        }

        analog::Filter::Descriptor fsr_descriptor(uint16_t value,
                                                  uint16_t max_value = MIDI_MAX_VALUE) const
        {
            analog::Filter::Descriptor descriptor = {};
            descriptor.type                       = analog::Type::Fsr;
            descriptor.value                      = value;
            descriptor.lower_offset               = 0;
            descriptor.upper_offset               = 0;
            descriptor.max_value                  = max_value;
            return descriptor;
        }

        uint16_t configured_lower_offset_raw(uint8_t percentage) const
        {
            if (percentage != 0)
            {
                const auto adc_span = static_cast<uint32_t>(ADC_MAX_VALUE) - static_cast<uint32_t>(ADC_MIN_VALUE);

                return static_cast<uint16_t>(static_cast<uint32_t>(ADC_MIN_VALUE) +
                                             ((adc_span * percentage) / PERCENTAGE_DIVISOR));
            }

            return ADC_MIN_VALUE;
        }

        uint16_t configured_upper_offset_raw(uint8_t percentage) const
        {
            if (percentage != 0)
            {
                const auto adc_span = static_cast<uint32_t>(ADC_MAX_VALUE) - static_cast<uint32_t>(ADC_MIN_VALUE);

                return static_cast<uint16_t>(static_cast<uint32_t>(ADC_MAX_VALUE) -
                                             ((adc_span * percentage) / PERCENTAGE_DIVISOR));
            }

            return ADC_MAX_VALUE;
        }

        uint16_t adc_value_for_output(uint16_t output_value,
                                      uint16_t max_value    = MIDI_MAX_VALUE,
                                      uint8_t  lower_offset = 0,
                                      uint8_t  upper_offset = 0) const
        {
            std::optional<uint16_t> first   = {};
            uint16_t                last    = 0;
            const auto              adc_min = configured_lower_offset_raw(lower_offset);
            const auto              adc_max = configured_upper_offset_raw(upper_offset);

            for (uint16_t adc = adc_min; adc <= adc_max; adc++)
            {
                const auto mapped = zlibs::utils::misc::map_range(static_cast<uint32_t>(adc),
                                                                  static_cast<uint32_t>(adc_min),
                                                                  static_cast<uint32_t>(adc_max),
                                                                  static_cast<uint32_t>(0),
                                                                  static_cast<uint32_t>(max_value));

                if (mapped == output_value)
                {
                    if (!first.has_value())
                    {
                        first = adc;
                    }

                    last = adc;
                }
                else if (first.has_value())
                {
                    break;
                }
            }

            EXPECT_TRUE(first.has_value());
            return first.has_value() ? static_cast<uint16_t>((first.value() + last) / 2) : adc_min;
        }

        uint16_t adc_value_for_fsr_output(uint16_t output_value,
                                          uint16_t max_value = MIDI_MAX_VALUE) const
        {
            std::optional<uint16_t> first = {};
            uint16_t                last  = 0;

            for (uint16_t adc = FSR_MIN_VALUE; adc <= FSR_MAX_VALUE; adc++)
            {
                const auto mapped = zlibs::utils::misc::map_range(static_cast<uint32_t>(adc),
                                                                  static_cast<uint32_t>(FSR_MIN_VALUE),
                                                                  static_cast<uint32_t>(FSR_MAX_VALUE),
                                                                  static_cast<uint32_t>(0),
                                                                  static_cast<uint32_t>(max_value));

                if (mapped == output_value)
                {
                    if (!first.has_value())
                    {
                        first = adc;
                    }

                    last = adc;
                }
                else if (first.has_value())
                {
                    break;
                }
            }

            EXPECT_TRUE(first.has_value());
            return first.has_value() ? static_cast<uint16_t>((first.value() + last) / 2) : FSR_MIN_VALUE;
        }

        std::optional<uint16_t> sample(uint16_t value,
                                       uint16_t max_value    = MIDI_MAX_VALUE,
                                       uint8_t  lower_offset = 0,
                                       uint8_t  upper_offset = 0)
        {
            auto filtered = descriptor(value, max_value, lower_offset, upper_offset);

            if (_filter.is_filtered(FILTER_INDEX, filtered))
            {
                return filtered.value;
            }

            return {};
        }

        std::optional<uint16_t> sample_button(uint16_t value)
        {
            auto filtered = button_descriptor(value);

            if (_filter.is_filtered(FILTER_INDEX, filtered))
            {
                return filtered.value;
            }

            return {};
        }

        std::optional<uint16_t> sample_fsr(uint16_t value,
                                           uint16_t max_value = MIDI_MAX_VALUE)
        {
            auto filtered = fsr_descriptor(value, max_value);

            if (_filter.is_filtered(FILTER_INDEX, filtered))
            {
                return filtered.value;
            }

            return {};
        }

        std::vector<uint16_t> sample_sequence(std::initializer_list<uint16_t> values)
        {
            std::vector<uint16_t> outputs;

            for (const auto value : values)
            {
                auto filtered = sample(value);

                if (filtered.has_value())
                {
                    outputs.push_back(filtered.value());
                }
            }

            return outputs;
        }

        std::vector<uint16_t> sample_midi_sequence(std::initializer_list<uint16_t> values)
        {
            std::vector<uint16_t> outputs;

            for (const auto value : values)
            {
                auto filtered = sample(adc_value_for_output(value));

                if (filtered.has_value())
                {
                    outputs.push_back(filtered.value());
                }
            }

            return outputs;
        }

        std::vector<uint16_t> sample_timed_midi_sequence(std::initializer_list<std::pair<uint32_t, uint16_t>> values)
        {
            std::vector<uint16_t> outputs;

            for (const auto& [delay_ms, value] : values)
            {
                if (delay_ms != 0)
                {
                    k_msleep(delay_ms);
                }

                auto filtered = sample(adc_value_for_output(value));

                if (filtered.has_value())
                {
                    outputs.push_back(filtered.value());
                }
            }

            return outputs;
        }

        std::vector<uint16_t> sample_midi_repeated(uint16_t midi_value,
                                                   size_t   count)
        {
            std::vector<uint16_t> outputs;
            const auto            adc_value = adc_value_for_output(midi_value);

            for (size_t i = 0; i < count; i++)
            {
                auto filtered = sample(adc_value);

                if (filtered.has_value())
                {
                    outputs.push_back(filtered.value());
                }
            }

            return outputs;
        }

        std::vector<uint16_t> sample_midi_for(uint16_t midi_value,
                                              uint32_t duration_ms,
                                              uint32_t step_ms = 1)
        {
            std::vector<uint16_t> outputs;
            const auto            adc_value = adc_value_for_output(midi_value);

            for (uint32_t elapsed = 0; elapsed < duration_ms; elapsed += step_ms)
            {
                if (step_ms != 0)
                {
                    k_msleep(step_ms);
                }

                auto filtered = sample(adc_value);

                if (filtered.has_value())
                {
                    outputs.push_back(filtered.value());
                }
            }

            return outputs;
        }

        uint16_t prime(uint16_t value        = MID_VALUE,
                       uint16_t max_value    = MIDI_MAX_VALUE,
                       uint8_t  lower_offset = 0,
                       uint8_t  upper_offset = 0)
        {
            std::vector<uint16_t> outputs;

            for (size_t i = 0; i < 5; i++)
            {
                auto filtered = sample(value, max_value, lower_offset, upper_offset);

                if (filtered.has_value())
                {
                    outputs.push_back(filtered.value());
                }
            }

            EXPECT_FALSE(outputs.empty());

            return outputs.empty() ? 0 : outputs.back();
        }

        uint16_t settle_to_raw(uint16_t value,
                               uint16_t max_value    = MIDI_MAX_VALUE,
                               uint8_t  lower_offset = 0,
                               uint8_t  upper_offset = 0)
        {
            std::optional<uint16_t> last_output = {};
            size_t                  quiet_count = 0;

            for (size_t i = 0; i < 128; i++)
            {
                auto filtered = sample(value, max_value, lower_offset, upper_offset);

                if (filtered.has_value())
                {
                    last_output = filtered;
                    quiet_count = 0;
                }
                else if (last_output.has_value())
                {
                    quiet_count++;

                    if (quiet_count >= 8)
                    {
                        break;
                    }
                }
            }

            EXPECT_TRUE(last_output.has_value());

            return last_output.value_or(0);
        }

        uint16_t prime_midi(uint16_t value     = DEFAULT_MIDI_OUT,
                            uint16_t max_value = MIDI_MAX_VALUE)
        {
            std::optional<uint16_t> last_output = {};
            size_t                  quiet_count = 0;

            for (size_t i = 0; i < 128; i++)
            {
                auto filtered = sample(adc_value_for_output(value, max_value), max_value);

                if (filtered.has_value())
                {
                    last_output = filtered;
                    quiet_count = 0;
                }
                else if (last_output.has_value())
                {
                    quiet_count++;

                    if (quiet_count >= 8)
                    {
                        break;
                    }
                }
            }

            EXPECT_TRUE(last_output.has_value());

            return last_output.value_or(0);
        }

        uint16_t prime_fsr_midi(uint16_t value     = DEFAULT_MIDI_OUT,
                                uint16_t max_value = MIDI_MAX_VALUE)
        {
            std::optional<uint16_t> last_output = {};
            size_t                  quiet_count = 0;

            for (size_t i = 0; i < 128; i++)
            {
                auto filtered = sample_fsr(adc_value_for_fsr_output(value, max_value), max_value);

                if (filtered.has_value())
                {
                    last_output = filtered;
                    quiet_count = 0;
                }
                else if (last_output.has_value())
                {
                    quiet_count++;

                    if (quiet_count >= 8)
                    {
                        break;
                    }
                }
            }

            EXPECT_TRUE(last_output.has_value());

            return last_output.value_or(0);
        }

        uint16_t settle_to_midi(uint16_t value,
                                uint32_t duration_ms = analog::FilterHw<ActiveAdcBits>::motion_context_timeout_ms() + 20)
        {
            const auto outputs = sample_midi_for(value, duration_ms);

            EXPECT_FALSE(outputs.empty());

            return outputs.empty() ? 0 : outputs.back();
        }

        void reset_filter()
        {
            _filter.reset(FILTER_INDEX);
        }

        analog::FilterHw<ActiveAdcBits> _filter = {};
    };
}    // namespace

TEST_F(AnalogFilterTest, StableInputPublishesOnlyOnce)
{
    const auto outputs = sample_sequence({ MID_VALUE, MID_VALUE, MID_VALUE, MID_VALUE, MID_VALUE, MID_VALUE });

    ASSERT_EQ(1, outputs.size());
}

TEST_F(AnalogFilterTest, SubDeadbandSamplesAreSuppressed)
{
    (void)prime();

    EXPECT_FALSE(sample(MID_VALUE + 20).has_value());
    EXPECT_TRUE(sample(MID_VALUE + 40).has_value());
}

TEST_F(AnalogFilterTest, LowerOffsetClampsToConfiguredLowerBound)
{
    static constexpr uint8_t LOWER_OFFSET = 10;

    const auto configured_lower = configured_lower_offset_raw(LOWER_OFFSET);
    const auto probe            = configured_lower > 32 ? static_cast<uint16_t>(configured_lower - 32U) : ADC_MIN_VALUE;

    ASSERT_LT(probe, configured_lower);

    const auto filtered = settle_to_raw(probe, MIDI_MAX_VALUE, LOWER_OFFSET);

    reset_filter();

    const auto expected_clamped = settle_to_raw(configured_lower, MIDI_MAX_VALUE, LOWER_OFFSET);

    EXPECT_EQ(expected_clamped, filtered);
}

TEST_F(AnalogFilterTest, UpperOffsetClampsToConfiguredUpperBound)
{
    static constexpr uint8_t UPPER_OFFSET = 10;

    const auto configured_upper = configured_upper_offset_raw(UPPER_OFFSET);
    const auto probe            = static_cast<uint16_t>(configured_upper + 32U);

    ASSERT_GT(probe, configured_upper);

    const auto filtered = settle_to_raw(probe, MIDI_MAX_VALUE, 0, UPPER_OFFSET);

    reset_filter();

    const auto expected_clamped = settle_to_raw(configured_upper, MIDI_MAX_VALUE, 0, UPPER_OFFSET);

    EXPECT_EQ(expected_clamped, filtered);
}

TEST_F(AnalogFilterTest, ButtonPathUsesHysteresisAndEmitsOnTransitions)
{
    EXPECT_FALSE(sample_button(BUTTON_OFF_VALUE).has_value());
    auto event = sample_button(BUTTON_ON_VALUE);
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(1, event.value());

    EXPECT_FALSE(sample_button(BUTTON_ON_VALUE).has_value());
    EXPECT_FALSE(sample_button(BUTTON_MID_VALUE).has_value());

    event = sample_button(BUTTON_OFF_VALUE);
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(0, event.value());

    EXPECT_FALSE(sample_button(BUTTON_OFF_VALUE).has_value());
    EXPECT_FALSE(sample_button(BUTTON_MID_VALUE).has_value());
}

TEST_F(AnalogFilterTest, ButtonPathPublishesPressedStateOnStartup)
{
    auto event = sample_button(BUTTON_ON_VALUE);
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(1, event.value());

    EXPECT_FALSE(sample_button(BUTTON_ON_VALUE).has_value());
}

TEST_F(AnalogFilterTest, IdlePotDriftRequiresRepeatConfirmation)
{
    const auto baseline = prime_midi(64);

    k_msleep(analog::FilterHw<ActiveAdcBits>::motion_context_timeout_ms() + 20);

    EXPECT_FALSE(sample(adc_value_for_output(static_cast<uint16_t>(baseline + 6))).has_value());

    const auto filtered = sample(adc_value_for_output(static_cast<uint16_t>(baseline + 6)));

    ASSERT_TRUE(filtered.has_value());
    EXPECT_GT(filtered.value(), baseline);
}

TEST_F(AnalogFilterTest, FastModeRequiresRepeatedLargeDeltasToEnter)
{
    (void)prime();
    EXPECT_FALSE(_filter.fast_mode_active(FILTER_INDEX));

    (void)sample(MID_VALUE + 70);
    EXPECT_FALSE(_filter.fast_mode_active(FILTER_INDEX));

    (void)sample(MID_VALUE + 140);
    EXPECT_TRUE(_filter.fast_mode_active(FILTER_INDEX));
}

TEST_F(AnalogFilterTest, FastModeThresholdScalesWithOutputResolution)
{
    (void)prime(MID_VALUE, MIDI_MAX_VALUE);
    EXPECT_FALSE(_filter.fast_mode_active(FILTER_INDEX));

    (void)sample(MID_VALUE + 20, MIDI_MAX_VALUE);
    (void)sample(MID_VALUE + 40, MIDI_MAX_VALUE);
    EXPECT_FALSE(_filter.fast_mode_active(FILTER_INDEX));

    reset_filter();

    (void)prime(MID_VALUE, MIDI_MAX_VALUE_14_BIT);
    EXPECT_FALSE(_filter.fast_mode_active(FILTER_INDEX));

    (void)sample(MID_VALUE + 20, MIDI_MAX_VALUE_14_BIT);
    (void)sample(MID_VALUE + 40, MIDI_MAX_VALUE_14_BIT);
    EXPECT_TRUE(_filter.fast_mode_active(FILTER_INDEX));
}

TEST_F(AnalogFilterTest, FastModeExpiresWhileContinuousIdleSamplesArrive)
{
    const auto baseline = prime_midi();

    (void)sample_sequence({ static_cast<uint16_t>(MID_VALUE + 140),
                            static_cast<uint16_t>(MID_VALUE + 210) });
    ASSERT_TRUE(_filter.fast_mode_active(FILTER_INDEX));

    (void)sample_midi_for(baseline,
                          analog::FilterHw<ActiveAdcBits>::motion_context_timeout_ms() + 20);

    EXPECT_FALSE(_filter.fast_mode_active(FILTER_INDEX));
    EXPECT_FALSE(sample(MID_VALUE + 280).has_value());
    EXPECT_FALSE(_filter.fast_mode_active(FILTER_INDEX));
}

TEST_F(AnalogFilterTest, TimedIdleDriftTraceNeedsRepeatedIdleSamples)
{
    (void)prime_midi();

    const auto outputs = sample_timed_midi_sequence({
        { 89, 77 },
        { 127, 78 },
        { 2244, 79 },
        { 25, 76 },
        { 1217, 75 },
        { 39, 74 },
        { 349, 75 },
        { 551, 74 },
        { 170, 77 },
        { 128, 78 },
    });

    EXPECT_FALSE(outputs.empty()) << testing::PrintToString(outputs);
    EXPECT_EQ(std::vector<uint16_t>({ 70, 74, 75 }), outputs);
}

TEST_F(AnalogFilterTest, ActivePotMovementPublishesWithoutIdleRepeat)
{
    const auto baseline = prime_midi(64);

    const auto filtered = sample(adc_value_for_output(static_cast<uint16_t>(baseline + 6)));

    ASSERT_TRUE(filtered.has_value());
    EXPECT_GT(filtered.value(), baseline);
}

TEST_F(AnalogFilterTest, FsrIdleDriftRequiresRepeatConfirmation)
{
    const auto baseline = prime_fsr_midi(64);

    k_msleep(analog::FilterHw<ActiveAdcBits>::motion_context_timeout_ms() + 20);

    EXPECT_FALSE(sample_fsr(adc_value_for_fsr_output(static_cast<uint16_t>(baseline + 6))).has_value());

    const auto filtered = sample_fsr(adc_value_for_fsr_output(static_cast<uint16_t>(baseline + 6)));

    ASSERT_TRUE(filtered.has_value());
    EXPECT_GT(filtered.value(), baseline);
}

TEST_F(AnalogFilterTest, ActiveFsrMovementPublishesWithoutIdleRepeat)
{
    const auto baseline = prime_fsr_midi(64);

    const auto filtered = sample_fsr(adc_value_for_fsr_output(static_cast<uint16_t>(baseline + 6)));

    ASSERT_TRUE(filtered.has_value());
    EXPECT_GT(filtered.value(), baseline);
}

#endif
