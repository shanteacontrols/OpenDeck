/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/database.h"

#include "firmware/src/database/builder/builder.h"
#include "firmware/src/io/i2c/hwa/test/hwa_test.h"
#include "firmware/src/io/i2c/peripherals/sensor_apds9960/instance/impl/sensor_apds9960.h"
#include "firmware/src/io/i2c/shared/value_filter.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/util/configurable/configurable.h"

#include "zlibs/utils/misc/mutex.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <variant>
#include <vector>

using namespace opendeck;
using namespace opendeck::firmware;
using namespace opendeck::firmware::io::i2c;
using namespace opendeck::firmware::io::i2c::sensor_apds9960;

namespace
{
    constexpr int64_t SAMPLE_DELAY_MS = 60;
    constexpr int64_t VALUE_DELAY_MS  = 280;
    constexpr int64_t STABLE_DELAY_MS = 2100;

    constexpr uint8_t apds9960_control_value(uint8_t proximity_gain, uint8_t als_gain)
    {
        return static_cast<uint8_t>((APDS9960_DEFAULT_LED_DRIVE << APDS9960_CONTROL_LED_DRIVE_SHIFT) |
                                    (proximity_gain << APDS9960_CONTROL_PROXIMITY_GAIN_SHIFT) |
                                    als_gain);
    }

    class SensorCollector
    {
        public:
        SensorCollector()
        {
            signaling::subscribe<signaling::OscSensorSignal>(
                [this](const signaling::OscSensorSignal& signal)
                {
                    const zlibs::utils::misc::LockGuard lock(_mutex);
                    _signals.push_back(signal);
                });
        }

        void clear()
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _signals.clear();
        }

        std::vector<signaling::OscSensorSignal> snapshot() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _signals;
        }

        size_t count() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _signals.size();
        }

        private:
        mutable zlibs::utils::misc::Mutex       _mutex;
        std::vector<signaling::OscSensorSignal> _signals = {};
    };

    class Apds9960SensorTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            _hwa.set_available_addresses({ I2C_ADDRESSES[0] });
            _hwa.require_available_address_for_transfers = true;
            _hwa.register_write_mode                     = true;
            _hwa.register_read_mode                      = true;
            _hwa.set_device_id_register(APDS9960_REGISTER_ID, sensor_apds9960::APDS9960_DEVICE_IDS[0]);

            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_database_admin.factory_reset());
            ASSERT_TRUE(_database_admin.update(database::Config::Section::I2c::Apds9960,
                                               sensor_apds9960::Setting::EnableProximity,
                                               1));
            ASSERT_TRUE(_database_admin.update(database::Config::Section::I2c::Apds9960,
                                               sensor_apds9960::Setting::EnableAmbientLight,
                                               1));
            ASSERT_TRUE(_database_admin.update(database::Config::Section::I2c::Apds9960,
                                               sensor_apds9960::Setting::EnableRgb,
                                               1));

            set_sensor_values(1, 62, 32, 25, 19);

            ASSERT_TRUE(_sensor.init(0));
            publish_network_identity();
            drain();
            _collector.clear();
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            signaling::clear_registry();
        }

        void set_sensor_values(uint8_t proximity, uint16_t ambient_light, uint16_t red, uint16_t green, uint16_t blue)
        {
            _hwa.set_u8(APDS9960_REGISTER_PDATA, proximity);
            _hwa.set_u16(APDS9960_REGISTER_CDATAL, ambient_light);
            _hwa.set_u16(APDS9960_REGISTER_RDATAL, red);
            _hwa.set_u16(APDS9960_REGISTER_GDATAL, green);
            _hwa.set_u16(APDS9960_REGISTER_BDATAL, blue);
            _hwa.set_u8(APDS9960_REGISTER_STATUS, APDS9960_STATUS_AVALID | APDS9960_STATUS_PVALID);
            _hwa.set_u8(APDS9960_REGISTER_GSTATUS, 0);
        }

        void update_after(int64_t delay_ms)
        {
            k_msleep(delay_ms);
            _sensor.update();
            drain();
        }

        void publish_network_identity()
        {
            ASSERT_TRUE(signaling::publish(signaling::NetworkIdentitySignal("opendeck-test.local", "192.168.1.129")));
        }

        void publish_empty_network_identity()
        {
            ASSERT_TRUE(signaling::publish(signaling::NetworkIdentitySignal("opendeck-test.local", {})));
        }

        void drain()
        {
            ASSERT_TRUE(zlibs::utils::signaling::drain());
        }

        void set_apds_output(sensor_apds9960::Setting setting, uint32_t value)
        {
            ASSERT_TRUE(_database_admin.update(database::Config::Section::I2c::Apds9960,
                                               setting,
                                               value));
            ASSERT_TRUE(_sensor.init(0));
            _collector.clear();
        }

        tests::NoOpDatabaseHandlers     _handlers;
        database::Builder               _database_builder;
        database::Admin&                _database_admin = _database_builder.instance();
        HwaTest                         _hwa;
        sensor_apds9960::Database       _database = sensor_apds9960::Database(_database_admin);
        sensor_apds9960::SensorApds9960 _sensor   = sensor_apds9960::SensorApds9960(_hwa, _database);
        SensorCollector                 _collector;
    };

    template<typename Payload>
    size_t count_signal(const std::vector<signaling::OscSensorSignal>& signals)
    {
        size_t count = 0;

        for (const auto& signal : signals)
        {
            if (std::holds_alternative<Payload>(signal.payload))
            {
                count++;
            }
        }

        return count;
    }

    template<typename Payload>
    const Payload* payload_as(const signaling::OscSensorSignal& signal)
    {
        return std::get_if<Payload>(&signal.payload);
    }
}    // namespace

