/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/database.h"
#include "tests/shared/helpers/midi.h"
#include "tests/shared/helpers/misc.h"
#include "firmware/src/database/layout.h"
#include "firmware/src/io/analog/shared/common.h"
#include "firmware/src/io/digital/encoders/shared/common.h"
#include "firmware/src/io/digital/switches/shared/common.h"
#include "firmware/src/io/i2c/peripherals/display/shared/common.h"
#include "firmware/src/io/i2c/peripherals/sensor_apds9960/shared/common.h"
#include "firmware/src/io/i2c/peripherals/sensor_vl53l4cx/shared/common.h"
#include "firmware/src/io/outputs/shared/common.h"
#include "firmware/src/io/touchscreen/shared/common.h"
#include "firmware/src/protocol/midi/shared/common.h"
#include "firmware/src/protocol/osc/shared/common.h"
#include "firmware/src/system/builder/builder.h"
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_TRAFFIC_INDICATORS
#include "common/src/io/indicators/hwa/test/hwa_test.h"
#include "firmware/src/io/indicators/instance/impl/indicators.h"
#endif
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/util/configurable/configurable.h"

#include <atomic>

using namespace opendeck::io;
using namespace opendeck::protocol;
using namespace opendeck::firmware;

namespace
{
    LOG_MODULE_REGISTER(system_test, LOG_LEVEL_INF);

    std::vector<uint8_t> handshake_req = { 0xF0, 0x00, 0x53, 0x43, 0x00, 0x00, 0x01, 0xF7 };
    std::vector<uint8_t> handshake_ack = { 0xF0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x01, 0xF7 };

    class SystemTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_MDNS
            _system._hwa._builder_mdns._hwa.ip_address_value.clear();
#endif
        }

        void TearDown() override
        {
            io::Base::resume();
            protocol::Base::resume();
            ConfigHandler.clear();
            signaling::clear_registry();
        }

        int supported_presets()
        {
            LOG_INF("Checking the number of supported presets");

            auto response = _helper.send_raw_sysex_to_stub(std::vector<uint8_t>({ 0xF0,
                                                                                  0x00,
                                                                                  0x53,
                                                                                  0x43,
                                                                                  0x00,
                                                                                  0x00,
                                                                                  SYSEX_CR_SUPPORTED_PRESETS,
                                                                                  0xF7 }));

            return response.at(8);
        }

        void handshake()
        {
            LOG_INF("Sending handshake");

            auto response = _helper.send_raw_sysex_to_stub(handshake_req);
            ASSERT_EQ(handshake_ack, response);
        }

        void unlock_config()
        {
            const auto token = sys::make_config_unlock_token(_system._hwa.serial);

            for (size_t i = 0; i < token.size(); i++)
            {
                ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Global::ConfigUnlock,
                                                                       i,
                                                                       token.at(i)));
            }
        }

        void init_initialized_system()
        {
            ASSERT_TRUE(_system._hwa.database().init(_database_handlers));
            ASSERT_TRUE(_system._instance.init());
            handshake();
            unlock_config();
        }

        sys::Builder                _system;
        tests::MIDIHelper           _helper = tests::MIDIHelper(_system);
        tests::NoOpDatabaseHandlers _database_handlers;
    };
}    // namespace

TEST_F(SystemTest, FirstBootDatabaseInitializationSkipsComponentInitialization)
{
    size_t init_complete_cnt = 0;
    size_t program_msg_cnt   = 0;

    signaling::subscribe<signaling::SystemSignal>(
        [&](const signaling::SystemSignal& event)
        {
            if (event.system_event == signaling::SystemEvent::InitComplete)
            {
                init_complete_cnt++;
            }
        });

    signaling::subscribe<signaling::InternalProgram>(
        [&](const signaling::InternalProgram& signal)
        {
            program_msg_cnt++;
        });

    ASSERT_TRUE(_system._instance.init());

    k_msleep(sys::INITIAL_IO_RESUME_DELAY_MS);

    ASSERT_EQ(0, init_complete_cnt);
    ASSERT_EQ(0, program_msg_cnt);
}

