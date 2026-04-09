/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef PROJECT_TARGET_HW_TESTS_SUPPORTED

#include "tests/common.h"
#include "tests/helpers/database.h"
#include "tests/helpers/misc.h"
#include "tests/helpers/midi.h"
#include "database/builder.h"
#include "system/common.h"

#include <string>
#include <filesystem>

LOG_MODULE_REGISTER(hw_test, LOG_LEVEL_INF);

namespace
{
    std::vector<uint8_t> handshake_req             = { 0xF0, 0x00, 0x53, 0x43, 0x00, 0x00, 0x01, 0xF7 };
    std::vector<uint8_t> handshake_ack             = { 0xF0, 0x00, 0x53, 0x43, 0x01, 0x00, 0x01, 0xF7 };
    std::vector<uint8_t> reboot_req                = { 0xF0, 0x00, 0x53, 0x43, 0x00, 0x00, 0x7F, 0xF7 };
    std::vector<uint8_t> factory_reset_req         = { 0xF0, 0x00, 0x53, 0x43, 0x00, 0x00, 0x44, 0xF7 };
    std::vector<uint8_t> btldr_req                 = { 0xF0, 0x00, 0x53, 0x43, 0x00, 0x00, 0x55, 0xF7 };
    std::vector<uint8_t> backup_req                = { 0xF0, 0x00, 0x53, 0x43, 0x00, 0x00, 0x1B, 0xF7 };
    std::vector<uint8_t> restore_start_indicator   = { 0xF0, 0x00, 0x53, 0x43, 0x00, 0x00, 0x1C, 0xF7 };
    std::vector<uint8_t> restore_end_indicator     = { 0xF0, 0x00, 0x53, 0x43, 0x00, 0x00, 0x1D, 0xF7 };
    const std::string    flash_cmd                 = "make --no-print-directory flash ";
    const std::string    temp_midi_data_location   = "/tmp/temp_midi_data";
    const uint32_t       max_conn_delay_ms         = 25000;
    const uint32_t       handshake_retry_delay_ms  = 2000;
    const uint32_t       flash_retry_delay_ms      = 2000;
    const uint32_t       disconnect_retry_delay_ms = 2000;
    const uint32_t       backup_capture_timeout_ms = 10000;

    class HWTest : public ::testing::Test
    {
        protected:
        static void SetUpTestSuite()
        {
            LOG_INF("Device not flashed. Starting flashing procedure.");
            flashed = flash();
        }

        static void TearDownTestSuite()
        {
        }

        void SetUp()
        {
            ASSERT_TRUE(flashed);

            LOG_INF("Setting up test");

            // dummy db - used only to retrieve correct amount of supported presets
            ASSERT_TRUE(_database._instance.init(_handlers));

            factory_reset();
        }

        void TearDown()
        {
            LOG_INF("Tearing down test");

            test::wsystem("rm -f " + temp_midi_data_location);

            if (HasFailure())
            {
                LOG_INF("Preserving backup capture for failed test");
            }

            test::wsystem("killall receivemidi > /dev/null 2>&1");
            test::wsystem("killall sendmidi > /dev/null 2>&1");
            test::wsystem("killall ssterm > /dev/null 2>&1");

            std::string cmd_response;

            if (test::wsystem("pgrep ssterm", cmd_response) == 0)
            {
                LOG_INF("Waiting for ssterm process to be terminated");

                while (test::wsystem("pgrep ssterm", cmd_response) == 0)
                    ;
            }
        }

        enum class PowerCycleType : uint8_t
        {
            Standard,
            StandardWithDeviceCheck,
            StandardWithDeviceCheckIfNotPresent,
        };

        enum class SoftRebootType : uint8_t
        {
            Standard,
            FactoryReset,
            Bootloader,
        };