TEST(Apds9960SensorStandaloneTest, AcceptsKnownApds9960DeviceIds)
{
    for (uint8_t device_id : sensor_apds9960::APDS9960_DEVICE_IDS)
    {
        SCOPED_TRACE(device_id);

        tests::NoOpDatabaseHandlers handlers;
        database::Builder           database_builder;
        database::Admin&            database_admin = database_builder.instance();
        HwaTest                     hwa;

        hwa.set_available_addresses({ I2C_ADDRESSES[0] });
        hwa.require_available_address_for_transfers = true;
        hwa.register_write_mode                     = true;
        hwa.register_read_mode                      = true;
        hwa.set_device_id_register(APDS9960_REGISTER_ID, device_id);

        ASSERT_TRUE(database_admin.init(handlers));
        ASSERT_TRUE(database_admin.factory_reset());
        ASSERT_TRUE(database_admin.update(database::Config::Section::I2c::Apds9960,
                                          sensor_apds9960::Setting::EnableProximity,
                                          1));

        sensor_apds9960::Database       database(database_admin);
        sensor_apds9960::SensorApds9960 sensor(hwa, database);

        EXPECT_TRUE(sensor.init(0));

        ConfigHandler.clear();
        signaling::clear_registry();
    }
}

TEST(I2cValueFilterTest, ConfirmsChangedValueWithoutTimeThrottle)
{
    ValueFilter<1> filter;

    EXPECT_TRUE(filter.update({ 62 }, 16, 2));

    EXPECT_FALSE(filter.update({ 90 }, 16, 2));
    EXPECT_TRUE(filter.update({ 91 }, 16, 2));
    EXPECT_EQ(filter.value()[0], 91U);
}