TEST_F(SystemTest, FullDatabaseInitialValues)
{
    init_initialized_system();

    auto& db = _system._hwa.database();

    const auto expected_signature = static_cast<uint16_t>(database::AppLayout::common_uid() ^
                                                          database::AppLayout::preset_uid() ^
                                                          static_cast<uint16_t>(OPENDECK_TARGET_UID) ^
                                                          static_cast<uint16_t>(db.supported_presets()));

    auto verify = [&](uint32_t expected, auto section, auto index)
    {
        uint32_t actual = 0;

        ASSERT_TRUE(db.read(section, index, actual));
        ASSERT_EQ(expected, actual);
    };

    auto verify_common = [&](uint32_t expected, auto index)
    {
        verify(expected, database::Config::Section::Common::CommonSettings, index);
    };

    for (int preset = 0; preset < db.supported_presets(); preset++)
    {
        ASSERT_TRUE(db.set_preset(preset));

        verify_common(preset, database::Config::CommonSetting::ActivePreset);
        verify_common(0, database::Config::CommonSetting::PresetPreserve);

        for (int i = static_cast<int>(database::Config::CommonSetting::CustomCommonSettingStart);
             i < static_cast<int>(database::Config::CommonSetting::CustomCommonSettingEnd);
             i++)
        {
            verify_common(0, i);
        }

        verify_common(expected_signature, database::Config::CommonSetting::Uid);

        for (int i = 0; i < static_cast<uint8_t>(protocol::midi::Setting::Count); i++)
        {
            const auto expected = (i == static_cast<int>(protocol::midi::Setting::GlobalChannel)) ? 1U : 0U;

            verify(expected, database::Config::Section::Global::MidiSettings, i);
        }

        for (int i = 0; i < static_cast<uint8_t>(protocol::osc::Setting::Count); i++)
        {
            uint32_t expected = 0;

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OSC
            if (i == static_cast<int>(protocol::osc::Setting::DestPort))
            {
                expected = protocol::osc::DEFAULT_DEST_PORT;
            }
            else if (i == static_cast<int>(protocol::osc::Setting::ListenPort))
            {
                expected = protocol::osc::DEFAULT_LISTEN_PORT;
            }
#endif

            verify(expected, database::Config::Section::Global::OscSettings, i);
        }

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_SWITCHES
        for (size_t i = 0; i < io::switches::Collection::size(); i++)
        {
            verify(0, database::Config::Section::Switch::Type, i);
            verify(0, database::Config::Section::Switch::MessageType, i);
            verify(127, database::Config::Section::Switch::Value, i);
            verify(1, database::Config::Section::Switch::Channel, i);
        }

        for (size_t group = 0; group < io::switches::Collection::groups(); group++)
        {
            for (size_t i = 0; i < io::switches::Collection::size(group); i++)
            {
                verify(i, database::Config::Section::Switch::MidiId, i + io::switches::Collection::start_index(group));
            }
        }
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS
        for (size_t i = 0; i < io::encoders::Collection::size(); i++)
        {
            verify(0, database::Config::Section::Encoder::Enable, i);
            verify(0, database::Config::Section::Encoder::Invert, i);
            verify(0, database::Config::Section::Encoder::Mode, i);
            verify(i, database::Config::Section::Encoder::MidiId1, i);
            verify(1, database::Config::Section::Encoder::Channel, i);
            verify(0, database::Config::Section::Encoder::LowerLimit, i);
            verify(16383, database::Config::Section::Encoder::UpperLimit, i);
            verify(127, database::Config::Section::Encoder::RepeatedValue, i);
            verify(i, database::Config::Section::Encoder::MidiId2, i);
        }
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ADC
        for (size_t i = 0; i < io::analog::Collection::size(); i++)
        {
            verify(0, database::Config::Section::Analog::Enable, i);
            verify(0, database::Config::Section::Analog::Invert, i);
            verify(0, database::Config::Section::Analog::Type, i);
            verify(0, database::Config::Section::Analog::LowerLimit, i);
            verify(16383, database::Config::Section::Analog::UpperLimit, i);
            verify(1, database::Config::Section::Analog::Channel, i);
            verify(0, database::Config::Section::Analog::LowerOffset, i);
            verify(0, database::Config::Section::Analog::UpperOffset, i);
        }

        for (size_t group = 0; group < io::analog::Collection::groups(); group++)
        {
            for (size_t i = 0; i < io::analog::Collection::size(group); i++)
            {
                verify(i, database::Config::Section::Analog::MidiId, i + io::analog::Collection::start_index(group));
            }
        }
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OUTPUTS
        for (int i = 0; i < static_cast<uint8_t>(io::outputs::Setting::Count); i++)
        {
            verify(0, database::Config::Section::Outputs::Global, i);
        }

        for (size_t group = 0; group < io::outputs::Collection::groups(); group++)
        {
            for (size_t i = 0; i < io::outputs::Collection::size(group); i++)
            {
                const auto index = i + io::outputs::Collection::start_index(group);

                verify(i, database::Config::Section::Outputs::ActivationId, index);
                verify(static_cast<uint32_t>(io::outputs::ControlType::MidiInNoteMultiVal),
                       database::Config::Section::Outputs::ControlType,
                       index);
            }
        }

        for (size_t i = 0; i < io::outputs::Collection::size(); i++)
        {
            verify(127, database::Config::Section::Outputs::ActivationValue, i);
            verify(1, database::Config::Section::Outputs::Channel, i);
        }
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_I2C
        for (int i = 0; i < static_cast<uint8_t>(io::i2c::display::Setting::Count); i++)
        {
            uint32_t expected = 0;

            if (i == static_cast<int>(io::i2c::display::Setting::Controller))
            {
                expected = static_cast<uint32_t>(io::i2c::display::DisplayController::Ssd1306);
            }
            else if (i == static_cast<int>(io::i2c::display::Setting::Resolution))
            {
                expected = static_cast<uint32_t>(io::i2c::display::DisplayResolution::Res128x64);
            }

            verify(expected, database::Config::Section::I2c::Display, i);
        }

        for (int i = 0; i < static_cast<uint8_t>(io::i2c::sensor_apds9960::Setting::Count); i++)
        {
            uint32_t expected = 0;

            if (i == static_cast<int>(io::i2c::sensor_apds9960::Setting::ProximityGain))
            {
                expected = 2;
            }
            else if (i == static_cast<int>(io::i2c::sensor_apds9960::Setting::AlsGain))
            {
                expected = 1;
            }

            verify(expected, database::Config::Section::I2c::Apds9960, i);
        }

        for (int i = 0; i < static_cast<uint8_t>(io::i2c::sensor_vl53l4cx::Setting::Count); i++)
        {
            uint32_t expected = 0;

            if (i == static_cast<int>(io::i2c::sensor_vl53l4cx::Setting::TrackingArea))
            {
                expected = static_cast<uint32_t>(io::i2c::sensor_vl53l4cx::TrackingArea::Narrow);
            }
            else if (i == static_cast<int>(io::i2c::sensor_vl53l4cx::Setting::Response))
            {
                expected = static_cast<uint32_t>(io::i2c::sensor_vl53l4cx::Response::Stable);
            }
            else if (i == static_cast<int>(io::i2c::sensor_vl53l4cx::Setting::DistanceMode))
            {
                expected = static_cast<uint32_t>(io::i2c::sensor_vl53l4cx::DistanceMode::Medium);
            }

            verify(expected, database::Config::Section::I2c::Vl53l4cx, i);
        }
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_TOUCHSCREEN
        for (int i = 0; i < static_cast<uint8_t>(io::touchscreen::Setting::Count); i++)
        {
            verify(0, database::Config::Section::Touchscreen::Setting, i);
        }

        for (size_t i = 0; i < io::touchscreen::Collection::size(); i++)
        {
            verify(0, database::Config::Section::Touchscreen::XPos, i);
            verify(0, database::Config::Section::Touchscreen::YPos, i);
            verify(0, database::Config::Section::Touchscreen::Width, i);
            verify(0, database::Config::Section::Touchscreen::Height, i);
            verify(0, database::Config::Section::Touchscreen::OnScreen, i);
            verify(0, database::Config::Section::Touchscreen::OffScreen, i);
            verify(0, database::Config::Section::Touchscreen::PageSwitchEnabled, i);
            verify(0, database::Config::Section::Touchscreen::PageSwitchIndex, i);
        }
#endif
    }
}

