/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"

#include "firmware/src/io/i2c/hwa/test/hwa_test.h"
#include "firmware/src/io/i2c/peripherals/sensor_bno085/sensor_bno085.h"

#include <vector>

using namespace opendeck::io::i2c;
using namespace opendeck::io::i2c::sensor_bno085;

namespace
{
    class Bno085SensorTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            _hwa.read_data = { 0x05, 0x00, 0x01, 0x00 };
        }

        HwaTest      _hwa;
        SensorBno085 _sensor = SensorBno085(_hwa);
    };
}    // namespace

TEST_F(Bno085SensorTest, ReportsSupportedI2cAddresses)
{
    const auto addresses = _sensor.i2c_addresses();

    ASSERT_EQ(addresses.size(), I2C_ADDRESSES.size());
    EXPECT_EQ(addresses[0], 0x4A);
    EXPECT_EQ(addresses[1], 0x4B);
}

TEST_F(Bno085SensorTest, RejectsInvalidAddressIndex)
{
    EXPECT_FALSE(_sensor.init(I2C_ADDRESSES.size()));
    EXPECT_FALSE(_sensor.update());
}

TEST_F(Bno085SensorTest, InitializesSelectedAddress)
{
    EXPECT_TRUE(_sensor.init(1));
    EXPECT_TRUE(_sensor.update());

    ASSERT_EQ(_hwa.write_addresses.size(), 1U + REPORT_COUNT);
    ASSERT_EQ(_hwa.read_addresses.size(), 2U);
    EXPECT_EQ(_hwa.write_addresses[0], I2C_ADDRESSES[1]);
    EXPECT_EQ(_hwa.read_addresses[0], I2C_ADDRESSES[1]);
    EXPECT_EQ(_hwa.read_addresses[1], I2C_ADDRESSES[1]);
    ASSERT_EQ(_hwa.last_write.size(), SET_FEATURE_COMMAND_SIZE);
    EXPECT_EQ(_hwa.last_write[0], SET_FEATURE_COMMAND_SIZE);
    EXPECT_EQ(_hwa.last_write[2], SHTP_CONTROL_CHANNEL);
    EXPECT_EQ(_hwa.last_write[4], SHTP_SET_FEATURE_COMMAND);
}

TEST_F(Bno085SensorTest, DeinitStopsUpdates)
{
    ASSERT_TRUE(_sensor.init(0));
    EXPECT_TRUE(_sensor.deinit());
    EXPECT_FALSE(_sensor.update());
}

TEST_F(Bno085SensorTest, RejectsInvalidStartupPacket)
{
    _hwa.read_data = { 0x03, 0x00, 0x01, 0x00 };

    EXPECT_FALSE(_sensor.init(0));
    EXPECT_FALSE(_sensor.update());
}