TEST(I2cValueFilterTest, AcceptsMeaningfulStepsImmediatelyWhileMoving)
{
    ValueFilter<1> filter;

    EXPECT_TRUE(filter.update({ 62 }, 16, 2));
    EXPECT_FALSE(filter.update({ 90 }, 16, 2));
    EXPECT_TRUE(filter.update({ 91 }, 16, 2));

    EXPECT_TRUE(filter.update({ 120 }, 16, 2));
    EXPECT_EQ(filter.value()[0], 120U);

    EXPECT_FALSE(filter.update({ 125 }, 16, 2));
    EXPECT_FALSE(filter.update({ 136 }, 16, 2));
    EXPECT_TRUE(filter.update({ 137 }, 16, 2));
    EXPECT_EQ(filter.value()[0], 137U);
}

TEST(I2cValueFilterTest, UsesLowerThresholdWhileMoving)
{
    ValueFilter<1> filter;

    EXPECT_TRUE(filter.update({ 25 }, 16, 2, ValueFilter<1>::ConfirmationMode::Exact, 8));
    EXPECT_FALSE(filter.update({ 42 }, 16, 2, ValueFilter<1>::ConfirmationMode::Exact, 8));
    EXPECT_TRUE(filter.update({ 42 }, 16, 2, ValueFilter<1>::ConfirmationMode::Exact, 8));

    EXPECT_TRUE(filter.update({ 51 }, 16, 2, ValueFilter<1>::ConfirmationMode::Exact, 8));
    EXPECT_EQ(filter.value()[0], 51U);

    EXPECT_FALSE(filter.update({ 55 }, 16, 2, ValueFilter<1>::ConfirmationMode::Exact, 8));
    EXPECT_FALSE(filter.update({ 62 }, 16, 2, ValueFilter<1>::ConfirmationMode::Exact, 8));
}

TEST(I2cValueFilterTest, RequiresExactRepeatWhenConfigured)
{
    ValueFilter<1> filter;

    EXPECT_TRUE(filter.update({ 5 }, 2, 2, ValueFilter<1>::ConfirmationMode::Exact));

    EXPECT_FALSE(filter.update({ 7 }, 2, 2, ValueFilter<1>::ConfirmationMode::Exact));
    EXPECT_FALSE(filter.update({ 8 }, 2, 2, ValueFilter<1>::ConfirmationMode::Exact));
    EXPECT_TRUE(filter.update({ 8 }, 2, 2, ValueFilter<1>::ConfirmationMode::Exact));
    EXPECT_EQ(filter.value()[0], 8U);
}

TEST(I2cValueFilterTest, RestartsConfirmationForDifferentChangedValue)
{
    ValueFilter<1> filter;

    EXPECT_TRUE(filter.update({ 62 }, 16, 2));

    EXPECT_FALSE(filter.update({ 90 }, 16, 2));
    EXPECT_FALSE(filter.update({ 120 }, 16, 2));
    EXPECT_TRUE(filter.update({ 121 }, 16, 2));
    EXPECT_EQ(filter.value()[0], 121U);
}

TEST(I2cValueFilterTest, FiltersGroupedJitter)
{
    ValueFilter<3> filter;

    EXPECT_TRUE(filter.update({ 483, 352, 338 }, 4, 2, 4));

    EXPECT_FALSE(filter.update({ 482, 352, 338 }, 4, 2, 4));
    EXPECT_FALSE(filter.update({ 483, 353, 338 }, 4, 2, 4));
    EXPECT_FALSE(filter.update({ 483, 352, 339 }, 4, 2, 4));
}

TEST(I2cValueFilterTest, ConfirmsGroupedChangesThenStreamsMovement)
{
    ValueFilter<3> filter;

    EXPECT_TRUE(filter.update({ 483, 352, 338 }, 4, 2, 4));

    EXPECT_FALSE(filter.update({ 490, 352, 338 }, 4, 2, 4));
    EXPECT_TRUE(filter.update({ 491, 352, 338 }, 4, 2, 4));
    EXPECT_EQ(filter.value()[0], 491U);

    EXPECT_TRUE(filter.update({ 496, 352, 338 }, 4, 2, 4));
    EXPECT_EQ(filter.value()[0], 496U);
}