TEST_F(SystemTest, ForcedResendOnPresetChange)
{
    constexpr uint32_t END_WAIT_TIMEOUT_MS           = 1000;
    constexpr size_t   PRESET_CHANGE_LOOP_ITERATIONS = 5;
    size_t             start_received_cnt            = 0;
    size_t             stop_received_cnt             = 0;
    size_t             preset_change_received_cnt    = 0;

    signaling::subscribe<signaling::ForcedRefreshStart>(
        [&](const signaling::ForcedRefreshStart& event)
        {
            start_received_cnt++;
        });

    signaling::subscribe<signaling::ForcedRefreshStop>(
        [&](const signaling::ForcedRefreshStop& event)
        {
            stop_received_cnt++;
        });

    signaling::subscribe<signaling::SystemSignal>(
        [&](const signaling::SystemSignal& event)
        {
            if (event.system_event == signaling::SystemEvent::PresetChanged)
            {
                preset_change_received_cnt++;
            }
        });

    init_initialized_system();

    auto presets = supported_presets();

    if (presets < 2)
    {
        LOG_WRN("Not enough supported presets for further tests, exiting");
        return;
    }

    ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Global::SystemSettings,
                                                           sys::Config::SystemSetting::ActivePreset,
                                                           1));

    // preset change message is published immediately
    ASSERT_EQ(1, preset_change_received_cnt);

    k_msleep(sys::PRESET_CHANGE_FORCED_REFRESH_DELAY);

    ASSERT_EQ(1, start_received_cnt);

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return stop_received_cnt == 1;
        },
        END_WAIT_TIMEOUT_MS));

    // now test rapid preset change - PRESET_CHANGE_FORCED_REFRESH_DELAY should block it

    for (size_t i = 0; i < PRESET_CHANGE_LOOP_ITERATIONS; i++)
    {
        auto new_preset = i % 2 ? 0 : 1;

        ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Global::SystemSettings,
                                                               sys::Config::SystemSetting::ActivePreset,
                                                               new_preset));
    }

    k_msleep(sys::PRESET_CHANGE_FORCED_REFRESH_DELAY);
    ASSERT_EQ(2, start_received_cnt);

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return stop_received_cnt == 2;
        },
        END_WAIT_TIMEOUT_MS));
}