        static bool handshake()
        {
            LOG_INF("Ensuring the device is available for handshake request...");
            auto start_time = test::millis();

            while (!test::MIDIHelper::device_present(true))
            {
                if ((test::millis() - start_time) > max_conn_delay_ms)
                {
                    LOG_ERR("Device not available - waited %u ms.", max_conn_delay_ms);
                    return false;
                }
            }

            LOG_INF("Device available, attempting to send handshake");

            start_time = test::millis();

            while (handshake_ack != test::MIDIHelper::send_raw_sysex_to_device(handshake_req))
            {
                if ((test::millis() - start_time) > max_conn_delay_ms)
                {
                    LOG_ERR("Device didn't respond to handshake in %u ms.", max_conn_delay_ms);
                    return false;
                }

                LOG_ERR("OpenDeck device not responding to handshake - trying again in %u ms", handshake_retry_delay_ms);
                k_msleep(2000);
            }

            return true;
        }

        void reboot(SoftRebootType type = SoftRebootType::Standard)
        {
            ASSERT_TRUE(handshake());

            std::vector<uint8_t> cmd;

            switch (type)
            {
            case SoftRebootType::Standard:
            {
                LOG_INF("Sending reboot request to the device");
                cmd = reboot_req;
            }
            break;

            case SoftRebootType::FactoryReset:
            {
                LOG_INF("Sending factory reset request to the device");
                cmd = factory_reset_req;
            }
            break;

            case SoftRebootType::Bootloader:
            {
                LOG_INF("Sending bootloader request to the device");
                cmd = btldr_req;
            }
            break;

            default:
                return;
            }

            const size_t allowed_repeats = 2;
            bool         disconnected    = false;

            for (size_t i = 0; i < allowed_repeats; i++)
            {
                LOG_INF("Attempting to reboot the device, attempt %u", static_cast<unsigned>(i + 1));
                auto ret = _helper.send_raw_sysex_to_device(cmd, false);
                ASSERT_TRUE(ret.empty());
                auto start_time = test::millis();
                LOG_INF("Request sent. Waiting for the device to disconnect.");

                while (test::MIDIHelper::device_present(true))
                {
                    if ((test::millis() - start_time) > disconnect_retry_delay_ms)
                    {
                        LOG_ERR("Device didn't disconnect in %u ms.", disconnect_retry_delay_ms);
                        break;
                    }
                }

                if (!test::MIDIHelper::device_present(true))
                {
                    disconnected = true;
                    break;
                }
            }

            ASSERT_TRUE(disconnected);
            LOG_INF("Device disconnected.");

            if (type != SoftRebootType::Bootloader)
            {
                ASSERT_TRUE(handshake());
                test::MIDIHelper::flush();
            }
            else
            {
                auto start_time = test::millis();

                while (!test::MIDIHelper::device_present(true, true))
                {
                    if ((test::millis() - start_time) > max_conn_delay_ms)
                    {
                        LOG_ERR("Device not available - waited %u ms.", max_conn_delay_ms);
                        FAIL();
                    }
                }
            }
        }

        void factory_reset()
        {
            reboot(SoftRebootType::FactoryReset);
        }

        void bootloader()
        {
            reboot(SoftRebootType::Bootloader);
        }

        static bool flash()
        {
            auto flash = [&](std::string target)
            {
                const size_t allowed_repeats = 2;
                int          result          = -1;
                std::string  flash_target    = " TARGET=" + target;

                LOG_INF("Flashing development binary");

                for (size_t i = 0; i < allowed_repeats; i++)
                {
                    LOG_INF("Flashing the device, attempt %u", static_cast<unsigned>(i + 1));
                    result = test::wsystem(flash_cmd + flash_target);

                    if (result)
                    {
                        LOG_ERR("Flashing failed");
                        k_msleep(flash_retry_delay_ms);
                    }
                    else
                    {
                        LOG_INF("Flashing successful");
                        break;
                    }
                }

                return result == 0;
            };

            bool ret = flash(std::string(OPENDECK_TARGET));

            if (ret)
            {
                // check handshake - if that doesn't work, try cycling the power

                if (!handshake())
                {
                    //??
                }
            }

            return ret;
        }

