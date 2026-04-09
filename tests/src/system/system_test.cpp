/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/database.h"
#include "tests/helpers/midi.h"
#include "system/builder.h"
#include "messaging/messaging.h"
#include "util/configurable/configurable.h"

using namespace io;
using namespace protocol;

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
        }

        void TearDown() override
        {
            // Signaling delivery is asynchronous; let any in-flight callbacks
            // finish before clearing registries and destroying subscriptions.
            k_msleep(50);
            io::Base::resume();
            protocol::Base::resume();
            ConfigHandler.clear();
            messaging::clear_registry();
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

        void init_initialized_system()
        {
            ASSERT_TRUE(_system._hwa.database().init(_database_handlers));
            ASSERT_TRUE(_system._instance.init());
            handshake();
        }

        sys::Builder                _system;
        test::MIDIHelper            _helper = test::MIDIHelper(_system);
        tests::NoOpDatabaseHandlers _database_handlers;
    };
}    // namespace

TEST_F(SystemTest, FirstBootDatabaseInitializationSkipsComponentInitialization)
{
    size_t init_complete_cnt = 0;
    size_t program_msg_cnt   = 0;

    messaging::subscribe<messaging::SystemSignal>(
        [&](const messaging::SystemSignal& event)
        {
            if (event.system_message == messaging::SystemMessage::InitComplete)
            {
                init_complete_cnt++;
            }
        });

    messaging::subscribe<messaging::InternalProgram>(
        [&](const messaging::InternalProgram& signal)
        {
            program_msg_cnt++;
        });

    ASSERT_TRUE(_system._instance.init());

    k_msleep(sys::INITIAL_IO_RESUME_DELAY_MS);

    ASSERT_EQ(0, init_complete_cnt);
    ASSERT_EQ(0, program_msg_cnt);
}

TEST_F(SystemTest, ForcedResendOnPresetChange)
{
    constexpr uint32_t END_WAIT_TIMEOUT_MS           = 1000;
    constexpr size_t   PRESET_CHANGE_LOOP_ITERATIONS = 5;
    size_t             start_received_cnt            = 0;
    size_t             stop_received_cnt             = 0;
    size_t             preset_change_received_cnt    = 0;

    messaging::subscribe<messaging::ForcedRefreshStart>(
        [&](const messaging::ForcedRefreshStart& event)
        {
            start_received_cnt++;
        });

    messaging::subscribe<messaging::ForcedRefreshStop>(
        [&](const messaging::ForcedRefreshStop& event)
        {
            stop_received_cnt++;
        });

    messaging::subscribe<messaging::SystemSignal>(
        [&](const messaging::SystemSignal& event)
        {
            if (event.system_message == messaging::SystemMessage::PresetChanged)
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

TEST_F(SystemTest, ProgramIndicatedOnStartup)
{
    size_t program_msg_cnt = 0;

    messaging::subscribe<messaging::InternalProgram>(
        [&](const messaging::InternalProgram& signal)
        {
            program_msg_cnt++;
        });

    init_initialized_system();
    k_msleep(sys::INITIAL_IO_RESUME_DELAY_MS);

    ASSERT_EQ(16, program_msg_cnt);
}

TEST_F(SystemTest, ConfigurationSessionTimesOutAfterInactivity)
{
    size_t opened_cnt = 0;
    size_t closed_cnt = 0;

    messaging::subscribe<messaging::SystemSignal>(
        [&](const messaging::SystemSignal& event)
        {
            switch (event.system_message)
            {
            case messaging::SystemMessage::ConfigurationSessionOpened:
                opened_cnt++;
                break;

            case messaging::SystemMessage::ConfigurationSessionClosed:
                closed_cnt++;
                break;

            default:
                break;
            }
        });

    init_initialized_system();

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return opened_cnt == 1;
        }));

    ASSERT_EQ(0, closed_cnt);

    k_msleep(sys::SYSEX_CONFIGURATION_TIMEOUT_MS / 2);
    handshake();
    k_msleep((sys::SYSEX_CONFIGURATION_TIMEOUT_MS / 2) + 100);

    ASSERT_EQ(0, closed_cnt);

    ASSERT_TRUE(tests::wait_until(
        [&]()
        {
            return closed_cnt == 1;
        },
        sys::SYSEX_CONFIGURATION_TIMEOUT_MS));

    auto response = _helper.send_raw_sysex_to_stub(_helper.generate_sysex_get_req(sys::Config::Section::Global::SystemSettings,
                                                                                  sys::Config::SystemSetting::ActivePreset));

    ASSERT_FALSE(response.empty());
    ASSERT_EQ(static_cast<uint8_t>(zlibs::utils::sysex_conf::Status::ErrorConnection), response.at(4));
}

TEST_F(SystemTest, ConfigurationSessionTimeoutDoesNotCloseBackup)
{
    bool   backup_active             = false;
    bool   closed_during_backup      = false;
    size_t backup_start_received_cnt = 0;
    size_t backup_end_received_cnt   = 0;

    messaging::subscribe<messaging::SystemSignal>(
        [&](const messaging::SystemSignal& event)
        {
            switch (event.system_message)
            {
            case messaging::SystemMessage::BackupStart:
                backup_active = true;
                backup_start_received_cnt++;
                break;

            case messaging::SystemMessage::BackupEnd:
                backup_active = false;
                backup_end_received_cnt++;
                break;

            case messaging::SystemMessage::ConfigurationSessionClosed:
                if (backup_active)
                {
                    closed_during_backup = true;
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

TEST_F(SystemTest, FactoryResetRequestIsDeferred)
{
    size_t factory_reset_start_cnt = 0;

    messaging::subscribe<messaging::SystemSignal>(
        [&](const messaging::SystemSignal& event)
        {
            if (event.system_message == messaging::SystemMessage::FactoryResetStart)
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

    test::MIDIInputEvent event = {};
    event.channel              = 1;
    event.index                = 0;
    event.value                = 127;
    event.message              = midi::MessageType::ControlChange;

    _helper.process_incoming(event);

    ASSERT_EQ(1, _system._hwa._builder_midi._hwaSerial._writeParser.total_written_channel_messages());

    _helper.process_incoming(event);

    ASSERT_EQ(1, _system._hwa._builder_midi._hwaSerial._writeParser.total_written_channel_messages());
}
#endif