TEST_F(SystemTest, ForcedResendOnNetworkIdentity)
{
    constexpr uint32_t     END_WAIT_TIMEOUT_MS = 1000;
    size_t                 start_received_cnt  = 0;
    size_t                 stop_received_cnt   = 0;
    sys::ForcedRefreshType refresh_type        = {};

    signaling::subscribe<signaling::ForcedRefreshStart>(
        [&](const signaling::ForcedRefreshStart& event)
        {
            start_received_cnt++;
            refresh_type = event.type;
        });

    signaling::subscribe<signaling::ForcedRefreshStop>(
        [&](const signaling::ForcedRefreshStop& event)
        {
            stop_received_cnt++;
            refresh_type = event.type;
        });

    init_initialized_system();

    signaling::publish(signaling::NetworkIdentitySignal("opendeck-test.local", "192.168.1.112"));

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return start_received_cnt == 1;
        },
        sys::NETWORK_CHANGE_FORCED_REFRESH_DELAY + END_WAIT_TIMEOUT_MS));

    ASSERT_EQ(sys::ForcedRefreshType::NetworkInit, refresh_type);

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return stop_received_cnt == 1;
        },
        END_WAIT_TIMEOUT_MS));
}

TEST_F(SystemTest, NetworkIdentityWithoutAddressDoesNotForceRefresh)
{
    size_t start_received_cnt = 0;

    signaling::subscribe<signaling::ForcedRefreshStart>(
        [&](const signaling::ForcedRefreshStart& event)
        {
            if (event.type == sys::ForcedRefreshType::NetworkInit)
            {
                start_received_cnt++;
            }
        });

    init_initialized_system();

    signaling::publish(signaling::NetworkIdentitySignal("opendeck-test.local", {}));

    k_msleep(sys::NETWORK_CHANGE_FORCED_REFRESH_DELAY + 100);

    ASSERT_EQ(0, start_received_cnt);
}

TEST_F(SystemTest, ForcedResendOnOscRefreshRequest)
{
    constexpr uint32_t     END_WAIT_TIMEOUT_MS = 1000;
    size_t                 start_received_cnt  = 0;
    size_t                 stop_received_cnt   = 0;
    sys::ForcedRefreshType refresh_type        = {};

    signaling::subscribe<signaling::ForcedRefreshStart>(
        [&](const signaling::ForcedRefreshStart& event)
        {
            start_received_cnt++;
            refresh_type = event.type;
        });

    signaling::subscribe<signaling::ForcedRefreshStop>(
        [&](const signaling::ForcedRefreshStop& event)
        {
            stop_received_cnt++;
            refresh_type = event.type;
        });

    init_initialized_system();

    signaling::publish(signaling::SystemSignal{
        .system_event = signaling::SystemEvent::OscRefreshReq,
    });

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return start_received_cnt == 1;
        },
        END_WAIT_TIMEOUT_MS));

    ASSERT_EQ(sys::ForcedRefreshType::OscRequest, refresh_type);

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return stop_received_cnt == 1;
        },
        END_WAIT_TIMEOUT_MS));
}

