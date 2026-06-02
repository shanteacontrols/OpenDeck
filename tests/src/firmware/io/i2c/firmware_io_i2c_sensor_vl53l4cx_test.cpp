/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/database.h"

#include "firmware/src/database/builder/builder.h"
#include "firmware/src/io/i2c/hwa/test/hwa_test.h"
#include "firmware/src/io/i2c/peripherals/sensor_vl53l4cx/sensor_vl53l4cx.h"
#include "firmware/src/util/configurable/configurable.h"

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