        database::Builder           _database;
        tests::NoOpDatabaseHandlers _handlers;
        test::MIDIHelper            _helper = test::MIDIHelper(true);
        static inline bool          flashed = false;
    };
}    // namespace

TEST_F(HWTest, DatabaseInitialValues)
{
    constexpr size_t PARAM_SKIP = 2;
    LOG_INF("Checking global SYSTEM_SETTINGS");

    ASSERT_EQ(0,
              _helper.database_read_from_system_via_sysex(sys::Config::Section::Global::SystemSettings,
                                                          sys::Config::SystemSetting::ActivePreset));
    ASSERT_EQ(0,
              _helper.database_read_from_system_via_sysex(sys::Config::Section::Global::SystemSettings,
                                                          sys::Config::SystemSetting::PresetPreserve));
    ASSERT_EQ(0,
              _helper.database_read_from_system_via_sysex(sys::Config::Section::Global::SystemSettings,
                                                          sys::Config::SystemSetting::DisableForcedRefreshAfterPresetChange));
    ASSERT_EQ(0,
              _helper.database_read_from_system_via_sysex(sys::Config::Section::Global::SystemSettings,
                                                          sys::Config::SystemSetting::EnablePresetChangeWithProgramChangeIn));

    LOG_INF("Checking global MIDI settings and preset-scoped data for the active preset");

    // global block
    //----------------------------------
    // MidiSettings are global and should not depend on the selected preset.
    for (size_t i = 0; i < static_cast<uint8_t>(protocol::midi::Setting::Count); i++)
    {
        switch (i)
        {
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI
        case static_cast<size_t>(protocol::midi::Setting::DinEnabled):
        case static_cast<size_t>(protocol::midi::Setting::DinThruUsb):
        case static_cast<size_t>(protocol::midi::Setting::UsbThruDin):
        case static_cast<size_t>(protocol::midi::Setting::DinThruDin):
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BLE
        case static_cast<size_t>(protocol::midi::Setting::BleEnabled):
        case static_cast<size_t>(protocol::midi::Setting::BleThruUsb):
        case static_cast<size_t>(protocol::midi::Setting::BleThruBle):
        case static_cast<size_t>(protocol::midi::Setting::UsbThruBle):
#endif

#if defined(CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI) && defined(CONFIG_PROJECT_TARGET_SUPPORT_BLE)
        case static_cast<size_t>(protocol::midi::Setting::DinThruBle):
        case static_cast<size_t>(protocol::midi::Setting::BleThruDin):
#endif

        case static_cast<size_t>(protocol::midi::Setting::StandardNoteOff):
        case static_cast<size_t>(protocol::midi::Setting::RunningStatus):
        case static_cast<size_t>(protocol::midi::Setting::UsbThruUsb):
        case static_cast<size_t>(protocol::midi::Setting::UseGlobalChannel):
            ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Global::MidiSettings, i));
            break;

        case static_cast<size_t>(protocol::midi::Setting::GlobalChannel):
            ASSERT_EQ(1, _helper.database_read_from_system_via_sysex(sys::Config::Section::Global::MidiSettings, i));
            break;

        default:
            break;
        }
    }

    // preset blocks
    //----------------------------------
    // The remaining sections are preset-scoped and are read from the currently active preset.

    // button block
    //----------------------------------
    for (size_t i = 0; i < io::buttons::Collection::size(); i += PARAM_SKIP)
    {
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Button::Type, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Button::MessageType, i));
        ASSERT_EQ(127, _helper.database_read_from_system_via_sysex(sys::Config::Section::Button::Value, i));
        ASSERT_EQ(1, _helper.database_read_from_system_via_sysex(sys::Config::Section::Button::Channel, i));
    }

    for (size_t group = 0; group < io::buttons::Collection::groups(); group++)
    {
        for (size_t i = 0; i < io::buttons::Collection::size(group); i += PARAM_SKIP)
        {
            ASSERT_EQ(i, _helper.database_read_from_system_via_sysex(sys::Config::Section::Button::MidiId, i + io::buttons::Collection::start_index(group)));
        }
    }

    // encoder block
    //----------------------------------
    for (size_t i = 0; i < io::encoders::Collection::size(); i += PARAM_SKIP)
    {
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::Enable, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::Invert, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::Mode, i));
        ASSERT_EQ(i, _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::MidiId1, i));
        ASSERT_EQ(1, _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::Channel, i));
        ASSERT_EQ(4, _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::PulsesPerStep, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::LowerLimit, i));
        ASSERT_EQ(16383, _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::UpperLimit, i));
        ASSERT_EQ(127, _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::RepeatedValue, i));
        ASSERT_EQ(i, _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::MidiId2, i));
    }

    // analog block
    //----------------------------------
    for (size_t i = 0; i < io::analog::Collection::size(); i += PARAM_SKIP)
    {
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Analog::Enable, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Analog::Invert, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Analog::Invert, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Analog::LowerLimit, i));
        ASSERT_EQ(16383, _helper.database_read_from_system_via_sysex(sys::Config::Section::Analog::UpperLimit, i));
        ASSERT_EQ(1, _helper.database_read_from_system_via_sysex(sys::Config::Section::Analog::Channel, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Analog::LowerOffset, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Analog::UpperOffset, i));
    }

    for (size_t group = 0; group < io::analog::Collection::groups(); group++)
    {
        for (size_t i = 0; i < io::analog::Collection::size(group); i += PARAM_SKIP)
        {
            ASSERT_EQ(i, _helper.database_read_from_system_via_sysex(sys::Config::Section::Analog::MidiId, i + io::analog::Collection::start_index(group)));
        }
    }

    // LED block
    //----------------------------------
    for (size_t i = 0; i < static_cast<uint8_t>(io::leds::Setting::Count); i += PARAM_SKIP)
    {
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Leds::Global, i));
    }

    for (size_t group = 0; group < io::leds::Collection::groups(); group++)
    {
        for (size_t i = 0; i < io::leds::Collection::size(group); i += PARAM_SKIP)
        {
            ASSERT_EQ(i, _helper.database_read_from_system_via_sysex(sys::Config::Section::Leds::ActivationId, i + io::leds::Collection::start_index(group)));
        }
    }

    for (size_t i = 0; i < io::leds::Collection::size() / 3 + (io::touchscreen::Collection::size() / 3); i += PARAM_SKIP)
    {
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Leds::RgbEnable, i));
    }

    for (size_t i = 0; i < io::leds::Collection::size(); i += PARAM_SKIP)
    {
        ASSERT_EQ(static_cast<uint32_t>(io::leds::ControlType::MidiInNoteMultiVal), _helper.database_read_from_system_via_sysex(sys::Config::Section::Leds::ControlType, i));
        ASSERT_EQ(127, _helper.database_read_from_system_via_sysex(sys::Config::Section::Leds::ActivationValue, i));
        ASSERT_EQ(1, _helper.database_read_from_system_via_sysex(sys::Config::Section::Leds::Channel, i));
    }

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_I2C
    // i2c block
    //----------------------------------
    ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::I2c::Display, static_cast<size_t>(io::i2c::display::Setting::Enable)));
    ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::I2c::Display, static_cast<size_t>(io::i2c::display::Setting::DeviceInfoMsg)));
    ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::I2c::Display, static_cast<size_t>(io::i2c::display::Setting::Controller)));
    ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::I2c::Display, static_cast<size_t>(io::i2c::display::Setting::Resolution)));
    ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::I2c::Display, static_cast<size_t>(io::i2c::display::Setting::EventTime)));
    ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::I2c::Display, static_cast<size_t>(io::i2c::display::Setting::MidiNotesAlternate)));
    ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::I2c::Display, static_cast<size_t>(io::i2c::display::Setting::OctaveNormalization)));
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_TOUCHSCREEN
    // touchscreen block
    //----------------------------------
    for (size_t i = 0; i < static_cast<uint8_t>(io::touchscreen::Setting::Count); i += PARAM_SKIP)
    {
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Touchscreen::Setting, i));
    }

    for (size_t i = 0; i < io::touchscreen::Collection::size(); i += PARAM_SKIP)
    {
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Touchscreen::XPos, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Touchscreen::YPos, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Touchscreen::Width, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Touchscreen::Height, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Touchscreen::OnScreen, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Touchscreen::OffScreen, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Touchscreen::PageSwitchEnabled, i));
        ASSERT_EQ(0, _helper.database_read_from_system_via_sysex(sys::Config::Section::Touchscreen::PageSwitchIndex, i));
    }