TEST_F(SystemTest, ProgramIndicatedOnStartup)
{
    size_t program_msg_cnt = 0;

    signaling::subscribe<signaling::InternalProgram>(
        [&](const signaling::InternalProgram& signal)
        {
            program_msg_cnt++;
        });

    init_initialized_system();
    k_msleep(sys::INITIAL_IO_RESUME_DELAY_MS);

    ASSERT_EQ(16, program_msg_cnt);
}

TEST_F(SystemTest, SerialNumberCustomRequestReturnsHwaSerial)
{
    init_initialized_system();

    const auto response = _helper.send_raw_sysex_to_stub(std::vector<uint8_t>({ 0xF0,
                                                                                0x00,
                                                                                0x53,
                                                                                0x43,
                                                                                0x00,
                                                                                0x00,
                                                                                SYSEX_CR_SERIAL_NUM,
                                                                                0xF7 }));

    ASSERT_EQ(7 + (_system._hwa.serial.size() * 2) + 1, response.size());
    ASSERT_EQ(0xF0, response.at(0));
    ASSERT_EQ(0x00, response.at(1));
    ASSERT_EQ(0x53, response.at(2));
    ASSERT_EQ(0x43, response.at(3));
    ASSERT_EQ(static_cast<uint8_t>(zlibs::utils::sysex_conf::Status::Ack), response.at(4));
    ASSERT_EQ(0x00, response.at(5));
    ASSERT_EQ(SYSEX_CR_SERIAL_NUM, response.at(6));
    ASSERT_EQ(0xF7, response.back());

    for (size_t i = 0; i < _system._hwa.serial.size(); i++)
    {
        const auto response_index = 7 + (i * 2);
        EXPECT_EQ(0, response.at(response_index));
        EXPECT_EQ(_system._hwa.serial.at(i), response.at(response_index + 1));
    }
}

TEST_F(SystemTest, ConfigurationWritesRequireUnlockToken)
{
    ASSERT_TRUE(_system._hwa.database().init(_database_handlers));
    ASSERT_TRUE(_system._instance.init());
    handshake();

    ASSERT_FALSE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Global::SystemSettings,
                                                            sys::Config::SystemSetting::DisableForcedRefreshAfterPresetChange,
                                                            1));

    unlock_config();

    ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Global::SystemSettings,
                                                           sys::Config::SystemSetting::DisableForcedRefreshAfterPresetChange,
                                                           1));
}

