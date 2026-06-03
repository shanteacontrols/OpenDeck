/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/database.h"

#include "firmware/src/database/builder/builder.h"
#include "firmware/src/io/i2c/hwa/test/hwa_test.h"
#include "firmware/src/io/i2c/peripherals/sensor_cap1188/instance/impl/sensor_cap1188.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/util/configurable/configurable.h"

#include "zlibs/utils/misc/mutex.h"
#include "zlibs/utils/signaling/signaling.h"

#include <variant>
#include <vector>

using namespace opendeck::firmware::io::i2c;
using namespace opendeck::firmware::io::i2c::sensor_cap1188;
using namespace opendeck::firmware;

namespace
{
    void configure_valid_cap1188_registers(HwaTest& hwa)
    {
        hwa.register_read_mode  = true;
        hwa.register_write_mode = true;
        hwa.set_u8(CAP1188_REGISTER_PRODUCT_ID, CAP1188_PRODUCT_ID);
        hwa.set_u8(CAP1188_REGISTER_MANUFACTURER_ID, CAP1188_MANUFACTURER_ID);
        hwa.set_u8(CAP1188_REGISTER_REVISION, 0x83);
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

        ~SensorCollector()
        {
            signaling::clear_registry();
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

        private:
        mutable zlibs::utils::misc::Mutex       _mutex;
        std::vector<signaling::OscSensorSignal> _signals = {};
    };

    const signaling::OscSensorTouchSignal* touch_payload(const signaling::OscSensorSignal& signal)
    {
        return std::get_if<signaling::OscSensorTouchSignal>(&signal.payload);
    }

    class Cap1188SensorTest : public ::testing::Test
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

        opendeck::tests::NoOpDatabaseHandlers _handlers;
        database::Builder                     _database_builder;
        database::Admin&                      _database_admin = _database_builder.instance();
        HwaTest                               _hwa;
        sensor_cap1188::Database              _database = sensor_cap1188::Database(_database_admin);
        SensorCap1188                         _sensor   = SensorCap1188(_hwa, _database);
    };
}    // namespace

TEST_F(Cap1188SensorTest, ExposesSupportedI2cAddresses)
{
    const auto addresses = _sensor.i2c_addresses();

    ASSERT_EQ(addresses.size(), I2C_ADDRESSES.size());

    for (size_t i = 0; i < I2C_ADDRESSES.size(); i++)
    {
        EXPECT_EQ(addresses[i], I2C_ADDRESSES[i]);
    }
}

TEST_F(Cap1188SensorTest, InitializesWhenProductAndManufacturerIdsMatch)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[1] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_cap1188_registers(_hwa);

    ASSERT_TRUE(_sensor.init(1));
    EXPECT_TRUE(_sensor.update());
    EXPECT_EQ(_hwa.registers[CAP1188_REGISTER_LED_LINKING], CAP1188_LED_LINK_ALL_INPUTS);
    EXPECT_EQ(_hwa.registers[CAP1188_REGISTER_SENSITIVITY], 0x20);
    EXPECT_EQ(_hwa.write_read_addresses.back(), I2C_ADDRESSES[1]);
}

TEST_F(Cap1188SensorTest, AppliesConfiguredSensitivityDuringInit)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[1] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_cap1188_registers(_hwa);
    _hwa.set_u8(CAP1188_REGISTER_SENSITIVITY, 0x8F);

    ASSERT_TRUE(_database.update(database::Config::Section::I2c::Cap1188, Setting::Sensitivity, Sensitivity::High));

    ASSERT_TRUE(_sensor.init(1));
    EXPECT_EQ(_hwa.registers[CAP1188_REGISTER_SENSITIVITY], 0x9F);

    ASSERT_TRUE(_database.update(database::Config::Section::I2c::Cap1188, Setting::Sensitivity, Sensitivity::Low));

    ASSERT_TRUE(_sensor.init(1));
    EXPECT_EQ(_hwa.registers[CAP1188_REGISTER_SENSITIVITY], 0xBF);
}

TEST_F(Cap1188SensorTest, RejectsUnexpectedProductId)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[1] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_cap1188_registers(_hwa);
    _hwa.set_u8(CAP1188_REGISTER_PRODUCT_ID, 0x00);

    EXPECT_FALSE(_sensor.init(1));
    EXPECT_FALSE(_sensor.update());
}

TEST_F(Cap1188SensorTest, RejectsUnexpectedManufacturerId)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[1] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_cap1188_registers(_hwa);
    _hwa.set_u8(CAP1188_REGISTER_MANUFACTURER_ID, 0x00);

    EXPECT_FALSE(_sensor.init(1));
    EXPECT_FALSE(_sensor.update());
}

