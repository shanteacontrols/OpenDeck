/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/database.h"

#include "firmware/src/database/builder/builder.h"
#include "firmware/src/io/i2c/hwa/test/hwa_test.h"
#include "firmware/src/io/i2c/peripherals/sensor_vl53l4cx/instance/impl/sensor_vl53l4cx.h"
#include "firmware/src/util/configurable/configurable.h"

#include <algorithm>

using namespace opendeck;
using namespace opendeck::firmware;
using namespace opendeck::firmware::io::i2c;
using namespace opendeck::firmware::io::i2c::sensor_vl53l4cx;

namespace
{
    class Vl53l4cxSensorTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            _hwa.set_available_addresses({ I2C_ADDRESSES[0] });
            _hwa.write_succeeds = false;

            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_database_admin.factory_reset());
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            signaling::clear_registry();
        }

        tests::NoOpDatabaseHandlers     _handlers;
        database::Builder               _database_builder;
        database::Admin&                _database_admin = _database_builder.instance();
        HwaTest                         _hwa;
        sensor_vl53l4cx::Database       _database = sensor_vl53l4cx::Database(_database_admin);
        sensor_vl53l4cx::SensorVl53l4cx _sensor   = sensor_vl53l4cx::SensorVl53l4cx(_hwa, _database);
    };
}    // namespace

TEST_F(Vl53l4cxSensorTest, ReportsSupportedI2cAddresses)
{
    const auto addresses = _sensor.i2c_addresses();

    ASSERT_EQ(addresses.size(), I2C_ADDRESSES.size());
    EXPECT_EQ(addresses[0], 0x29);
}

TEST_F(Vl53l4cxSensorTest, DerivesUpdateIntervalFromResponseProfile)
{
    ASSERT_TRUE(_database_admin.update(database::Config::Section::I2c::Vl53l4cx,
                                       Setting::Response,
                                       Response::Fast));
    EXPECT_EQ(_sensor.update_interval_ms(), 35);

    ASSERT_TRUE(_database_admin.update(database::Config::Section::I2c::Vl53l4cx,
                                       Setting::Response,
                                       Response::Balanced));
    EXPECT_EQ(_sensor.update_interval_ms(), 68);

    ASSERT_TRUE(_database_admin.update(database::Config::Section::I2c::Vl53l4cx,
                                       Setting::Response,
                                       Response::Stable));
    EXPECT_EQ(_sensor.update_interval_ms(), 102);
}

TEST_F(Vl53l4cxSensorTest, RejectsInvalidAddressIndex)
{
    EXPECT_FALSE(_sensor.init(I2C_ADDRESSES.size()));
    EXPECT_TRUE(_hwa.write_addresses.empty());
}

TEST_F(Vl53l4cxSensorTest, UsesSelectedAddressWhenSoftwareResetStarts)
{
    EXPECT_FALSE(_sensor.init(0));

    ASSERT_EQ(_hwa.write_addresses.size(), 1U);
    EXPECT_EQ(_hwa.write_addresses[0], I2C_ADDRESSES[0]);
    EXPECT_EQ(_hwa.written_bytes, 3U);
}

TEST_F(Vl53l4cxSensorTest, RejectsUnexpectedSensorIdentityAtDefaultAddress)
{
    constexpr uint16_t CAP1188_LIKE_ID = 0x505D;

    _hwa.write_succeeds     = true;
    _hwa.register_read_mode = true;
    _hwa.set_u16_be(VL53L4CX_REGISTER_MODEL_ID, CAP1188_LIKE_ID);

    EXPECT_FALSE(_sensor.init(0));

    ASSERT_FALSE(_hwa.write_read_addresses.empty());
    EXPECT_EQ(_hwa.write_read_addresses.back(), I2C_ADDRESSES[0]);
    const auto model_id_read = std::vector<uint8_t>{ 0x01, 0x0F };

    EXPECT_NE(std::find(_hwa.write_read_buffers.begin(), _hwa.write_read_buffers.end(), model_id_read),
              _hwa.write_read_buffers.end());
}

TEST_F(Vl53l4cxSensorTest, ReadsIdentityFromSixteenBitRegisterAddress)
{
    _hwa.write_succeeds     = true;
    _hwa.register_read_mode = true;
    _hwa.set_u16_be(VL53L4CX_REGISTER_MODEL_ID, VL53L4CX_EXPECTED_SENSOR_ID);

    EXPECT_FALSE(_sensor.init(0));

    const auto model_id_read = std::vector<uint8_t>{ 0x01, 0x0F };

    EXPECT_NE(std::find(_hwa.write_read_buffers.begin(), _hwa.write_read_buffers.end(), model_id_read),
              _hwa.write_read_buffers.end());
}
