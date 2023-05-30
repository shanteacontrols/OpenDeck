#ifndef PROJECT_TARGET_USB_OVER_SERIAL_HOST
#ifdef PROJECT_TARGET_SUPPORT_HW_TESTS

#include <string>
#include <filesystem>
#include "tests/Common.h"
#include "tests/helpers/Misc.h"
#include "tests/helpers/MIDI.h"
#include "tests/stubs/Database.h"
#include <HWTestDefines.h>
#include "system/Builder.h"

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
    const std::string    usb_power_off_cmd         = "echo write_high 1 > /dev/actl";
    const std::string    usb_power_on_cmd          = "echo write_low 1 > /dev/actl";
    const std::string    flash_cmd                 = "make --no-print-directory -C ../src flash ";
    const std::string    temp_midi_data_location   = "/tmp/temp_midi_data";
    const std::string    backup_file_location      = "/tmp/backup.txt";
    const std::string    latest_github_release_dir = "/tmp/latest_github_release";
    const uint32_t       max_conn_delay_ms         = 25000;
    const uint32_t       handshake_retry_delay_ms  = 2000;
    const uint32_t       flash_retry_delay_ms      = 2000;
    const uint32_t       disconnect_retry_delay_ms = 2000;

    class HWTest : public ::testing::Test
    {
        protected:
        static void SetUpTestSuite()
        {
            powerOn();
            LOG(INFO) << "Waiting " << max_conn_delay_ms << " ms to ensure target and programmer are available.";
            test::sleepMs(max_conn_delay_ms);

#ifdef TEST_FLASHING
            LOG(INFO) << "Device not flashed. Starting flashing procedure.";
            flashed = flash();
#endif
        }

        static void TearDownTestSuite()
        {
            powerOff();
        }

        void SetUp()
        {
#ifdef TEST_FLASHING
            ASSERT_TRUE(flashed);
#endif

            LOG(INFO) << "Setting up test";

            // dummy db - used only to retrieve correct amount of supported presets
            ASSERT_TRUE(_database._instance.init());

            factoryReset();
        }

        void TearDown()
        {
            LOG(INFO) << "Tearing down test";

            test::wsystem("rm -f " + temp_midi_data_location);
            test::wsystem("rm -f " + backup_file_location);
            test::wsystem("killall amidi > /dev/null 2>&1");
            test::wsystem("killall ssterm > /dev/null 2>&1");

            std::string cmdResponse;

            if (test::wsystem("pgrep ssterm", cmdResponse) == 0)
            {
                LOG(INFO) << "Waiting for ssterm process to be terminated";

                while (test::wsystem("pgrep ssterm", cmdResponse) == 0)
                    ;
            }
        }

        enum class powerCycleType_t : uint8_t
        {
            STANDARD,
            STANDARD_WITH_DEVICE_CHECK,
            STANDARD_WITH_DEVICE_CHECK_IF_NOT_PRESENT,
        };

        enum class softRebootType_t : uint8_t
        {
            STANDARD,
            FACTORY_RESET,
            BOOTLOADER,
        };

        enum class flashType_t : uint8_t
        {
            DEVELOPMENT,
            RELEASE,
        };

        static void powerOn()
        {
            // Send newline to arduino controller to make sure on/off commands
            // are properly parsed.
            // Add slight delay between power commands to avoid issues.
            ASSERT_EQ(0, test::wsystem("echo > /dev/actl && sleep 1"));

            LOG(INFO) << "Turning USB devices on";
            ASSERT_EQ(0, test::wsystem(usb_power_on_cmd));
        }

        static void powerOff()
        {
            // Send newline to arduino controller to make sure on/off commands
            // are properly parsed.
            // Add slight delay between power commands to avoid issues.
            ASSERT_EQ(0, test::wsystem("echo > /dev/actl && sleep 1"));

            LOG(INFO) << "Turning USB devices off";
            ASSERT_EQ(0, test::wsystem(usb_power_off_cmd));
        }

        static void cyclePower(powerCycleType_t powerCycleType)
        {
            auto cycle = []()
            {
                LOG(INFO) << "Cycling power";
                powerOff();
                powerOn();
            };

            auto checkPresence = []()
            {
                LOG(INFO) << "Checking for device availability";

                // ensure the device is present
                auto startTime = test::millis();

                while (!MIDIHelper::devicePresent(true))
                {
                    if ((test::millis() - startTime) > max_conn_delay_ms)
                    {
                        LOG(ERROR) << "Device didn't connect in " << max_conn_delay_ms << " ms.";
                        return false;
                    }
                }

                return true;
            };

            // power cycle first if needed
            switch (powerCycleType)
            {
            case powerCycleType_t::STANDARD:
            {
                cycle();
            }
            break;

            case powerCycleType_t::STANDARD_WITH_DEVICE_CHECK:
            {
                cycle();

                if (!checkPresence())
                {
                    FAIL();
                }
            }
            break;

            case powerCycleType_t::STANDARD_WITH_DEVICE_CHECK_IF_NOT_PRESENT:
            {
                if (!checkPresence())
                {
                    cycle();

                    if (!checkPresence())
                    {
                        FAIL();
                    }
                }
            }
            break;

            default:
                break;
            }
        }

        static bool handshake()
        {
            LOG(INFO) << "Ensuring the device is available for handshake request...";
            auto startTime = test::millis();

            while (!MIDIHelper::devicePresent(true))
            {
                if ((test::millis() - startTime) > max_conn_delay_ms)
                {
                    LOG(ERROR) << "Device not available - waited " << max_conn_delay_ms << " ms.";
                    return false;
                }
            }

            LOG(INFO) << "Device available, attempting to send handshake";

            startTime = test::millis();

            while (handshake_ack != MIDIHelper::sendRawSysExToDevice(handshake_req))
            {
                if ((test::millis() - startTime) > max_conn_delay_ms)
                {
                    LOG(ERROR) << "Device didn't respond to handshake in " << max_conn_delay_ms << " ms.";
                    return false;
                }

                LOG(ERROR) << "OpenDeck device not responding to handshake - trying again in " << handshake_retry_delay_ms << " ms";
                test::sleepMs(2000);
            }

            return true;
        }

        void reboot(softRebootType_t type = softRebootType_t::STANDARD)
        {
            ASSERT_TRUE(handshake());

            std::vector<uint8_t> cmd;

            switch (type)
            {
            case softRebootType_t::STANDARD:
            {
                LOG(INFO) << "Sending reboot request to the device";
                cmd = reboot_req;
            }
            break;

            case softRebootType_t::FACTORY_RESET:
            {
                LOG(INFO) << "Sending factory reset request to the device";
                cmd = factory_reset_req;
            }
            break;

            case softRebootType_t::BOOTLOADER:
            {
                LOG(INFO) << "Sending bootloader request to the device";
                cmd = btldr_req;
            }
            break;

            default:
                return;
            }

            const size_t ALLOWED_REPEATS = 2;
            bool         disconnected    = false;

            for (size_t i = 0; i < ALLOWED_REPEATS; i++)
            {
                LOG(INFO) << "Attempting to reboot the device, attempt " << i + 1;
                auto ret = _helper.sendRawSysExToDevice(cmd, false);
                ASSERT_TRUE(ret.empty());
                auto startTime = test::millis();
                LOG(INFO) << "Request sent. Waiting for the device to disconnect.";

                while (MIDIHelper::devicePresent(true))
                {
                    if ((test::millis() - startTime) > disconnect_retry_delay_ms)
                    {
                        LOG(ERROR) << "Device didn't disconnect in " << disconnect_retry_delay_ms << " ms.";
                        break;
                    }
                }

                if (!MIDIHelper::devicePresent(true))
                {
                    disconnected = true;
                    break;
                }
            }

            ASSERT_TRUE(disconnected);
            LOG(INFO) << "Device disconnected.";

            if (type != softRebootType_t::BOOTLOADER)
            {
                ASSERT_TRUE(handshake());
                MIDIHelper::flush();
            }
            else
            {
                auto startTime = test::millis();

                while (!MIDIHelper::devicePresent(true, true))
                {
                    if ((test::millis() - startTime) > max_conn_delay_ms)
                    {
                        LOG(ERROR) << "Device not available - waited " << max_conn_delay_ms << " ms.";
                        FAIL();
                    }
                }
            }
        }

        void factoryReset()
        {
            reboot(softRebootType_t::FACTORY_RESET);
        }

        void bootloader()
        {
            reboot(softRebootType_t::BOOTLOADER);
        }

        static bool flash(flashType_t flashType = flashType_t::DEVELOPMENT)
        {
            auto flash = [&](std::string target, std::string args)
            {
                const size_t ALLOWED_REPEATS = 2;
                int          result          = -1;
                std::string  flashTarget     = " TARGET=" + target;
                std::string  extraArgs       = "";

                if (flashType == flashType_t::RELEASE)
                {
                    LOG(INFO) << "Flashing release binary";
                    extraArgs += " FLASH_BINARY_DIR=" + latest_github_release_dir + " ";
                }
                else
                {
                    LOG(INFO) << "Flashing development binary";
                }

                for (size_t i = 0; i < ALLOWED_REPEATS; i++)
                {
                    LOG(INFO) << "Flashing the device, attempt " << i + 1;
                    result = test::wsystem(flash_cmd + flashTarget + " " + args + extraArgs);

                    if (result)
                    {
                        LOG(ERROR) << "Flashing failed";
                        test::sleepMs(flash_retry_delay_ms);
                    }
                    else
                    {
                        LOG(INFO) << "Flashing successful";
                        break;
                    }
                }

                return result == 0;
            };

            bool ret = false;

#ifndef PROJECT_TARGET_SUPPORT_USB
            LOG(INFO) << "Flashing USB Link MCU";
            ret = flash(std::string(USB_LINK_TARGET), std::string(FLASH_ARGS_USB_LINK));

            if (ret)
            {
#endif

                ret = flash(std::string(PROJECT_TARGET_NAME), std::string(FLASH_ARGS));

                if (ret)
                {
                    // check handshake - if that doesn't work, try cycling the power

                    if (!handshake())
                    {
                        cyclePower(powerCycleType_t::STANDARD_WITH_DEVICE_CHECK);
                        ret = handshake();
                    }
                }

#ifndef PROJECT_TARGET_SUPPORT_USB
            }
#endif

            return ret;
        }

        TestDatabase _database;
        MIDIHelper   _helper = MIDIHelper(true);
#ifdef TEST_FLASHING
        static bool flashed;
#endif
    };
}    // namespace