TEST_F(Apds9960SensorTest, PublishesInitialContinuousOutputsIndependently)
{
    update_after(SAMPLE_DELAY_MS);

    auto signals = _collector.snapshot();

    EXPECT_EQ(count_signal<signaling::OscSensorProximitySignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorAmbientLightSignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorRgbSignal>(signals), 1U);
}

TEST_F(Apds9960SensorTest, LeavesGestureModeOffSoAlsProximityAndColorCanUpdate)
{
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_GCONF4] & APDS9960_GCONF4_GMODE, 0U);
}

TEST_F(Apds9960SensorTest, KeepsGestureEngineDisabledForContinuousSensorMode)
{
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_ENABLE] & APDS9960_ENABLE_GEN, 0U);
}

TEST_F(Apds9960SensorTest, ConfiguresProximityPulseGainAndLedDrive)
{
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_PPULSE], APDS9960_DEFAULT_PPULSE);
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_CONTROL],
              apds9960_control_value(APDS9960_DEFAULT_PROXIMITY_GAIN, APDS9960_DEFAULT_ALS_GAIN));
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_CONFIG2], APDS9960_DEFAULT_CONFIG2);
}

TEST_F(Apds9960SensorTest, ConfiguresSelectedProximityGain)
{
    set_apds_output(sensor_apds9960::Setting::ProximityGain, 3);

    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_CONTROL],
              apds9960_control_value(3, APDS9960_DEFAULT_ALS_GAIN));
}

TEST_F(Apds9960SensorTest, ConfiguresSelectedAlsGain)
{
    set_apds_output(sensor_apds9960::Setting::AlsGain, 3);

    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_CONTROL],
              apds9960_control_value(APDS9960_DEFAULT_PROXIMITY_GAIN, 3));
}

TEST_F(Apds9960SensorTest, ConfiguresGestureThresholdsAndGain)
{
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_GPENTH], APDS9960_DEFAULT_GPENTH);
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_GEXTH], APDS9960_DEFAULT_GEXTH);
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_GCONF2], APDS9960_DEFAULT_GCONF2);
}

TEST_F(Apds9960SensorTest, EnablesGestureEngineAlongsideContinuousOutputs)
{
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_ENABLE] & APDS9960_ENABLE_GEN, 0U);
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_GCONF4] & APDS9960_GCONF4_GMODE, 0U);

    set_apds_output(sensor_apds9960::Setting::EnableGesture, 1);

    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_ENABLE] & APDS9960_ENABLE_GESTURE, APDS9960_ENABLE_GESTURE);
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_GCONF4] & APDS9960_GCONF4_GMODE, 0U);
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_PPULSE], APDS9960_DEFAULT_PPULSE);
}

TEST_F(Apds9960SensorTest, GestureSettingDoesNotBlockContinuousRefresh)
{
    set_apds_output(sensor_apds9960::Setting::EnableGesture, 1);

    update_after(SAMPLE_DELAY_MS);

    auto signals = _collector.snapshot();

    EXPECT_EQ(count_signal<signaling::OscSensorProximitySignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorAmbientLightSignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorRgbSignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorGestureSignal>(signals), 0U);
}

TEST_F(Apds9960SensorTest, DoesNotPublishStaleContinuousRegistersBeforeStatusIsValid)
{
    _hwa.set_u8(APDS9960_REGISTER_STATUS, 0);

    update_after(SAMPLE_DELAY_MS);

    const auto signals = _collector.snapshot();

    EXPECT_EQ(count_signal<signaling::OscSensorProximitySignal>(signals), 0U);
    EXPECT_EQ(count_signal<signaling::OscSensorAmbientLightSignal>(signals), 0U);
    EXPECT_EQ(count_signal<signaling::OscSensorRgbSignal>(signals), 0U);
}