TEST_F(Cap1188SensorTest, ClearsPendingInterruptDuringInit)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[1] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_cap1188_registers(_hwa);
    _hwa.set_u8(CAP1188_REGISTER_MAIN_CONTROL, CAP1188_MAIN_CONTROL_INTERRUPT);

    ASSERT_TRUE(_sensor.init(1));
    EXPECT_EQ(_hwa.registers[CAP1188_REGISTER_MAIN_CONTROL], 0);
}

TEST_F(Cap1188SensorTest, RejectsWhenLedLinkingFails)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[1] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_cap1188_registers(_hwa);

    _hwa.write_succeeds = false;

    EXPECT_FALSE(_sensor.init(1));
    EXPECT_FALSE(_sensor.update());
}

TEST_F(Cap1188SensorTest, ReadsSensorInputStatusDuringUpdate)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[1] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_cap1188_registers(_hwa);
    _hwa.set_u8(CAP1188_REGISTER_SENSOR_INPUT, 0x05);
    _hwa.set_u8(CAP1188_REGISTER_MAIN_CONTROL, CAP1188_MAIN_CONTROL_INTERRUPT);

    ASSERT_TRUE(_sensor.init(1));

    _hwa.write_read_buffers.clear();

    EXPECT_TRUE(_sensor.update());

    ASSERT_GE(_hwa.write_read_buffers.size(), 2U);
    EXPECT_EQ(_hwa.write_read_buffers[0], (std::vector<uint8_t>{ CAP1188_REGISTER_SENSOR_INPUT }));
    EXPECT_EQ(_hwa.write_read_buffers[1], (std::vector<uint8_t>{ CAP1188_REGISTER_MAIN_CONTROL }));
    EXPECT_EQ(_hwa.registers[CAP1188_REGISTER_MAIN_CONTROL], 0);
}

TEST_F(Cap1188SensorTest, PublishesTouchChangesDuringUpdate)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[1] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_cap1188_registers(_hwa);

    SensorCollector collector;

    ASSERT_TRUE(_sensor.init(1));
    ASSERT_TRUE(zlibs::utils::signaling::drain());
    collector.clear();

    _hwa.set_u8(CAP1188_REGISTER_SENSOR_INPUT, 0x05);

    ASSERT_TRUE(_sensor.update());
    ASSERT_TRUE(zlibs::utils::signaling::drain());

    auto signals = collector.snapshot();
    ASSERT_EQ(signals.size(), 2U);

    const auto* touch0 = touch_payload(signals[0]);
    const auto* touch2 = touch_payload(signals[1]);

    ASSERT_NE(touch0, nullptr);
    ASSERT_NE(touch2, nullptr);
    EXPECT_EQ(touch0->index, 0U);
    EXPECT_EQ(touch0->value, 1);
    EXPECT_EQ(touch2->index, 2U);
    EXPECT_EQ(touch2->value, 1);

    _hwa.set_u8(CAP1188_REGISTER_SENSOR_INPUT, 0x04);

    ASSERT_TRUE(_sensor.update());
    ASSERT_TRUE(zlibs::utils::signaling::drain());

    signals = collector.snapshot();
    ASSERT_EQ(signals.size(), 3U);

    const auto* release0 = touch_payload(signals[2]);

    ASSERT_NE(release0, nullptr);
    EXPECT_EQ(release0->index, 0U);
    EXPECT_EQ(release0->value, 0);
}

TEST_F(Cap1188SensorTest, PublishesAllTouchStatesOnForcedRefreshStart)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[1] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_cap1188_registers(_hwa);

    SensorCollector collector;

    ASSERT_TRUE(_sensor.init(1));
    ASSERT_TRUE(zlibs::utils::signaling::drain());
    collector.clear();

    _hwa.set_u8(CAP1188_REGISTER_SENSOR_INPUT, 0x05);

    ASSERT_TRUE(_sensor.update());
    ASSERT_TRUE(zlibs::utils::signaling::drain());
    collector.clear();

    ASSERT_TRUE(signaling::publish(signaling::ForcedRefreshStart{ sys::ForcedRefreshType::OscRequest }));
    ASSERT_TRUE(zlibs::utils::signaling::drain());

    const auto signals = collector.snapshot();
    ASSERT_EQ(signals.size(), static_cast<size_t>(CAP1188_SENSOR_INPUT_COUNT));

    for (size_t index = 0; index < signals.size(); index++)
    {
        const auto* touch = touch_payload(signals[index]);

        ASSERT_NE(touch, nullptr);
        EXPECT_EQ(touch->index, index);
        EXPECT_EQ(touch->value, (index == 0U) || (index == 2U) ? 1 : 0);
    }
}

TEST_F(Cap1188SensorTest, UpdateFailsWhenSensorInputReadFails)
{
    _hwa.set_available_addresses({ I2C_ADDRESSES[1] });
    _hwa.require_available_address_for_transfers = true;
    configure_valid_cap1188_registers(_hwa);

    ASSERT_TRUE(_sensor.init(1));

    _hwa.write_read_succeeds = false;

    EXPECT_FALSE(_sensor.update());
}