#ifdef TEST_FLASHING
bool HWTest::flashed = false;
#endif

TEST_F(HWTest, DatabaseInitialValues)
{
    constexpr size_t PARAM_SKIP = 2;

    // check only first and the last preset
    for (int preset = 0; preset < _database._instance.getSupportedPresets(); preset += (_database._instance.getSupportedPresets() - 1))
    {
        LOG(INFO) << "Checking initial values for preset " << preset + 1;
        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::PRESETS, 0, preset));
        ASSERT_EQ(preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::global_t::PRESETS, 0));

        // MIDI block
        //----------------------------------
        // settings section
        // all values should be set to 0 except for the global channel which should be 1
        for (size_t i = 0; i < static_cast<uint8_t>(protocol::MIDI::setting_t::AMOUNT); i++)
        {
            switch (i)
            {
#ifdef PROJECT_TARGET_SUPPORT_DIN_MIDI
            case static_cast<size_t>(protocol::MIDI::setting_t::DIN_ENABLED):
            case static_cast<size_t>(protocol::MIDI::setting_t::DIN_THRU_USB):
            case static_cast<size_t>(protocol::MIDI::setting_t::USB_THRU_DIN):
            case static_cast<size_t>(protocol::MIDI::setting_t::DIN_THRU_DIN):
#endif

#ifdef PROJECT_TARGET_SUPPORT_BLE
            case static_cast<size_t>(protocol::MIDI::setting_t::BLE_ENABLED):
            case static_cast<size_t>(protocol::MIDI::setting_t::BLE_THRU_USB):
            case static_cast<size_t>(protocol::MIDI::setting_t::BLE_THRU_BLE):
            case static_cast<size_t>(protocol::MIDI::setting_t::USB_THRU_BLE):
#endif

#if defined(PROJECT_TARGET_SUPPORT_DIN_MIDI) && defined(PROJECT_TARGET_SUPPORT_BLE)
            case static_cast<size_t>(protocol::MIDI::setting_t::DIN_THRU_BLE):
            case static_cast<size_t>(protocol::MIDI::setting_t::BLE_THRU_DIN):
#endif

            case static_cast<size_t>(protocol::MIDI::setting_t::STANDARD_NOTE_OFF):
            case static_cast<size_t>(protocol::MIDI::setting_t::RUNNING_STATUS):
            case static_cast<size_t>(protocol::MIDI::setting_t::USB_THRU_USB):
            case static_cast<size_t>(protocol::MIDI::setting_t::USE_GLOBAL_CHANNEL):
            {
                ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::global_t::MIDI_SETTINGS, i));
            }
            break;

            case static_cast<size_t>(protocol::MIDI::setting_t::GLOBAL_CHANNEL):
            {
                ASSERT_EQ(1, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::global_t::MIDI_SETTINGS, i));
            }
            break;

            default:
                break;
            }
        }

        // button block
        //----------------------------------
        // type section
        // all values should be set to 0 (default type)
        for (size_t i = 0; i < io::Buttons::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::button_t::TYPE, i));
        }

        // midi message section
        // all values should be set to 0 (default/note)
        for (size_t i = 0; i < io::Buttons::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::button_t::MESSAGE_TYPE, i));
        }

        // midi id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < io::Buttons::Collection::GROUPS(); group++)
        {
            for (size_t i = 0; i < io::Buttons::Collection::SIZE(group); i += PARAM_SKIP)
            {
                ASSERT_EQ(i, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::button_t::MIDI_ID, i + io::Buttons::Collection::START_INDEX(group)));
            }
        }

        // midi velocity section
        // all values should be set to 127
        for (size_t i = 0; i < io::Buttons::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(127, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::button_t::VALUE, i));
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < io::Buttons::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(1, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::button_t::CHANNEL, i));
        }

        // encoders block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (size_t i = 0; i < io::Encoders::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::encoder_t::ENABLE, i));
        }

        // invert section
        // all values should be set to 0
        for (size_t i = 0; i < io::Encoders::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::encoder_t::INVERT, i));
        }

        // mode section
        // all values should be set to 0
        for (size_t i = 0; i < io::Encoders::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::encoder_t::MODE, i));
        }

        // midi id section
        for (size_t i = 0; i < io::Encoders::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(i, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::encoder_t::MIDI_ID, i));
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < io::Encoders::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(1, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::encoder_t::CHANNEL, i));
        }

        // pulses per step section
        // all values should be set to 4
        for (size_t i = 0; i < io::Encoders::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(4, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::encoder_t::PULSES_PER_STEP, i));
        }

        // analog block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (size_t i = 0; i < io::Analog::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::ENABLE, i));
        }

        // invert section
        // all values should be set to 0
        for (size_t i = 0; i < io::Analog::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::INVERT, i));
        }

        // type section
        // all values should be set to 0
        for (size_t i = 0; i < io::Analog::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::INVERT, i));
        }

        // midi id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < io::Analog::Collection::GROUPS(); group++)
        {
            for (size_t i = 0; i < io::Analog::Collection::SIZE(group); i += PARAM_SKIP)
            {
                ASSERT_EQ(i, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::MIDI_ID, i + io::Analog::Collection::START_INDEX(group)));
            }
        }

        // lower limit section
        // all values should be set to 0
        for (size_t i = 0; i < io::Analog::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::LOWER_LIMIT, i));
        }

        // upper limit section
        // all values should be set to 16383
        for (size_t i = 0; i < io::Analog::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(16383, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::UPPER_LIMIT, i));
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < io::Analog::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(1, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::CHANNEL, i));
        }

        // lower offset section
        // all values should be set to 0
        for (size_t i = 0; i < io::Analog::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::LOWER_OFFSET, i));
        }

        // upper offset section
        // all values should be set to 0
        for (size_t i = 0; i < io::Analog::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::UPPER_OFFSET, i));
        }

        // LED block
        //----------------------------------
        // global section
        // all values should be set to 0
        for (size_t i = 0; i < static_cast<uint8_t>(io::LEDs::setting_t::AMOUNT); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::GLOBAL, i));
        }

        // activation id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < io::LEDs::Collection::GROUPS(); group++)
        {
            for (size_t i = 0; i < io::LEDs::Collection::SIZE(group); i += PARAM_SKIP)
            {
                ASSERT_EQ(i, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::ACTIVATION_ID, i + io::LEDs::Collection::START_INDEX(group)));
            }
        }

        // rgb enable section
        // all values should be set to 0
        for (size_t i = 0; i < io::LEDs::Collection::SIZE() / 3 + (io::Touchscreen::Collection::SIZE() / 3); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::RGB_ENABLE, i));
        }

        // control type section
        // all values should be set to MIDI_IN_NOTE_MULTI_VAL
        for (size_t i = 0; i < io::LEDs::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(static_cast<uint32_t>(io::LEDs::controlType_t::MIDI_IN_NOTE_MULTI_VAL), _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::CONTROL_TYPE, i));
        }

        // activation value section
        // all values should be set to 127
        for (size_t i = 0; i < io::LEDs::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(127, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::ACTIVATION_VALUE, i));
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < io::LEDs::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(1, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::CHANNEL, i));
        }

