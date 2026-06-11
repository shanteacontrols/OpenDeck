/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/database.h"

#include "firmware/src/database/builder/builder.h"
#include "firmware/src/io/i2c/hwa/test/hwa_test.h"
#include "firmware/src/io/i2c/peripherals/sensor_vl53l5cx/instance/impl/sensor_vl53l5cx.h"
#include "firmware/src/util/configurable/configurable.h"

#include <vector>

using namespace opendeck;
using namespace opendeck::firmware;
using namespace opendeck::firmware::io::i2c;
using namespace opendeck::firmware::io::i2c::sensor_vl53l5cx;

namespace
{
    void configure_valid_vl53l5cx_registers(HwaTest& hwa)
    {
        hwa.register_read_mode  = true;
        hwa.register_write_mode = true;
        hwa.set_u8(VL53L5CX_REGISTER_DEVICE_ID, VL53L5CX_EXPECTED_DEVICE_ID);
        hwa.set_u8(VL53L5CX_REGISTER_REVISION_ID, VL53L5CX_EXPECTED_REVISION_ID);
    }

    class Vl53l5cxSensorTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_database_admin.factory_reset());
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            signaling::clear_registry();
        }

        uint8_t set_config(Setting setting, uint16_t value)
        {
            return ConfigHandler.set(sys::Config::Block::I2c,
                                     static_cast<uint8_t>(sys::Config::Section::I2c::Vl53l5cx),
                                     static_cast<size_t>(setting),
                                     value);
        }

        uint8_t get_config(Setting setting, uint16_t& value)
        {
            return ConfigHandler.get(sys::Config::Block::I2c,
                                     static_cast<uint8_t>(sys::Config::Section::I2c::Vl53l5cx),
                                     static_cast<size_t>(setting),
                                     value);
        }

        tests::NoOpDatabaseHandlers _handlers;
        database::Builder           _database_builder;
        database::Admin&            _database_admin = _database_builder.instance();
        HwaTest                     _hwa;
        Database                    _database = Database(_database_admin);
        SensorVl53l5cx              _sensor   = SensorVl53l5cx(_hwa, _database);
    };
}    // namespace

TEST_F(Vl53l5cxSensorTest, ExposesSupportedI2cAddresses)
{
    const auto addresses = _sensor.i2c_addresses();

    ASSERT_EQ(addresses.size(), I2C_ADDRESSES.size());
    EXPECT_EQ(addresses[0], 0x29);
}

TEST_F(Vl53l5cxSensorTest, RejectsInvalidAddressIndex)
{
    EXPECT_FALSE(_sensor.init(I2C_ADDRESSES.size()));
    EXPECT_TRUE(_hwa.write_addresses.empty());
}

TEST_F(Vl53l5cxSensorTest, RejectsUnexpectedDeviceId)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[0] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_vl53l5cx_registers(_hwa);
    _hwa.set_u8(VL53L5CX_REGISTER_DEVICE_ID, 0x00);

    EXPECT_FALSE(_sensor.init(0));
    EXPECT_FALSE(_sensor.update());
    EXPECT_EQ(_hwa.registers[VL53L5CX_REGISTER_PAGE_SELECT], VL53L5CX_DEFAULT_PAGE);
}

TEST_F(Vl53l5cxSensorTest, RejectsUnexpectedRevisionId)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[0] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_vl53l5cx_registers(_hwa);
    _hwa.set_u8(VL53L5CX_REGISTER_REVISION_ID, 0x00);

    EXPECT_FALSE(_sensor.init(0));
    EXPECT_FALSE(_sensor.update());
    EXPECT_EQ(_hwa.registers[VL53L5CX_REGISTER_PAGE_SELECT], VL53L5CX_DEFAULT_PAGE);
}

TEST_F(Vl53l5cxSensorTest, RejectsWhenPageSelectFails)
{
    _hwa.write_succeeds = false;
    configure_valid_vl53l5cx_registers(_hwa);

    EXPECT_FALSE(_sensor.init(0));
    EXPECT_FALSE(_sensor.update());
    EXPECT_EQ(_hwa.write_addresses.size(), 1U);
}

TEST_F(Vl53l5cxSensorTest, RejectsWhenIdentityReadFails)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[0] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_vl53l5cx_registers(_hwa);
    _hwa.write_read_succeeds = false;

    EXPECT_FALSE(_sensor.init(0));
    EXPECT_FALSE(_sensor.update());
    EXPECT_EQ(_hwa.registers[VL53L5CX_REGISTER_PAGE_SELECT], VL53L5CX_DEFAULT_PAGE);
}