TEST_F(Apds9960SensorTest, DebouncesEachContinuousOutputIndependently)
{
    set_apds_output(sensor_apds9960::Setting::EnableRgb, 0);

    update_after(SAMPLE_DELAY_MS);
    _collector.clear();

    set_sensor_values(24, 62, 32, 25, 19);
    update_after(0);

    EXPECT_TRUE(_collector.snapshot().empty());

    update_after(0);

    auto signals = _collector.snapshot();
    ASSERT_EQ(signals.size(), 1U);
    const auto* proximity = payload_as<signaling::OscSensorProximitySignal>(signals[0]);
    ASSERT_NE(proximity, nullptr);
    EXPECT_EQ(proximity->value, 24);
    _collector.clear();

    set_sensor_values(24, 90, 32, 25, 19);
    update_after(VALUE_DELAY_MS);

    EXPECT_TRUE(_collector.snapshot().empty());

    update_after(0);

    signals = _collector.snapshot();
    ASSERT_EQ(signals.size(), 1U);
    const auto* ambient_light = payload_as<signaling::OscSensorAmbientLightSignal>(signals[0]);
    ASSERT_NE(ambient_light, nullptr);
    EXPECT_EQ(ambient_light->value, 90);
}

TEST_F(Apds9960SensorTest, FiltersAmbientLightWithoutTimeThrottle)
{
    set_apds_output(sensor_apds9960::Setting::EnableRgb, 0);

    update_after(SAMPLE_DELAY_MS);
    _collector.clear();

    set_sensor_values(1, 90, 32, 25, 19);
    update_after(0);

    EXPECT_TRUE(_collector.snapshot().empty());

    update_after(0);

    const auto signals = _collector.snapshot();

    ASSERT_EQ(signals.size(), 1U);
    const auto* ambient_light = payload_as<signaling::OscSensorAmbientLightSignal>(signals[0]);
    ASSERT_NE(ambient_light, nullptr);
    EXPECT_EQ(ambient_light->value, 90);
}

TEST_F(Apds9960SensorTest, FiltersProximityNoiseWithoutTimeThrottle)
{
    set_apds_output(sensor_apds9960::Setting::EnableRgb, 0);

    update_after(SAMPLE_DELAY_MS);
    _collector.clear();

    for (uint8_t proximity : { 8, 8, 5, 5, 10, 10, 6, 6, 9, 9, 7, 7, 9, 9, 6, 6, 8, 8, 5, 5, 9, 9, 6, 6, 9, 9, 6, 6, 8, 8, 13, 13 })
    {
        set_sensor_values(proximity, 62, 32, 25, 19);
        update_after(0);
    }

    EXPECT_TRUE(_collector.snapshot().empty());

    set_sensor_values(24, 62, 32, 25, 19);
    update_after(0);

    EXPECT_TRUE(_collector.snapshot().empty());

    update_after(0);

    auto signals = _collector.snapshot();

    ASSERT_EQ(signals.size(), 1U);
    const auto* proximity = payload_as<signaling::OscSensorProximitySignal>(signals[0]);
    ASSERT_NE(proximity, nullptr);
    EXPECT_EQ(proximity->value, 24);
    _collector.clear();

    set_sensor_values(34, 62, 32, 25, 19);
    update_after(0);

    signals = _collector.snapshot();

    ASSERT_EQ(signals.size(), 1U);
    proximity = payload_as<signaling::OscSensorProximitySignal>(signals[0]);
    ASSERT_NE(proximity, nullptr);
    EXPECT_EQ(proximity->value, 34);
}