#endif
}

TEST_F(HWTest, BackupAndRestore)
{
    ASSERT_GT(io::analog::Collection::size(), 0U);
    ASSERT_GT(io::buttons::Collection::size(), 0U);

    const size_t analog_component_count = io::analog::Collection::size();
    const size_t analog_component_index = analog_component_count - 1;

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS
    const size_t encoder_component_count = io::encoders::Collection::size();
    const bool   has_encoders            = encoder_component_count > 0;
    const size_t encoder_component_index = has_encoders ? encoder_component_count - 1 : 0;
#endif

    LOG_INF("Setting few random values in each available preset");

    for (int preset = 0; preset < _database._instance.supported_presets(); preset++)
    {
        ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Global::SystemSettings, sys::Config::SystemSetting::ActivePreset, preset));

        const size_t analog_midi_id = 15 + preset;

        ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Analog::MidiId,
                                                               analog_component_index,
                                                               analog_midi_id));

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS
        const size_t encoder_midi_id = 2 + preset;

        if (has_encoders)
        {
            ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Encoder::Channel,
                                                                   encoder_component_index,
                                                                   encoder_midi_id));
        }
#endif

        constexpr size_t BUTTON_COMPONENT = 0;
        const size_t     button_midi_id   = 90 + preset;

        ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Button::Value, BUTTON_COMPONENT, button_midi_id));
    }

    LOG_INF("Verifying that the custom values are active before backup");

    for (int preset = 0; preset < _database._instance.supported_presets(); preset++)
    {
        ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Global::SystemSettings,
                                                               sys::Config::SystemSetting::ActivePreset,
                                                               preset));
        ASSERT_EQ(preset,
                  _helper.database_read_from_system_via_sysex(sys::Config::Section::Global::SystemSettings,
                                                              sys::Config::SystemSetting::ActivePreset));
        ASSERT_EQ(15 + preset,
                  _helper.database_read_from_system_via_sysex(sys::Config::Section::Analog::MidiId,
                                                              analog_component_index));
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS
        if (has_encoders)
        {
            ASSERT_EQ(2 + preset,
                      _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::Channel,
                                                                  encoder_component_index));
        }