#ifdef PROJECT_TARGET_SUPPORT_I2C
        // i2c block
        //----------------------------------
        // display section
        ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::i2c_t::DISPLAY, static_cast<size_t>(io::Display::setting_t::ENABLE)));
        ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::i2c_t::DISPLAY, static_cast<size_t>(io::Display::setting_t::DEVICE_INFO_MSG)));
        ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::i2c_t::DISPLAY, static_cast<size_t>(io::Display::setting_t::CONTROLLER)));
        ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::i2c_t::DISPLAY, static_cast<size_t>(io::Display::setting_t::RESOLUTION)));
        ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::i2c_t::DISPLAY, static_cast<size_t>(io::Display::setting_t::EVENT_TIME)));
        ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::i2c_t::DISPLAY, static_cast<size_t>(io::Display::setting_t::MIDI_NOTES_ALTERNATE)));
        ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::i2c_t::DISPLAY, static_cast<size_t>(io::Display::setting_t::OCTAVE_NORMALIZATION)));
#endif

#ifdef PROJECT_TARGET_SUPPORT_TOUCHSCREEN
        // touchscreen block
        //----------------------------------
        // setting section
        // all values should be set to 0
        for (size_t i = 0; i < static_cast<uint8_t>(io::Touchscreen::setting_t::AMOUNT); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::touchscreen_t::SETTING, i));
        }

        // x position section
        // all values should be set to 0
        for (size_t i = 0; i < io::Touchscreen::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::touchscreen_t::X_POS, i));
        }

        // y position section
        // all values should be set to 0
        for (size_t i = 0; i < io::Touchscreen::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::touchscreen_t::Y_POS, i));
        }

        // WIDTH section
        // all values should be set to 0
        for (size_t i = 0; i < io::Touchscreen::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::touchscreen_t::WIDTH, i));
        }

        // HEIGHT section
        // all values should be set to 0
        for (size_t i = 0; i < io::Touchscreen::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::touchscreen_t::HEIGHT, i));
        }

        // on screen section
        // all values should be set to 0
        for (size_t i = 0; i < io::Touchscreen::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::touchscreen_t::ON_SCREEN, i));
        }

        // off screen section
        // all values should be set to 0
        for (size_t i = 0; i < io::Touchscreen::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::touchscreen_t::OFF_SCREEN, i));
        }

        // page switch enabled section
        // all values should be set to 0
        for (size_t i = 0; i < io::Touchscreen::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::touchscreen_t::PAGE_SWITCH_ENABLED, i));
        }

        // page switch index section
        // all values should be set to 0
        for (size_t i = 0; i < io::Touchscreen::Collection::SIZE(); i += PARAM_SKIP)
        {
            ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::touchscreen_t::PAGE_SWITCH_INDEX, i));
        }