TEST_F(Vl53l5cxSensorTest, DeinitClearsDetectedState)
{
    EXPECT_TRUE(_sensor.deinit());
    EXPECT_FALSE(_sensor.update());
}

TEST_F(Vl53l5cxSensorTest, KeepsSensorPollingIntervalIndependentOfOutputRate)
{
    uint16_t value = 0;

    EXPECT_EQ(_sensor.update_interval_ms(), 10);

    ASSERT_EQ(set_config(Setting::OutputRate, static_cast<uint16_t>(OutputRate::Low)),
              sys::Config::Status::Ack);
    ASSERT_EQ(get_config(Setting::OutputRate, value), sys::Config::Status::Ack);
    EXPECT_EQ(value, static_cast<uint16_t>(OutputRate::Low));
    EXPECT_EQ(_sensor.update_interval_ms(), 10);

    ASSERT_EQ(set_config(Setting::OutputRate, static_cast<uint16_t>(OutputRate::Normal)),
              sys::Config::Status::Ack);
    ASSERT_EQ(get_config(Setting::OutputRate, value), sys::Config::Status::Ack);
    EXPECT_EQ(value, static_cast<uint16_t>(OutputRate::Normal));
    EXPECT_EQ(_sensor.update_interval_ms(), 10);

    ASSERT_EQ(set_config(Setting::OutputRate, static_cast<uint16_t>(OutputRate::High)),
              sys::Config::Status::Ack);
    ASSERT_EQ(get_config(Setting::OutputRate, value), sys::Config::Status::Ack);
    EXPECT_EQ(value, static_cast<uint16_t>(OutputRate::High));
    EXPECT_EQ(_sensor.update_interval_ms(), 10);
}

TEST_F(Vl53l5cxSensorTest, RejectsInvalidConfigValues)
{
    EXPECT_EQ(set_config(Setting::Smoothing, static_cast<uint16_t>(Smoothing::Count)),
              sys::Config::Status::ErrorWrite);
    EXPECT_EQ(set_config(Setting::OutputMode, static_cast<uint16_t>(OutputMode::Count)),
              sys::Config::Status::ErrorWrite);
    EXPECT_EQ(set_config(Setting::DistanceLowerValue, DISTANCE_MAX_MM + 1U),
              sys::Config::Status::ErrorWrite);
    EXPECT_EQ(set_config(Setting::DistanceUpperValue, DISTANCE_MAX_MM + 1U),
              sys::Config::Status::ErrorWrite);
    EXPECT_EQ(set_config(Setting::InvertX, 2),
              sys::Config::Status::ErrorWrite);
    EXPECT_EQ(set_config(Setting::InvertY, 2),
              sys::Config::Status::ErrorWrite);
    EXPECT_EQ(set_config(Setting::Rotation, static_cast<uint16_t>(Rotation::Count)),
              sys::Config::Status::ErrorWrite);
    EXPECT_EQ(set_config(Setting::OutputRate, static_cast<uint16_t>(OutputRate::Count)),
              sys::Config::Status::ErrorWrite);
}

TEST_F(Vl53l5cxSensorTest, AcceptsValidConfigValues)
{
    EXPECT_EQ(set_config(Setting::Smoothing, static_cast<uint16_t>(Smoothing::Heavy)),
              sys::Config::Status::Ack);
    EXPECT_EQ(set_config(Setting::OutputMode, static_cast<uint16_t>(OutputMode::Presence)),
              sys::Config::Status::Ack);
    EXPECT_EQ(set_config(Setting::DistanceLowerValue, 123),
              sys::Config::Status::Ack);
    EXPECT_EQ(set_config(Setting::DistanceUpperValue, DISTANCE_MAX_MM),
              sys::Config::Status::Ack);
    EXPECT_EQ(set_config(Setting::InvertX, 1),
              sys::Config::Status::Ack);
    EXPECT_EQ(set_config(Setting::InvertY, 1),
              sys::Config::Status::Ack);
    EXPECT_EQ(set_config(Setting::Rotation, static_cast<uint16_t>(Rotation::Deg270)),
              sys::Config::Status::Ack);
    EXPECT_EQ(set_config(Setting::OutputRate, static_cast<uint16_t>(OutputRate::High)),
              sys::Config::Status::Ack);
}