#endif
        ASSERT_EQ(90 + preset, _helper.database_read_from_system_via_sysex(sys::Config::Section::Button::Value, 0));
    }

    LOG_INF("Sending backup request");
    std::vector<std::vector<uint8_t>> responses = {};
    ASSERT_TRUE(test::MIDIHelper::capture_sysex_responses(backup_req,
                                                          responses,
                                                          backup_capture_timeout_ms));

    LOG_INF("Verifying backup");

    size_t restore_start_marker_index = responses.size();
    size_t restore_end_marker_index   = responses.size();

    for (size_t i = 0; i < responses.size(); ++i)
    {
        if (restore_start_marker_index == responses.size())
        {
            if (responses[i] == restore_start_indicator)
            {
                restore_start_marker_index = i;
            }

            continue;
        }

        if (restore_end_marker_index == responses.size())
        {
            if (responses[i] == restore_end_indicator)
            {
                restore_end_marker_index = i;
                break;
            }
        }
    }

    ASSERT_TRUE(restore_start_marker_index != responses.size());
    ASSERT_TRUE(restore_end_marker_index != responses.size());

    responses.erase(responses.begin(),
                    responses.begin() + restore_start_marker_index);

    // end index must be adjusted because the vector was shifted left
    restore_end_marker_index -= restore_start_marker_index;

    // erase everything after end marker, keep end marker itself
    responses.erase(responses.begin() + restore_end_marker_index + 1,
                    responses.end());

    LOG_INF("Received backup valid");

    factory_reset();

    LOG_INF("Verifying that the default values are active again");

    for (int preset = 0; preset < _database._instance.supported_presets(); preset++)
    {
        LOG_INF("Checking values in preset %d", preset);

        ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Global::SystemSettings, sys::Config::SystemSetting::ActivePreset, preset));
        ASSERT_EQ(preset, _helper.database_read_from_system_via_sysex(sys::Config::Section::Global::SystemSettings, sys::Config::SystemSetting::ActivePreset));
        ASSERT_EQ(analog_component_index,
                  _helper.database_read_from_system_via_sysex(sys::Config::Section::Analog::MidiId,
                                                              analog_component_index));
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS
        if (has_encoders)
        {
            ASSERT_EQ(encoder_component_index,
                      _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::Channel,
                                                                  encoder_component_index));
        }