#endif
    }
}

#ifdef PROJECT_MCU_SUPPORT_BOOTLOADER
TEST_F(HWTest, FwUpdate)
{
    LOG(INFO) << "Setting few random values in each available preset";

    for (int preset = 0; preset < _database._instance.getSupportedPresets(); preset++)
    {
        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::PRESETS, database::Config::presetSetting_t::ACTIVE_PRESET, preset));
        ASSERT_EQ(preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::global_t::PRESETS, database::Config::presetSetting_t::ACTIVE_PRESET));

        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::analog_t::MIDI_ID, 4, 15 + preset));
#ifdef ENCODERS_SUPPORTED
        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::encoder_t::CHANNEL, 1, 2 + preset));
#endif
        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::button_t::VALUE, 0, 90 + preset));

        ASSERT_EQ(15 + preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::MIDI_ID, 4));
#ifdef ENCODERS_SUPPORTED
        ASSERT_EQ(2 + preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::encoder_t::CHANNEL, 1));
#endif
        ASSERT_EQ(90 + preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::button_t::VALUE, 0));
    }

    bootloader();

    std::string cmd = flash_cmd + std::string(" FLASH_TOOL=opendeck TARGET=") + std::string(PROJECT_TARGET_NAME);
    ASSERT_EQ(0, test::wsystem(cmd));
    LOG(INFO) << "Firmware file sent successfully, verifying that device responds to handshake";

    ASSERT_TRUE(handshake());

    LOG(INFO) << "Verifying that the custom values are still active after FW update";

    for (int preset = 0; preset < _database._instance.getSupportedPresets(); preset++)
    {
        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::PRESETS, database::Config::presetSetting_t::ACTIVE_PRESET, preset));
        ASSERT_EQ(preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::global_t::PRESETS, database::Config::presetSetting_t::ACTIVE_PRESET));

        ASSERT_EQ(15 + preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::MIDI_ID, 4));
