/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/database.h"

#include "firmware/src/database/builder/builder.h"
#include "firmware/src/io/i2c/hwa/test/hwa_test.h"
#include "firmware/src/io/i2c/peripherals/sensor_vl53l4cx/instance/impl/mapper.h"
#include "firmware/src/io/i2c/peripherals/sensor_vl53l4cx/instance/impl/sensor_vl53l4cx.h"
#include "firmware/src/util/configurable/configurable.h"

#include <algorithm>
#include <variant>

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

    class Vl53l4cxMapperTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_database_admin.factory_reset());
            set_mapping(Setting::EnableDistanceMm, 1);
            set_mapping(Setting::EnableDistanceNorm, 1);
            set_mapping(Setting::DistanceUpperValue, DISTANCE_MAX_MM);
        }

        void set_mapping(sensor_vl53l4cx::Setting setting, uint32_t value)
        {
            ASSERT_TRUE(_database_admin.update(database::Config::Section::I2c::Vl53l4cx,
                                               setting,
                                               value));
        }

        tests::NoOpDatabaseHandlers _handlers;
        database::Builder           _database_builder;
        database::Admin&            _database_admin = _database_builder.instance();
        sensor_vl53l4cx::Database   _database       = sensor_vl53l4cx::Database(_database_admin);
        Mapper                      _mapper         = Mapper(_database);
    };

    template<typename Payload>
    const Payload* payload_as(const signaling::OscSensorSignal& signal)
    {
        return std::get_if<Payload>(&signal.payload);
    }
}    // namespace

TEST_F(Vl53l4cxMapperTest, MapsDistanceResult)
{
    const auto result = _mapper.result(1234U);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->distance_mm.has_value());
    ASSERT_TRUE(result->distance_normalized.has_value());

    const auto* mm_payload   = payload_as<signaling::OscSensorVl53l4cxDistanceSignal>(result->distance_mm.value());
    const auto* norm_payload = payload_as<signaling::OscSensorVl53l4cxDistanceNormSignal>(result->distance_normalized.value());

    ASSERT_NE(mm_payload, nullptr);
    ASSERT_NE(norm_payload, nullptr);
    EXPECT_EQ(result->distance_mm->direction, signaling::SignalDirection::Out);
    EXPECT_EQ(result->distance_normalized->direction, signaling::SignalDirection::Out);
    EXPECT_EQ(mm_payload->value, 1234);
    EXPECT_FLOAT_EQ(norm_payload->value, 1234.0F / static_cast<float>(DISTANCE_MAX_MM));
}

TEST_F(Vl53l4cxMapperTest, AppliesDistanceValues)
{
    constexpr uint16_t lower = 300;
    constexpr uint16_t upper = 4500;

    set_mapping(Setting::DistanceLowerValue, lower);
    set_mapping(Setting::DistanceUpperValue, upper);

    const auto below = _mapper.result(lower - 1U);
    const auto mid   = _mapper.result((lower + upper) / 2U);
    const auto above = _mapper.result(upper + 1U);

    ASSERT_TRUE(below.has_value());
    ASSERT_TRUE(mid.has_value());
    ASSERT_TRUE(above.has_value());
    ASSERT_TRUE(below->distance_normalized.has_value());
    ASSERT_TRUE(mid->distance_normalized.has_value());
    ASSERT_TRUE(above->distance_normalized.has_value());

    const auto* below_payload = payload_as<signaling::OscSensorVl53l4cxDistanceNormSignal>(below->distance_normalized.value());
    const auto* mid_payload   = payload_as<signaling::OscSensorVl53l4cxDistanceNormSignal>(mid->distance_normalized.value());
    const auto* above_payload = payload_as<signaling::OscSensorVl53l4cxDistanceNormSignal>(above->distance_normalized.value());

    ASSERT_NE(below_payload, nullptr);
    ASSERT_NE(mid_payload, nullptr);
    ASSERT_NE(above_payload, nullptr);

    EXPECT_FLOAT_EQ(below_payload->value, 0.0F);
    EXPECT_FLOAT_EQ(mid_payload->value, 0.5F);
    EXPECT_FLOAT_EQ(above_payload->value, 1.0F);
}

TEST_F(Vl53l4cxMapperTest, SuppressesUnchangedNormalizedDistance)
{
    constexpr uint16_t lower = 300;
    constexpr uint16_t upper = 4500;

    set_mapping(Setting::EnableDistanceMm, 0);
    set_mapping(Setting::DistanceLowerValue, lower);
    set_mapping(Setting::DistanceUpperValue, upper);

    const auto first = _mapper.result(upper + 1U);
    const auto same  = _mapper.result(upper + 200U);

    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(first->distance_normalized.has_value());
    EXPECT_FALSE(first->distance_mm.has_value());
    EXPECT_FALSE(same.has_value());
}

TEST_F(Vl53l4cxMapperTest, RebuildsLastDistanceResult)
{
    ASSERT_TRUE(_mapper.result(1234U).has_value());

    const auto result = _mapper.last_result();

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->distance_mm.has_value());
    ASSERT_TRUE(result->distance_normalized.has_value());
}

TEST_F(Vl53l4cxSensorTest, ReportsSupportedI2cAddresses)
{
    const auto addresses = _sensor.i2c_addresses();

    ASSERT_EQ(addresses.size(), I2C_ADDRESSES.size());
    EXPECT_EQ(addresses[0], 0x29);
}

TEST_F(Vl53l4cxSensorTest, UsesFastUpdateInterval)
{
    EXPECT_EQ(_sensor.update_interval_ms(), 35);
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
