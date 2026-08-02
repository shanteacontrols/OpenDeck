/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS

#include "firmware/src/io/digital/encoders/filter/hw/filter_hw.h"

using namespace opendeck::firmware::io::encoders;

namespace
{
    class EncoderFilterHwTest : public ::testing::Test
    {
        protected:
        bool sample(uint8_t pair_state, std::optional<uint32_t> movement_elapsed_time)
        {
            _position = Position::Stopped;
            return _filter.is_filtered(0, pair_state, _position, movement_elapsed_time);
        }

        bool rotate_clockwise(std::optional<uint32_t> movement_elapsed_time)
        {
            static constexpr std::array<uint8_t, Filter::PULSES_PER_STEP> states = {
                0b10,
                0b11,
                0b01,
                0b00,
            };

            bool movement = false;

            for (size_t i = 0; i < states.size(); i++)
            {
                movement = sample(states[i], movement_elapsed_time);
            }

            return movement;
        }

        FilterHw _filter   = {};
        Position _position = Position::Stopped;
    };
}    // namespace

TEST_F(EncoderFilterHwTest, DecodesFirstMovementWithoutPreviousMovement)
{
    EXPECT_FALSE(sample(0b00, std::nullopt));

    EXPECT_TRUE(rotate_clockwise(std::nullopt));
    EXPECT_EQ(Position::Cw, _position);
}

TEST_F(EncoderFilterHwTest, DecodesMovementAfterIdleTimeout)
{
    EXPECT_FALSE(sample(0b00, std::nullopt));
    ASSERT_TRUE(rotate_clockwise(std::nullopt));
    ASSERT_EQ(Position::Cw, _position);

    EXPECT_TRUE(rotate_clockwise(100));
    EXPECT_EQ(Position::Cw, _position);
}

#else

TEST(EncoderFilterHwTest, SkippedWhenPresetDoesNotSupportEncoders)
{
    SUCCEED();
}

#endif