#ifdef ENCODERS_SUPPORTED
        ASSERT_EQ(2 + preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::encoder_t::CHANNEL, 1));
#endif
        ASSERT_EQ(90 + preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::button_t::VALUE, 0));
    }
}

TEST_F(HWTest, FwUpdateFromLastRelease)
{
    flash(flashType_t::RELEASE);
    bootloader();

    std::string cmd = flash_cmd + std::string(" FLASH_TOOL=opendeck TARGET=") + std::string(PROJECT_TARGET_NAME);
    ASSERT_EQ(0, test::wsystem(cmd));
    LOG(INFO) << "Firmware file sent successfully, verifying that device responds to handshake";

    ASSERT_TRUE(handshake());
}
#endif

TEST_F(HWTest, BackupAndRestore)
{
    LOG(INFO) << "Setting few random values in each available preset";

    for (int preset = 0; preset < _database._instance.getSupportedPresets(); preset++)
    {
        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::PRESETS, database::Config::presetSetting_t::ACTIVE_PRESET, preset));

        const size_t ANALOG_ID      = 4;
        const size_t ANALOG_MIDI_ID = 15 + preset;

        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::analog_t::MIDI_ID, ANALOG_ID, ANALOG_MIDI_ID));

#ifdef ENCODERS_SUPPORTED
        const size_t ENCODER_COMPONENT = 1;
        const size_t ENCODER_MIDI_ID   = 2 + preset;

        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::encoder_t::CHANNEL, ENCODER_COMPONENT, ENCODER_MIDI_ID));
#endif

        const size_t BUTTON_COMPONENT = 0;
        const size_t BUTTON_MIDI_ID   = 90 + preset;

        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::button_t::VALUE, BUTTON_COMPONENT, BUTTON_MIDI_ID));
    }

    LOG(INFO) << "Sending backup request";
    std::string cmd = std::string("amidi -p ") + _helper.amidiPort(OPENDECK_MIDI_DEVICE_NAME) + " -S \"" + test::vectorToHexString(backup_req) + "\" -d -t 4 > " + backup_file_location;
    ASSERT_EQ(0, test::wsystem(cmd));

    factoryReset();

    LOG(INFO) << "Verifying that the default values are active again";

    for (int preset = 0; preset < _database._instance.getSupportedPresets(); preset++)
    {
        LOG(INFO) << "Checking values in preset " << preset;

        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::PRESETS, database::Config::presetSetting_t::ACTIVE_PRESET, preset));
        ASSERT_EQ(preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::global_t::PRESETS, database::Config::presetSetting_t::ACTIVE_PRESET));
        ASSERT_EQ(4, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::MIDI_ID, 4));
