/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"

#include "firmware/src/io/i2c/hwa/test/hwa_test.h"
#include "firmware/src/io/i2c/peripherals/sensor_bno085/instance/impl/mapper.h"
#include "firmware/src/io/i2c/peripherals/sensor_bno085/instance/impl/sensor_bno085.h"
#include "firmware/src/signaling/signaling.h"

#include <variant>
#include <vector>

using namespace opendeck;
using namespace opendeck::firmware;
using namespace opendeck::firmware::io::i2c;
using namespace opendeck::firmware::io::i2c::sensor_bno085;

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

    template<typename Payload>
    const Payload* payload_as(const signaling::OscSensorSignal& signal)
    {
        return std::get_if<Payload>(&signal.payload);
    }
}    // namespace

TEST(Bno085MapperTest, MapsRotationVectorToQuaternionAndEuler)
{
    Mapper mapper;

    const auto result = mapper.result(ROTATION_VECTOR_REPORT_ID,
                                      {
                                          ROTATION_VECTOR_SCALE,
                                          0,
                                          0,
                                          0,
                                      });

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->quaternion.has_value());
    ASSERT_TRUE(result->euler.has_value());

    const auto* quaternion = payload_as<signaling::OscSensorImuQuaternionSignal>(result->quaternion.value());
    const auto* euler      = payload_as<signaling::OscSensorImuEulerSignal>(result->euler.value());

    ASSERT_NE(quaternion, nullptr);
    ASSERT_NE(euler, nullptr);
    EXPECT_FLOAT_EQ(quaternion->real, 0.0F);
    EXPECT_FLOAT_EQ(quaternion->i, 1.0F);
    EXPECT_FLOAT_EQ(quaternion->j, 0.0F);
    EXPECT_FLOAT_EQ(quaternion->k, 0.0F);
    EXPECT_FLOAT_EQ(euler->yaw, 0.0F);
    EXPECT_FLOAT_EQ(euler->pitch, 0.0F);
    EXPECT_FLOAT_EQ(euler->roll, 180.0F);
}

TEST(Bno085MapperTest, MapsVectorReports)
{
    Mapper mapper;

    const auto gyro    = mapper.result(GYROSCOPE_REPORT_ID,
                                       {
                                           GYROSCOPE_SCALE,
                                           GYROSCOPE_SCALE / 2,
                                           -GYROSCOPE_SCALE,
                                           0,
                                       });
    const auto accel   = mapper.result(LINEAR_ACCEL_REPORT_ID,
                                       {
                                           ACCELERATION_SCALE,
                                           ACCELERATION_SCALE / 2,
                                           -ACCELERATION_SCALE,
                                           0,
                                       });
    const auto gravity = mapper.result(GRAVITY_REPORT_ID,
                                       {
                                           ACCELERATION_SCALE,
                                           ACCELERATION_SCALE / 2,
                                           -ACCELERATION_SCALE,
                                           0,
                                       });

    ASSERT_TRUE(gyro.has_value());
    ASSERT_TRUE(accel.has_value());
    ASSERT_TRUE(gravity.has_value());
    ASSERT_TRUE(gyro->gyroscope.has_value());
    ASSERT_TRUE(accel->linear_acceleration.has_value());
    ASSERT_TRUE(gravity->gravity.has_value());

    const auto* gyro_payload    = payload_as<signaling::OscSensorImuGyroscopeSignal>(gyro->gyroscope.value());
    const auto* accel_payload   = payload_as<signaling::OscSensorImuLinearAccelerationSignal>(accel->linear_acceleration.value());
    const auto* gravity_payload = payload_as<signaling::OscSensorImuGravitySignal>(gravity->gravity.value());

    ASSERT_NE(gyro_payload, nullptr);
    ASSERT_NE(accel_payload, nullptr);
    ASSERT_NE(gravity_payload, nullptr);
    EXPECT_FLOAT_EQ(gyro_payload->x, 1.0F);
    EXPECT_FLOAT_EQ(gyro_payload->y, 0.5F);
    EXPECT_FLOAT_EQ(gyro_payload->z, -1.0F);
    EXPECT_FLOAT_EQ(accel_payload->x, 1.0F);
    EXPECT_FLOAT_EQ(accel_payload->y, 0.5F);
    EXPECT_FLOAT_EQ(accel_payload->z, -1.0F);
    EXPECT_FLOAT_EQ(gravity_payload->x, 1.0F);
    EXPECT_FLOAT_EQ(gravity_payload->y, 0.5F);
    EXPECT_FLOAT_EQ(gravity_payload->z, -1.0F);
}

TEST(Bno085MapperTest, IgnoresUnknownReports)
{
    Mapper mapper;

    EXPECT_FALSE(mapper.result(0xFF, {}).has_value());
}

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