TEST_F(SystemTest, ConfigurationSessionTimesOutAfterInactivity)
{
    std::atomic_size_t opened_cnt = 0;
    std::atomic_size_t closed_cnt = 0;

    signaling::subscribe<signaling::SystemSignal>(
        [&](const signaling::SystemSignal& event)
        {
            switch (event.system_event)
            {
            case signaling::SystemEvent::ConfigurationSessionOpened:
            {
                opened_cnt++;
            }
            break;

            case signaling::SystemEvent::ConfigurationSessionClosed:
            {
                closed_cnt++;
            }
            break;

            default:
                break;
            }
        });

    init_initialized_system();

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return opened_cnt.load() == 1;
        }));

    ASSERT_EQ(0, closed_cnt.load());

    k_msleep(sys::SYSEX_CONFIGURATION_TIMEOUT_MS / 2);
    handshake();
    k_msleep((sys::SYSEX_CONFIGURATION_TIMEOUT_MS / 2) + 100);

    ASSERT_EQ(0, closed_cnt.load());

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return closed_cnt.load() == 1;
        },
        sys::SYSEX_CONFIGURATION_TIMEOUT_MS));

    auto response = _helper.send_raw_sysex_to_stub(_helper.generate_sysex_get_req(sys::Config::Section::Global::SystemSettings,
                                                                                  sys::Config::SystemSetting::ActivePreset));

    ASSERT_FALSE(response.empty());
    ASSERT_EQ(static_cast<uint8_t>(zlibs::utils::sysex_conf::Status::ErrorConnection), response.at(4));
}

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_TRAFFIC_INDICATORS
TEST_F(SystemTest, IndicatorsInvertWhileConfigurationSessionIsOpen)
{
    opendeck::common::io::indicators::HwaTest hwa;
    io::indicators::Indicators                indicators(hwa);

    ASSERT_TRUE(indicators.init());
    ASSERT_FALSE(hwa.is_on(opendeck::common::io::indicators::Type::UsbIn));
    ASSERT_FALSE(hwa.is_on(opendeck::common::io::indicators::Type::UsbOut));

    signaling::SystemSignal open_signal = {};
    open_signal.system_event            = signaling::SystemEvent::ConfigurationSessionOpened;
    signaling::publish(open_signal);

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return hwa.is_on(opendeck::common::io::indicators::Type::UsbIn) &&
                   hwa.is_on(opendeck::common::io::indicators::Type::UsbOut);
        }));

    signaling::TrafficSignal traffic_signal = {};
    traffic_signal.transport                = signaling::TrafficTransport::Usb;
    traffic_signal.direction                = signaling::SignalDirection::In;
    signaling::publish(traffic_signal);

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return !hwa.is_on(opendeck::common::io::indicators::Type::UsbIn);
        }));

    k_msleep(io::indicators::TRAFFIC_INDICATOR_TIMEOUT_MS + 20);
    ASSERT_TRUE(hwa.is_on(opendeck::common::io::indicators::Type::UsbIn));

    signaling::SystemSignal close_signal = {};
    close_signal.system_event            = signaling::SystemEvent::ConfigurationSessionClosed;
    signaling::publish(close_signal);

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return !hwa.is_on(opendeck::common::io::indicators::Type::UsbIn) &&
                   !hwa.is_on(opendeck::common::io::indicators::Type::UsbOut);
        }));
}
#endif

TEST_F(SystemTest, ConfigurationSessionTimeoutDoesNotCloseBackup)
{
    bool   backup_active             = false;
    bool   closed_during_backup      = false;
    size_t backup_start_received_cnt = 0;
    size_t backup_end_received_cnt   = 0;

    signaling::subscribe<signaling::SystemSignal>(
        [&](const signaling::SystemSignal& event)
        {
            switch (event.system_event)
            {
            case signaling::SystemEvent::BackupStart:
            {
                backup_active = true;
                backup_start_received_cnt++;
            }
            break;

            case signaling::SystemEvent::BackupEnd:
            {
                backup_active = false;
                backup_end_received_cnt++;
            }
            break;

            case signaling::SystemEvent::ConfigurationSessionClosed:
            {
                if (backup_active)
                {
                    closed_during_backup = true;
                }
            }
            break;

            default:
                break;
            }
        });

    init_initialized_system();

    auto response = _helper.send_raw_sysex_to_stub(std::vector<uint8_t>({ 0xF0,
                                                                          0x00,
                                                                          0x53,
                                                                          0x43,
                                                                          0x00,
                                                                          0x00,
                                                                          SYSEX_CR_FULL_BACKUP,
                                                                          0xF7 }));

    ASSERT_FALSE(response.empty());

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return backup_start_received_cnt == 1;
        }));

    k_msleep(sys::SYSEX_CONFIGURATION_TIMEOUT_MS + 100);

    ASSERT_FALSE(closed_during_backup);

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return backup_end_received_cnt == 1;
        },
        10000));

    ASSERT_FALSE(closed_during_backup);
}

TEST_F(SystemTest, WebSocketsDisconnectClosesConfigurationSession)
{
    std::atomic_size_t opened_cnt = 0;
    std::atomic_size_t closed_cnt = 0;

    signaling::subscribe<signaling::SystemSignal>(
        [&](const signaling::SystemSignal& event)
        {
            switch (event.system_event)
            {
            case signaling::SystemEvent::ConfigurationSessionOpened:
                opened_cnt++;
                break;

            case signaling::SystemEvent::ConfigurationSessionClosed:
                closed_cnt++;
                break;

            default:
                break;
            }
        });

    ASSERT_TRUE(_system._hwa.database().init(_database_handlers));
    ASSERT_TRUE(_system._instance.init());

    ASSERT_TRUE(signaling::publish(signaling::ConfigRequestSignal(signaling::ConfigTransport::WebSockets, handshake_req, 1)));
    ASSERT_TRUE(zlibs::utils::signaling::drain());

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return opened_cnt.load() == 1;
        }));

    ASSERT_TRUE(signaling::publish(signaling::ConfigDisconnectSignal{
        .transport  = signaling::ConfigTransport::WebSockets,
        .session_id = 1,
    }));
    ASSERT_TRUE(zlibs::utils::signaling::drain());

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return closed_cnt.load() == 1;
        }));
}