#ifdef ENCODERS_SUPPORTED
        ASSERT_EQ(1, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::encoder_t::CHANNEL, 1));
#endif
        ASSERT_EQ(127, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::button_t::VALUE, 0));
    }

    LOG(INFO) << "Restoring backup";

    // remove everything before the restore start indicator
    ASSERT_EQ(0, test::wsystem("sed -n -i '/^" + test::vectorToHexString(restore_start_indicator) + "$/,$p' " + backup_file_location));

    // ...and also after the restore end indicator
    ASSERT_EQ(0, test::wsystem("sed -i '/^" + test::vectorToHexString(restore_end_indicator) + "$/q' " + backup_file_location));

    // send backup
    std::ifstream backupStream(backup_file_location);
    std::string   line;

    while (getline(backupStream, line))
    {
        // restore end indicator will reboot the board - in that case don't expect any response
        if (line.find(test::vectorToHexString(restore_end_indicator)) == std::string::npos)
        {
            auto ret = _helper.sendRawSysExToDevice(test::hexStringToVector(line));
            ASSERT_FALSE(ret.empty());
        }
        else
        {
            _helper.sendRawSysExToDevice(test::hexStringToVector(line), false);
        }
    }

    LOG(INFO) << "Backup file sent successfully";

    ASSERT_TRUE(handshake());

    LOG(INFO) << "Verifying that the custom values are active again";

    for (int preset = 0; preset < _database._instance.getSupportedPresets(); preset++)
    {
        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::PRESETS, database::Config::presetSetting_t::ACTIVE_PRESET, preset));
        ASSERT_EQ(preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::global_t::PRESETS, database::Config::presetSetting_t::ACTIVE_PRESET));

        ASSERT_EQ(15 + preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::analog_t::MIDI_ID, 4));
#ifdef ENCODERS_SUPPORTED
        ASSERT_EQ(2 + preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::encoder_t::CHANNEL, 1));
#endif
        ASSERT_EQ(90 + preset, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::button_t::VALUE, 0));
    }
}

TEST_F(HWTest, USBMIDIData)
{
    std::string cmd;
    std::string response;

    auto changePreset = [&]()
    {
        LOG(INFO) << "Switching preset";
        const size_t PRESET     = 0;
        auto         preset0Req = _helper.generateSysExSetReq(sys::Config::Section::global_t::PRESETS, database::Config::presetSetting_t::ACTIVE_PRESET, PRESET);
        cmd                     = std::string("amidi -p ") + _helper.amidiPort(OPENDECK_MIDI_DEVICE_NAME) + " -S \"" + test::vectorToHexString(preset0Req) + "\" -d -t 3 > " + temp_midi_data_location;
        ASSERT_EQ(0, test::wsystem(cmd, response));
        test::wsystem("cat " + temp_midi_data_location, response);
    };

    changePreset();
    auto receivedMessages = _helper.totalChannelMessages(response);
    LOG(INFO) << "Received " << receivedMessages << " USB messages after preset change";
    ASSERT_EQ(io::Buttons::Collection::SIZE(io::Buttons::GROUP_DIGITAL_INPUTS), receivedMessages);
}