TEST_F(Apds9960SensorTest, FiltersRgbNoiseAsTupleWithoutTimeThrottle)
{
    update_after(SAMPLE_DELAY_MS);
    _collector.clear();

    set_sensor_values(1, 62, 483, 352, 338);
    update_after(0);

    EXPECT_TRUE(_collector.snapshot().empty());

    update_after(0);

    auto signals = _collector.snapshot();
    ASSERT_EQ(signals.size(), 1U);
    const auto* rgb = payload_as<signaling::OscSensorRgbSignal>(signals[0]);
    ASSERT_NE(rgb, nullptr);
    EXPECT_EQ(rgb->red, 483);
    EXPECT_EQ(rgb->green, 352);
    EXPECT_EQ(rgb->blue, 338);
    _collector.clear();

    for (const auto& rgb : {
             std::array<uint16_t, 3>{ 482, 352, 338 },
             std::array<uint16_t, 3>{ 483, 353, 338 },
             std::array<uint16_t, 3>{ 483, 352, 339 },
         })
    {
        set_sensor_values(1, 62, rgb[0], rgb[1], rgb[2]);
        update_after(0);
    }

    EXPECT_TRUE(_collector.snapshot().empty());

    set_sensor_values(1, 62, 490, 352, 338);
    update_after(0);

    EXPECT_TRUE(_collector.snapshot().empty());

    set_sensor_values(1, 62, 491, 352, 338);
    update_after(0);

    signals = _collector.snapshot();
    ASSERT_EQ(signals.size(), 1U);
    rgb = payload_as<signaling::OscSensorRgbSignal>(signals[0]);
    ASSERT_NE(rgb, nullptr);
    EXPECT_EQ(rgb->red, 491);
    EXPECT_EQ(rgb->green, 352);
    EXPECT_EQ(rgb->blue, 338);
}

TEST_F(Apds9960SensorTest, ProximityOutputCanBeDisabled)
{
    set_apds_output(sensor_apds9960::Setting::EnableProximity, 0);

    update_after(SAMPLE_DELAY_MS);

    const auto signals = _collector.snapshot();

    EXPECT_EQ(count_signal<signaling::OscSensorProximitySignal>(signals), 0U);
    EXPECT_EQ(count_signal<signaling::OscSensorAmbientLightSignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorRgbSignal>(signals), 1U);
}

TEST_F(Apds9960SensorTest, AmbientLightOutputCanBeDisabled)
{
    set_apds_output(sensor_apds9960::Setting::EnableAmbientLight, 0);

    update_after(SAMPLE_DELAY_MS);

    const auto signals = _collector.snapshot();

    EXPECT_EQ(count_signal<signaling::OscSensorProximitySignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorAmbientLightSignal>(signals), 0U);
    EXPECT_EQ(count_signal<signaling::OscSensorRgbSignal>(signals), 1U);
}

TEST_F(Apds9960SensorTest, RgbOutputCanBeDisabled)
{
    set_apds_output(sensor_apds9960::Setting::EnableRgb, 0);

    update_after(SAMPLE_DELAY_MS);

    const auto signals = _collector.snapshot();

    EXPECT_EQ(count_signal<signaling::OscSensorProximitySignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorAmbientLightSignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorRgbSignal>(signals), 0U);
}

TEST_F(Apds9960SensorTest, PublishesGestureFromFifoEdgePair)
{
    set_apds_output(sensor_apds9960::Setting::EnableGesture, 1);
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_ENABLE] & APDS9960_ENABLE_GESTURE, APDS9960_ENABLE_GESTURE);
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_GCONF4] & APDS9960_GCONF4_GMODE, 0U);
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_PPULSE], APDS9960_DEFAULT_PPULSE);
    EXPECT_EQ(_hwa.registers[APDS9960_REGISTER_CONFIG2], APDS9960_DEFAULT_CONFIG2);

    _hwa.set_gesture_fifo(APDS9960_REGISTER_GFIFO_U,
                          APDS9960_REGISTER_GFLVL,
                          APDS9960_REGISTER_GSTATUS,
                          APDS9960_GSTATUS_GVALID,
                          {
                              { 70, 30, 40, 40 },
                              { 65, 35, 40, 40 },
                              { 55, 45, 40, 40 },
                              { 45, 55, 40, 40 },
                              { 35, 65, 40, 40 },
                              { 30, 70, 40, 40 },
                          });

    update_after(SAMPLE_DELAY_MS);
    const auto signals        = _collector.snapshot();
    const auto gesture_signal = std::find_if(signals.begin(),
                                             signals.end(),
                                             [](const signaling::OscSensorSignal& signal)
                                             {
                                                 return std::holds_alternative<signaling::OscSensorGestureSignal>(signal.payload);
                                             });

    ASSERT_NE(gesture_signal, signals.end());
    const auto* gesture = payload_as<signaling::OscSensorGestureSignal>(*gesture_signal);
    ASSERT_NE(gesture, nullptr);
    EXPECT_EQ(gesture->gesture, signaling::OscSensorGesture::Up);
}