TEST_F(SystemTest, StaleWebSocketsDisconnectDoesNotCloseNewerConfigurationSession)
{
    std::atomic_size_t opened_cnt = 0;
    std::atomic_size_t closed_cnt = 0;

    signaling::subscribe<signaling::SystemSignal>(
        [&](const signaling::SystemSignal& event)
        {
            switch (event.system_event)
            {
            case signaling::SystemEvent::ConfigurationSessionOpened:
                opened_cnt++;
                break;

            case signaling::SystemEvent::ConfigurationSessionClosed:
                closed_cnt++;
                break;

            default:
                break;
            }
        });

    ASSERT_TRUE(_system._hwa.database().init(_database_handlers));
    ASSERT_TRUE(_system._instance.init());

    ASSERT_TRUE(signaling::publish(signaling::ConfigRequestSignal(signaling::ConfigTransport::WebSockets, handshake_req, 1)));
    ASSERT_TRUE(zlibs::utils::signaling::drain());

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return opened_cnt.load() == 1;
        }));

    ASSERT_TRUE(signaling::publish(signaling::ConfigRequestSignal(signaling::ConfigTransport::WebSockets, handshake_req, 2)));
    ASSERT_TRUE(zlibs::utils::signaling::drain());

    ASSERT_TRUE(signaling::publish(signaling::ConfigDisconnectSignal{
        .transport  = signaling::ConfigTransport::WebSockets,
        .session_id = 1,
    }));
    ASSERT_TRUE(zlibs::utils::signaling::drain());

    EXPECT_EQ(closed_cnt.load(), 0U);

    ASSERT_TRUE(signaling::publish(signaling::ConfigDisconnectSignal{
        .transport  = signaling::ConfigTransport::WebSockets,
        .session_id = 2,
    }));
    ASSERT_TRUE(zlibs::utils::signaling::drain());

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return closed_cnt.load() == 1;
        }));
}

TEST_F(SystemTest, FactoryResetRequestIsDeferred)
{
    size_t factory_reset_start_cnt = 0;

    signaling::subscribe<signaling::SystemSignal>(
        [&](const signaling::SystemSignal& event)
        {
            if (event.system_event == signaling::SystemEvent::FactoryResetStart)
            {
                factory_reset_start_cnt++;
            }
        });

    init_initialized_system();

    auto response = _helper.send_raw_sysex_to_stub(std::vector<uint8_t>({ 0xF0,
                                                                          0x00,
                                                                          0x53,
                                                                          0x43,
                                                                          0x00,
                                                                          0x00,
                                                                          SYSEX_CR_FACTORY_RESET,
                                                                          0xF7 }));

    ASSERT_FALSE(response.empty());
    ASSERT_EQ(0, factory_reset_start_cnt);

    k_msleep(sys::FACTORY_RESET_DELAY_MS / 2);
    ASSERT_EQ(0, factory_reset_start_cnt);

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return factory_reset_start_cnt == 1;
        },
        sys::FACTORY_RESET_DELAY_MS));
}

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI
TEST_F(SystemTest, UsbThruDin)
{
    init_initialized_system();

    // enable both din midi and usb to din thru

    EXPECT_CALL(_system._hwa._builder_midi._hwaSerial, init())
        .WillOnce(Return(true));

    ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Global::MidiSettings,
                                                           protocol::midi::Setting::DinEnabled,
                                                           1));

    ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Global::MidiSettings,
                                                           protocol::midi::Setting::UsbThruDin,
                                                           1));

    // generate incoming USB message

    tests::MIDIInputEvent event = {};
    event.channel               = 1;
    event.index                 = 0;
    event.value                 = 127;
    event.message               = midi::MessageType::ControlChange;

    _helper.process_incoming(event);

    ASSERT_EQ(1, _system._hwa._builder_midi._hwaSerial._writeParser.total_written_channel_messages());

    _helper.process_incoming(event);

    ASSERT_EQ(1, _system._hwa._builder_midi._hwaSerial._writeParser.total_written_channel_messages());
}
#endif