#ifdef PROJECT_TARGET_SUPPORT_DIN_MIDI
#ifdef TEST_DIN_MIDI
TEST_F(HWTest, DINMIDIData)
{
    std::string cmd;
    std::string response;

    auto changePreset = [&]()
    {
        LOG(INFO) << "Switching preset";
        const size_t PRESET     = 0;
        auto         preset0Req = _helper.generateSysExSetReq(sys::Config::Section::global_t::PRESETS, database::Config::presetSetting_t::ACTIVE_PRESET, PRESET);
        cmd                     = std::string("amidi -p ") + _helper.amidiPort(OPENDECK_MIDI_DEVICE_NAME) + " -S \"" + test::vectorToHexString(preset0Req) + "\" -d -t 3";
        ASSERT_EQ(0, test::wsystem(cmd, response));
    };

    auto monitor = [&](bool usbMonitoring = false)
    {
        LOG(INFO) << "Monitoring DIN MIDI interface " << OUT_DIN_MIDI_PORT;
        cmd = std::string("amidi -p ") + _helper.amidiPort(OUT_DIN_MIDI_PORT) + " -d > " + temp_midi_data_location + " &";
        ASSERT_EQ(0, test::wsystem(cmd));

        if (usbMonitoring)
        {
            cmd = std::string("amidi -p ") + _helper.amidiPort(OPENDECK_MIDI_DEVICE_NAME) + " -d & ";
            ASSERT_EQ(0, test::wsystem(cmd));
        }
    };

    auto stopMonitoring = [&]()
    {
        test::wsystem("killall amidi > /dev/null");
        test::wsystem("cat " + temp_midi_data_location, response);
    };

    monitor();
    changePreset();
    stopMonitoring();
    auto receivedMessages = _helper.totalChannelMessages(response);

    LOG(INFO) << "Verifying that no data reached DIN MIDI interface due to the DIN MIDI being disabled";
    ASSERT_EQ(0, receivedMessages);

    LOG(INFO) << "Enabling DIN MIDI";
    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::MIDI_SETTINGS, protocol::MIDI::setting_t::DIN_ENABLED, 1));
    monitor();
    changePreset();
    stopMonitoring();
    receivedMessages = _helper.totalChannelMessages(response);
    LOG(INFO) << "Received " << receivedMessages << " DIN MIDI messages after preset change";
    ASSERT_EQ(io::Buttons::Collection::SIZE(io::Buttons::GROUP_DIGITAL_INPUTS), receivedMessages);

    // enable DIN MIDI passthrough, send data to DIN MIDI in to device and expect the same message passed to output port
    LOG(INFO) << "Enabling DIN to DIN thru";
    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::MIDI_SETTINGS, protocol::MIDI::setting_t::DIN_THRU_DIN, 1));

    // having DIN to USB thru activated should not influence din to din thruing
    LOG(INFO) << "Enabling DIN to USB thru";
    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::MIDI_SETTINGS, protocol::MIDI::setting_t::DIN_THRU_USB, 1));

    // monitor usb as well so that outgoing usb messages aren't "stuck"
    monitor(true);
    std::string  msg              = "90007F";
    const size_t MESSAGES_TO_SEND = 100;

    LOG(INFO) << "Sending data to DIN MIDI interface " << IN_DIN_MIDI_PORT;
    LOG(INFO) << "Message: " << msg;

    cmd = "amidi -p " + _helper.amidiPort(IN_DIN_MIDI_PORT) + " -S \"" + msg + "\"";

    for (size_t i = 0; i < MESSAGES_TO_SEND; i++)
    {
        ASSERT_EQ(0, test::wsystem(cmd));
    }

    test::sleepMs(1000);
    stopMonitoring();
    receivedMessages = _helper.totalChannelMessages(response);
    LOG(INFO) << "Received " << receivedMessages << " DIN MIDI messages on passthrough interface";
    ASSERT_EQ(MESSAGES_TO_SEND, receivedMessages);
    ASSERT_NE(response.find(msg), std::string::npos);
}
#endif
#endif

#ifdef TEST_IO
TEST_F(HWTest, InputOutput)
{
    std::string response;

    auto monitor = [&]()
    {
        LOG(INFO) << "Monitoring USB MIDI interface";
        auto cmd = std::string("amidi -p ") + _helper.amidiPort(OPENDECK_MIDI_DEVICE_NAME) + " -d > " + temp_midi_data_location + " &";
        ASSERT_EQ(0, test::wsystem(cmd));
    };

    auto stopMonitoring = [&]()
    {
        test::wsystem("killall amidi > /dev/null");
        test::wsystem("cat " + temp_midi_data_location, response);
    };

#ifdef TEST_IO_ANALOG
    for (size_t i = 0; i < hwTestAnalogDescriptor.size(); i++)
    {
        LOG(INFO) << "Enabling analog input " << hwTestAnalogDescriptor.at(i).index;
        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::analog_t::ENABLE, hwTestAnalogDescriptor.at(i).index, 1));

        LOG(INFO) << "Resetting analog input " << hwTestAnalogDescriptor.at(i).index << " to default state";
        test::wsystem(std::string("echo write_low ") + std::to_string(hwTestAnalogDescriptor.at(i).pin) + " > /dev/actl");
        test::sleepMs(1000);
    }
#endif

    monitor();

#ifdef TEST_IO_SWITCHES
    // reset the state first
    for (size_t i = 0; i < hwTestSwDescriptor.size(); i++)
    {
        LOG(INFO) << "Resetting switch " << hwTestSwDescriptor.at(i).index << " to default state";
        test::wsystem(std::string("echo write_low ") + std::to_string(hwTestSwDescriptor.at(i).pin) + " > /dev/actl");
        test::sleepMs(1000);
    }

    for (size_t i = 0; i < hwTestSwDescriptor.size(); i++)
    {
        LOG(INFO) << "Toggling switch " << hwTestSwDescriptor.at(i).index;

        test::wsystem(std::string("echo write_high ") + std::to_string(hwTestSwDescriptor.at(i).pin) + " > /dev/actl");
        test::sleepMs(1000);
        test::wsystem(std::string("echo write_low ") + std::to_string(hwTestSwDescriptor.at(i).pin) + " > /dev/actl");
        test::sleepMs(1000);
    }
#endif