TEST_F(Apds9960SensorTest, RateLimitsGestureOutput)
{
    set_apds_output(sensor_apds9960::Setting::EnableProximity, 0);
    set_apds_output(sensor_apds9960::Setting::EnableAmbientLight, 0);
    set_apds_output(sensor_apds9960::Setting::EnableRgb, 0);
    set_apds_output(sensor_apds9960::Setting::EnableGesture, 1);

    auto set_up_gesture = [this]()
    {
        _hwa.set_gesture_fifo(APDS9960_REGISTER_GFIFO_U,
                              APDS9960_REGISTER_GFLVL,
                              APDS9960_REGISTER_GSTATUS,
                              APDS9960_GSTATUS_GVALID,
                              {
                                  { 70, 30, 40, 40 },
                                  { 65, 35, 40, 40 },
                                  { 55, 45, 40, 40 },
                                  { 45, 55, 40, 40 },
                                  { 35, 65, 40, 40 },
                                  { 30, 70, 40, 40 },
                              });
    };

    set_up_gesture();
    update_after(SAMPLE_DELAY_MS);
    EXPECT_EQ(count_signal<signaling::OscSensorGestureSignal>(_collector.snapshot()), 1U);
    _collector.clear();

    set_up_gesture();
    update_after(SAMPLE_DELAY_MS);
    EXPECT_EQ(count_signal<signaling::OscSensorGestureSignal>(_collector.snapshot()), 0U);

    set_up_gesture();
    update_after(800);
    EXPECT_EQ(count_signal<signaling::OscSensorGestureSignal>(_collector.snapshot()), 1U);
}

TEST_F(Apds9960SensorTest, DoesNotRepublishStableContinuousOutputs)
{
    set_apds_output(sensor_apds9960::Setting::EnableRgb, 0);

    update_after(SAMPLE_DELAY_MS);
    _collector.clear();

    update_after(STABLE_DELAY_MS);

    const auto signals = _collector.snapshot();

    EXPECT_TRUE(signals.empty());
}

TEST_F(Apds9960SensorTest, PublishesLastContinuousOutputsOnForcedRefreshStart)
{
    update_after(SAMPLE_DELAY_MS);
    _collector.clear();

    ASSERT_TRUE(signaling::publish(signaling::ForcedRefreshStart{ firmware::sys::ForcedRefreshType::OscRequest }));
    drain();

    const auto signals = _collector.snapshot();

    EXPECT_EQ(count_signal<signaling::OscSensorProximitySignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorAmbientLightSignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorRgbSignal>(signals), 1U);
}

TEST_F(Apds9960SensorTest, RecoversAfterSensorDisconnectsAndReconnects)
{
    update_after(SAMPLE_DELAY_MS);
    _collector.clear();

    _hwa.connected = false;

    update_after(0);
    update_after(0);
    update_after(0);

    EXPECT_TRUE(_collector.snapshot().empty());

    _hwa.connected = true;
    set_sensor_values(24, 90, 483, 352, 338);

    ASSERT_TRUE(_sensor.init(0));
    update_after(SAMPLE_DELAY_MS);

    const auto signals = _collector.snapshot();

    EXPECT_EQ(count_signal<signaling::OscSensorProximitySignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorAmbientLightSignal>(signals), 1U);
    EXPECT_EQ(count_signal<signaling::OscSensorRgbSignal>(signals), 1U);
}