#endif
        ASSERT_EQ(127, _helper.database_read_from_system_via_sysex(sys::Config::Section::Button::Value, 0));
    }

    LOG_INF("Restoring backup");

    test::MIDIHelper::flush();

    for (const auto& msg : responses)
    {
        auto ret = _helper.send_raw_sysex_to_device(msg);
        ASSERT_FALSE(ret.empty());
    }

    LOG_INF("Backup file sent successfully");

    // device is rebooted after restore
    test::sleep_ms(sys::REBOOT_DELAY_MS);

    ASSERT_TRUE(handshake());
    test::MIDIHelper::flush();

    LOG_INF("Verifying that the custom values are active again");

    for (int preset = 0; preset < _database._instance.supported_presets(); preset++)
    {
        ASSERT_TRUE(_helper.database_write_to_system_via_sysex(sys::Config::Section::Global::SystemSettings, sys::Config::SystemSetting::ActivePreset, preset));
        ASSERT_EQ(preset, _helper.database_read_from_system_via_sysex(sys::Config::Section::Global::SystemSettings, sys::Config::SystemSetting::ActivePreset));

        ASSERT_EQ(15 + preset,
                  _helper.database_read_from_system_via_sysex(sys::Config::Section::Analog::MidiId,
                                                              analog_component_index));
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS
        if (has_encoders)
        {
            ASSERT_EQ(2 + preset,
                      _helper.database_read_from_system_via_sysex(sys::Config::Section::Encoder::Channel,
                                                                  encoder_component_index));
        }
#endif
        ASSERT_EQ(90 + preset, _helper.database_read_from_system_via_sysex(sys::Config::Section::Button::Value, 0));
    }
}

TEST_F(HWTest, USBMIDIData)
{
    std::string cmd;
    std::string response;

    auto change_preset = [&]()
    {
        LOG_INF("Switching preset");
        const size_t preset      = 0;
        auto         preset0_req = _helper.generate_sysex_set_req(sys::Config::Section::Global::SystemSettings, sys::Config::SystemSetting::ActivePreset, preset);
        response                 = test::MIDIHelper::capture_midi_voice_messages_dump_after_request(preset0_req, sys::USB_CHANGE_FORCED_REFRESH_DELAY * 2);
    };

    change_preset();
    std::stringstream response_stream(response);
    std::string       line;
    size_t            received_messages = 0;

    while (std::getline(response_stream, line))
    {
        if (!test::trim_whitespace(line).empty())
        {
            received_messages++;
        }
    }

    LOG_INF("Received %u USB messages after preset change", static_cast<unsigned>(received_messages));
    ASSERT_EQ(io::buttons::Collection::size(io::buttons::GroupDigitalInputs), received_messages);
}

#endif