#ifdef TEST_IO_ANALOG

    for (size_t i = 0; i < hwTestAnalogDescriptor.size(); i++)
    {
        LOG(INFO) << "Toggling analog input " << hwTestAnalogDescriptor.at(i).index;

        test::wsystem(std::string("echo write_high ") + std::to_string(hwTestAnalogDescriptor.at(i).pin) + " > /dev/actl");
        test::sleepMs(1000);
        test::wsystem(std::string("echo write_low ") + std::to_string(hwTestAnalogDescriptor.at(i).pin) + " > /dev/actl");
        test::sleepMs(1000);
    }
#endif

    stopMonitoring();

    _helper.totalChannelMessages(response);

    LOG(INFO) << "Verifying if received messages are valid";

#ifdef TEST_IO_SWITCHES
    for (size_t i = 0; i < hwTestSwDescriptor.size(); i++)
    {
        std::stringstream sstream;
        sstream << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << hwTestSwDescriptor.at(i).index;

        std::string msgOn  = "90" + sstream.str() + "00";
        std::string msgOff = "90" + sstream.str() + "7F";

        LOG(INFO) << "Searching for: " << msgOn;
        ASSERT_TRUE(response.find(msgOn) != std::string::npos);

        LOG(INFO) << "Searching for: " << msgOff;
        ASSERT_TRUE(response.find(msgOff) != std::string::npos);
    }
#endif

#ifdef TEST_IO_ANALOG
    for (size_t i = 0; i < hwTestAnalogDescriptor.size(); i++)
    {
        std::stringstream sstream;
        bool              atLeast1Match;
        sstream << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << hwTestAnalogDescriptor.at(i).index;

        // allow some variance for analog inputs
        std::vector<std::string> msgOn = {
            { "B0" + sstream.str() + "00" },
            { "B0" + sstream.str() + "01" },
            { "B0" + sstream.str() + "02" },
        };

        std::vector<std::string> msgOff = {
            { "B0" + sstream.str() + "7D" },
            { "B0" + sstream.str() + "7E" },
            { "B0" + sstream.str() + "7F" },
        };

        atLeast1Match = false;

        for (size_t i = 0; i < msgOn.size(); i++)
        {
            LOG(INFO) << "Searching for: " << msgOn.at(i);

            if (response.find(msgOn.at(i)) != std::string::npos)
            {
                atLeast1Match = true;
                break;
            }
        }

        ASSERT_TRUE(atLeast1Match);

        atLeast1Match = false;

        for (size_t i = 0; i < msgOff.size(); i++)
        {
            LOG(INFO) << "Searching for: " << msgOff.at(i);

            if (response.find(msgOff.at(i)) != std::string::npos)
            {
                atLeast1Match = true;
                break;
            }
        }

        ASSERT_TRUE(atLeast1Match);
    }
#endif

#ifdef TEST_IO_LEDS
    LOG(INFO) << "Checking LED output";

    for (size_t i = 0; i < hwTestLEDDescriptor.size(); i++)
    {
        LOG(INFO) << "Toggling LED " << hwTestLEDDescriptor.at(i).index << " on";
        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, hwTestLEDDescriptor.at(i).index, 1));
        test::sleepMs(1000);

        LOG(INFO) << "Verifying that the LED is turned on";
        std::string ledOn = "read " + std::to_string(hwTestLEDDescriptor.at(i).pin);

#ifdef PROJECT_TARGET_LEDS_EXT_INVERT
        ledOn += " ok: 0";
#else
        ledOn += " ok: 1";
#endif

        test::wsystem("echo read " + std::to_string(hwTestLEDDescriptor.at(i).pin) + " | ssterm /dev/actl > " + temp_midi_data_location + " &", response);
        test::sleepMs(1000);

        test::wsystem("killall ssterm", response);
        while (test::wsystem("pgrep ssterm", response) == 0)
            ;

        LOG(INFO) << "Printing response";
        test::wsystem("cat " + temp_midi_data_location);

        ASSERT_EQ(0, test::wsystem("cat " + temp_midi_data_location + " | grep \"" + ledOn + "\"", response));

        LOG(INFO) << "Toggling LED " << hwTestLEDDescriptor.at(i).index << " off";
        ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, hwTestLEDDescriptor.at(i).index, 0));
        test::sleepMs(1000);

        LOG(INFO) << "Verifying that the LED is turned off";
        std::string ledOff = "read " + std::to_string(hwTestLEDDescriptor.at(i).pin);

#ifdef PROJECT_TARGET_LEDS_EXT_INVERT
        ledOff += " ok: 1";
#else
        ledOff += " ok: 0";
#endif

        test::wsystem("echo read " + std::to_string(hwTestLEDDescriptor.at(i).pin) + " | ssterm /dev/actl > " + temp_midi_data_location + " &", response);
        test::sleepMs(1000);

        test::wsystem("killall ssterm", response);
        while (test::wsystem("pgrep ssterm", response) == 0)
            ;

        LOG(INFO) << "Printing response";
        test::wsystem("cat " + temp_midi_data_location);

        ASSERT_EQ(0, test::wsystem("cat " + temp_midi_data_location + " | grep \"" + ledOff + "\"", response));
    }
#endif
}
#endif

#endif
#endif