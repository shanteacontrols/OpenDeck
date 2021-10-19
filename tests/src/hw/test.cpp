#ifndef USB_LINK_MCU
#ifdef HW_TESTING

#include "unity/Framework.h"
#include <string>
#include <filesystem>
#include "helpers/Misc.h"
#include "helpers/MIDI.h"
#include "helpers/Serial.h"
#include "application/database/Database.h"
#include "stubs/database/DB_ReadWrite.h"
#include <HWTestDefines.h>
#include <glog/logging.h>

namespace
{
    const std::string flash_cmd = "make -C ../src flash";

    enum class powerCycleType_t : uint8_t
    {
        standard,
        standardWithDeviceCheck
    };

    const std::string handshake_req            = "F0 00 53 43 00 00 01 F7";
    const std::string reboot_req               = "F0 00 53 43 00 00 7F F7";
    const std::string handshake_ack            = "F0 00 53 43 01 00 01 F7";
    const std::string factory_reset_req        = "F0 00 53 43 00 00 44 F7";
    const std::string btldr_req                = "F0 00 53 43 00 00 55 F7";
    const std::string backup_req               = "F0 00 53 43 00 00 1B F7";
    const std::string usb_power_off_cmd        = "uhubctl -a off -l 1-1 > /dev/null";
    const std::string usb_power_on_cmd         = "uhubctl -a on -l 1-1 > /dev/null";
    const std::string sysex_fw_update_delay_ms = "5";
    const uint32_t    startup_delay_ms         = 10000;
    const std::string fw_build_dir             = "../src/build/merged/";
    const std::string fw_build_type_subdir     = "release/";
    const std::string temp_midi_data_location  = "/tmp/temp_midi_data";
    const std::string backup_file_location     = "/tmp/backup.txt";

    DBstorageMock dbStorageMock;
    Database      database = Database(dbStorageMock, false);

    void cyclePower(powerCycleType_t powerCycleType)
    {
        auto cycle = [&]() {
            LOG(INFO) << "Turning USB devices off";
            TEST_ASSERT_EQUAL_INT(0, test::wsystem(usb_power_off_cmd));

            LOG(INFO) << "Turning USB devices on";
            TEST_ASSERT_EQUAL_INT(0, test::wsystem(usb_power_on_cmd));

            test::sleepMs(startup_delay_ms);
        };

        if (powerCycleType != powerCycleType_t::standardWithDeviceCheck)
        {
            LOG(INFO) << "Cycling power without device check";
            cycle();
        }
        else
        {
            LOG(INFO) << "Cycling power with device check";

            // ensure the device is present
            do
            {
                cycle();
            } while (!MIDIHelper::devicePresent());
        }
    }

    void reboot()
    {
        LOG(INFO) << "Reboting the board";

        TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
        MIDIHelper::sendRawSysEx(reboot_req, false);
        test::sleepMs(startup_delay_ms);

        if (!MIDIHelper::devicePresent())
        {
            LOG(ERROR) << "OpenDeck device not found after reboot, attempting power cycle";
            cyclePower(powerCycleType_t::standardWithDeviceCheck);

            if (!MIDIHelper::devicePresent())
            {
                LOG(ERROR) << "OpenDeck device not found after power cycle";
                exit(1);
            }
        }
        else
        {
            LOG(INFO) << "Board rebooted";
        }

        TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
        MIDIHelper::flush();
    }

    void factoryReset()
    {
        LOG(INFO) << "Performing factory reset";

        TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
        MIDIHelper::sendRawSysEx(factory_reset_req, false);

        test::sleepMs(startup_delay_ms);

        if (!MIDIHelper::devicePresent())
        {
            LOG(ERROR) << "OpenDeck device not found after factory reset, attempting power cycle";
            cyclePower(powerCycleType_t::standardWithDeviceCheck);

            if (!MIDIHelper::devicePresent())
            {
                LOG(ERROR) << "OpenDeck device not found after power cycle";
                exit(1);
            }
        }
        else
        {
            LOG(INFO) << "Factory reset complete";
        }

        TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
        MIDIHelper::flush();
    }

    void bootloaderMode()
    {
        LOG(INFO) << "Entering bootloader mode";

        TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
        MIDIHelper::sendRawSysEx(btldr_req, false);

        test::sleepMs(startup_delay_ms);

        if (!MIDIHelper::devicePresent(true))
        {
            LOG(ERROR) << "OpenDeck DFU device not found after bootloader request";
            exit(1);
        }
        else
        {
            LOG(INFO) << "Entered bootloader mode";
        }
    }

    void flash()
    {
        auto flash = [](std::string target, std::string port) {
            int result;

            std::string flashTarget = " TARGET=" + target;
            std::string flashPort   = " PORT=" + port;

            do
            {
                LOG(INFO) << "Flashing the board";
                result = test::wsystem(flash_cmd + flashTarget + flashPort);

                if (result)
                {
                    LOG(ERROR) << "Flashing failed - repeating";
                    cyclePower(powerCycleType_t::standard);
                }
                else
                {
                    LOG(INFO) << "Flashing sucessful";
                }
            } while (result);
        };

        flash(std::string(BOARD_STRING), std::string(FLASH_PORT));

#ifndef USB_SUPPORTED
        LOG(INFO) << "Flashing USB Link MCU";
        flash(std::string(USB_LINK_TARGET), std::string(FLASH_PORT_USB_LINK));
#endif

        LOG(INFO) << "Waiting " << startup_delay_ms << " ms after flashing...";
        test::sleepMs(startup_delay_ms);

        cyclePower(powerCycleType_t::standardWithDeviceCheck);
    }
}    // namespace

TEST_SETUP()
{
    LOG(INFO) << "Setting up test";

    // dummy db - used only to retrieve correct amount of supported presets
    TEST_ASSERT(database.init() == true);
    cyclePower(powerCycleType_t::standard);
    MIDIHelper::flush();
}

TEST_TEARDOWN()
{
    LOG(INFO) << "Tearing down test";

    test::wsystem("rm -f " + temp_midi_data_location);
    test::wsystem("rm -f " + backup_file_location);
    test::wsystem("killall amidi > /dev/null 2>&1");
    test::wsystem("killall olad > /dev/null 2>&1");
}

#ifdef TEST_FLASHING
TEST_CASE(FlashAndBoot)
{
    flash();
}
#endif

TEST_CASE(DatabaseInitialValues)
{
    constexpr size_t PARAM_SKIP = 2;

    factoryReset();

    // check only first and the last preset
    for (int preset = 0; preset < database.getSupportedPresets(); preset += (database.getSupportedPresets() - 1))
    {
        LOG(INFO) << "Checking initial values for preset " << preset + 1;
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::presets, 0, preset) == true);
        TEST_ASSERT_EQUAL_UINT32(preset, MIDIHelper::readFromBoard(System::Section::global_t::presets, 0));

        // MIDI block
        //----------------------------------
        // feature section
        // all values should be set to 0
        for (size_t i = 0; i < static_cast<uint8_t>(System::midiFeature_t::AMOUNT); i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::global_t::midiFeatures, i));

        // merge section
        // all values should be set to 0
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::global_t::midiMerge, static_cast<size_t>(System::midiMerge_t::mergeType)));
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::global_t::midiMerge, static_cast<size_t>(System::midiMerge_t::mergeUSBchannel)));
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::global_t::midiMerge, static_cast<size_t>(System::midiMerge_t::mergeDINchannel)));

        // button block
        //----------------------------------
        // type section
        // all values should be set to 0 (default type)
        for (size_t i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::button_t::type, i));

        // midi message section
        // all values should be set to 0 (default/note)
        for (size_t i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::button_t::midiMessage, i));

        // midi id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        //(physical/analog/touchscreen)
        for (size_t i = 0; i < 1; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(i, MIDIHelper::readFromBoard(System::Section::button_t::midiID, i));

        for (size_t i = 0; i < MAX_NUMBER_OF_ANALOG; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(i, MIDIHelper::readFromBoard(System::Section::button_t::midiID, MAX_NUMBER_OF_BUTTONS + i));

        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(i, MIDIHelper::readFromBoard(System::Section::button_t::midiID, MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + i));

        // midi velocity section
        // all values should be set to 127
        for (size_t i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(127, MIDIHelper::readFromBoard(System::Section::button_t::velocity, i));

        // midi channel section
        // all values should be set to 0
        // note: midi channels are in range 1-16 via sysex and written in range 0-15 in db
        for (size_t i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::button_t::midiChannel, i));

        // encoders block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_ENCODERS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::encoder_t::enable, i));

        // invert section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_ENCODERS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::encoder_t::invert, i));

        // mode section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_ENCODERS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::encoder_t::mode, i));

        // midi id section
        // incremental values - first value should be set to MAX_NUMBER_OF_ANALOG, each successive value should be incremented by 1
        for (size_t i = 0; i < MAX_NUMBER_OF_ENCODERS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ANALOG + i, MIDIHelper::readFromBoard(System::Section::encoder_t::midiID, i));

        // midi channel section
        // all values should be set to 0
        // note: midi channels are in range 1-16 via sysex and written in range 0-15 in db
        for (size_t i = 0; i < MAX_NUMBER_OF_ENCODERS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::encoder_t::midiChannel, i));

        // pulses per step section
        // all values should be set to 4
        for (size_t i = 0; i < MAX_NUMBER_OF_ENCODERS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(4, MIDIHelper::readFromBoard(System::Section::encoder_t::pulsesPerStep, i));

        // analog block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_ANALOG; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::analog_t::enable, i));

        // invert section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_ANALOG; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::analog_t::invert, i));

        // type section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_ANALOG; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::analog_t::invert, i));

        // midi id section
        // incremental values - first value should be set to 0, each successive value should be incremented by 1
        for (size_t i = 0; i < MAX_NUMBER_OF_ANALOG; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(i, MIDIHelper::readFromBoard(System::Section::analog_t::midiID, i));

        // lower limit section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_ANALOG; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::analog_t::lowerLimit, i));

        // upper limit section
        // all values should be set to 16383
        for (size_t i = 0; i < MAX_NUMBER_OF_ANALOG; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(16383, MIDIHelper::readFromBoard(System::Section::analog_t::upperLimit, i));

        // midi channel section
        // all values should be set to 0
        // note: midi channels are in range 1-16 via sysex and written in range 0-15 in db
        for (size_t i = 0; i < MAX_NUMBER_OF_ANALOG; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::analog_t::midiChannel, i));

        // LED block
        //----------------------------------
        // global section
        // all values should be set to 0
        for (size_t i = 0; i < static_cast<uint8_t>(IO::LEDs::setting_t::AMOUNT); i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::leds_t::global, i));

        // activation id section
        // incremental values - first value should be set to 0, each successive value should be incremented by 1 for each group
        //(physical/touchscreen)
        for (size_t i = 0; i < MAX_NUMBER_OF_LEDS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(i, MIDIHelper::readFromBoard(System::Section::leds_t::activationID, i));

        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(i, MIDIHelper::readFromBoard(System::Section::leds_t::activationID, MAX_NUMBER_OF_LEDS + i));

        // rgb enable section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_RGB_LEDS + (MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS / 3); i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::leds_t::rgbEnable, i));

        // control type section
        // all values should be set to midiInNoteMultiVal
        for (size_t i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(IO::LEDs::controlType_t::midiInNoteMultiVal), MIDIHelper::readFromBoard(System::Section::leds_t::controlType, i));

        // activation value section
        // all values should be set to 127
        for (size_t i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(127, MIDIHelper::readFromBoard(System::Section::leds_t::activationValue, i));

        // midi channel section
        // all values should be set to 0
        // note: midi channels are in range 1-16 via sysex and written in range 0-15 in db
        for (size_t i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::leds_t::midiChannel, i));

#ifdef DISPLAY_SUPPORTED
        // display block
        //----------------------------------
        // feature section
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::display_t::features, static_cast<size_t>(IO::Display::feature_t::enable)));
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::display_t::features, static_cast<size_t>(IO::Display::feature_t::welcomeMsg)));
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::display_t::features, static_cast<size_t>(IO::Display::feature_t::vInfoMsg)));
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::display_t::features, static_cast<size_t>(IO::Display::feature_t::MIDInotesAlternate)));

        // setting section
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::display_t::setting, static_cast<size_t>(IO::Display::setting_t::MIDIeventTime)));
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::display_t::setting, static_cast<size_t>(IO::Display::setting_t::octaveNormalization)));
#endif

#ifdef TOUCHSCREEN_SUPPORTED
        // touchscreen block
        //----------------------------------
        // setting section
        // all values should be set to 0
        for (size_t i = 0; i < static_cast<uint8_t>(IO::Touchscreen::setting_t::AMOUNT); i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::setting, i));

        // x position section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::xPos, i));

        // y position section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::yPos, i));

        // width section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::width, i));

        // height section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::height, i));

        // on screen section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::onScreen, i));

        // off screen section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::offScreen, i));

        // page switch enabled section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::pageSwitchEnabled, i));

        // page switch index section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::pageSwitchIndex, i));

        // analog page section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogPage, i));

        // analog start x coordinate section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogStartXCoordinate, i));

        // analog end x coordinate section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogEndXCoordinate, i));

        // analog start y coordinate section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogStartYCoordinate, i));

        // analog end y coordinate section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogEndYCoordinate, i));

        // analog type section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogType, i));

        // analog reset on release section
        // all values should be set to 0
        for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i += PARAM_SKIP)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogResetOnRelease, i));
#endif
    }
}

TEST_CASE(FwUpdate)
{
    std::string syxPath = fw_build_dir + BOARD_STRING + "/" + fw_build_type_subdir + BOARD_STRING + ".sysex.syx";

    if (!std::filesystem::exists(syxPath))
    {
        LOG(ERROR) << ".syx file not found, aborting";
        exit(1);
    }

    bootloaderMode();

    LOG(INFO) << "Sending firmware file to the board";
    std::string cmd = std::string("amidi -p ") + MIDIHelper::amidiPort(true) + " -s " + syxPath + " -i " + sysex_fw_update_delay_ms;
    TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd));
    LOG(INFO) << "Firmware file sent sucessfully, waiting " << startup_delay_ms << " ms";
    test::sleepMs(startup_delay_ms);

    if (!MIDIHelper::devicePresent())
    {
        LOG(ERROR) << "OpenDeck device not found after firmware update, aborting";
        exit(1);
    }
}

TEST_CASE(BackupAndRestore)
{
    factoryReset();

    LOG(INFO) << "Setting few random values in each available preset";

    for (int preset = 0; preset < database.getSupportedPresets(); preset++)
    {
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::presets, static_cast<int>(System::presetSetting_t::activePreset), preset) == true);
        TEST_ASSERT_EQUAL_UINT32(preset, MIDIHelper::readFromBoard(System::Section::global_t::presets, static_cast<int>(System::presetSetting_t::activePreset)));

        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::analog_t::midiID, 4, 15 + preset) == true);
#ifdef ENCODERS_SUPPORTED
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::encoder_t::midiChannel, 1, 2 + preset) == true);
#endif
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::button_t::velocity, 0, 90 + preset) == true);

        TEST_ASSERT_EQUAL_UINT32(15 + preset, MIDIHelper::readFromBoard(System::Section::analog_t::midiID, 4));
#ifdef ENCODERS_SUPPORTED
        TEST_ASSERT_EQUAL_UINT32(2 + preset, MIDIHelper::readFromBoard(System::Section::encoder_t::midiChannel, 1));
#endif
        TEST_ASSERT_EQUAL_UINT32(90 + preset, MIDIHelper::readFromBoard(System::Section::button_t::velocity, 0));
    }

    LOG(INFO) << "Sending backup request";
    std::string cmd = std::string("amidi -p ") + MIDIHelper::amidiPort() + " -S \"" + backup_req + "\" -d -t 5 > " + backup_file_location;
    TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd));

    factoryReset();

    LOG(INFO) << "Verifying that the default values are active again";

    for (int preset = 0; preset < database.getSupportedPresets(); preset++)
    {
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::presets, static_cast<int>(System::presetSetting_t::activePreset), preset) == true);
        TEST_ASSERT_EQUAL_UINT32(preset, MIDIHelper::readFromBoard(System::Section::global_t::presets, static_cast<int>(System::presetSetting_t::activePreset)));

        TEST_ASSERT_EQUAL_UINT32(4, MIDIHelper::readFromBoard(System::Section::analog_t::midiID, 4));
#ifdef ENCODERS_SUPPORTED
        TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::encoder_t::midiChannel, 1));
#endif
        TEST_ASSERT_EQUAL_UINT32(127, MIDIHelper::readFromBoard(System::Section::button_t::velocity, 0));
    }

    LOG(INFO) << "Restoring backup";

    // remove everything before the first line containing F0 00 53 43 01 00 1B F7
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("sed -i '0,/^F0 00 53 43 01 00 1B F7$/d' " + backup_file_location));

    //...and also after the last line containing the same
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("sed -i '/^F0 00 53 43 01 00 1B F7$/Q' " + backup_file_location));

    // send backup
    std::ifstream backupStream(backup_file_location);
    std::string   line;

    while (getline(backupStream, line))
    {
        TEST_ASSERT(MIDIHelper::sendRawSysEx(line) != std::string(""));
    }

    LOG(INFO) << "Verifying that the custom values are active again";

    for (int preset = 0; preset < database.getSupportedPresets(); preset++)
    {
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::presets, static_cast<int>(System::presetSetting_t::activePreset), preset) == true);
        TEST_ASSERT_EQUAL_UINT32(preset, MIDIHelper::readFromBoard(System::Section::global_t::presets, static_cast<int>(System::presetSetting_t::activePreset)));

        TEST_ASSERT_EQUAL_UINT32(15 + preset, MIDIHelper::readFromBoard(System::Section::analog_t::midiID, 4));
#ifdef ENCODERS_SUPPORTED
        TEST_ASSERT_EQUAL_UINT32(2 + preset, MIDIHelper::readFromBoard(System::Section::encoder_t::midiChannel, 1));
#endif
        TEST_ASSERT_EQUAL_UINT32(90 + preset, MIDIHelper::readFromBoard(System::Section::button_t::velocity, 0));
    }
}

TEST_CASE(MIDIData)
{
    std::string cmd;
    std::string response;

    factoryReset();

    auto changePreset = [&](bool redirect) {
        LOG(INFO) << "Switching preset";

        if (redirect)
            cmd = std::string("amidi -p ") + MIDIHelper::amidiPort() + " -S \"F0 00 53 43 00 00 01 00 00 02 00 00 00 00 F7\" -d -t 5 > " + temp_midi_data_location;
        else
            cmd = std::string("amidi -p ") + MIDIHelper::amidiPort() + " -S \"F0 00 53 43 00 00 01 00 00 02 00 00 00 00 F7\" -d -t 2";

        TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd, response));
    };

    changePreset(true);

    // drop empty lines
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("sed -i '/^$/d' " + temp_midi_data_location));

    // verify line count
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("grep -c . " + temp_midi_data_location, response));

    // raspberry pi has slow USB MIDI and drops some packets
    // half of expected results is okay
    TEST_ASSERT(stoi(response) >= (MAX_NUMBER_OF_BUTTONS / 2));
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("rm " + temp_midi_data_location));

    // run the same test for DIN MIDI

    reboot();

#ifdef DIN_MIDI_SUPPORTED
#ifdef TEST_DIN_MIDI_PORT
    auto monitor = [&]() {
        LOG(INFO) << "Monitoring DIN MIDI interface";
        cmd = std::string("amidi -p $(amidi -l | grep -E '") + DIN_MIDI_PORT + std::string("' | grep -Eo 'hw:\\S*')") + " -d > " + temp_midi_data_location + " &";
        TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd));
    };

    monitor();
    changePreset(false);

    test::wsystem("killall amidi", response);

    LOG(INFO) << "Verifying that no data reached DIN MIDI interface";
    test::wsystem("grep -c . " + temp_midi_data_location, response);
    TEST_ASSERT_EQUAL_INT(0, stoi(response));
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("rm " + temp_midi_data_location));

    LOG(INFO) << "Enabling DIN MIDI";
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled), 1) == true);
    monitor();
    changePreset(false);

    LOG(INFO) << "Verifying that the data has now reached DIN MIDI interface";
    test::wsystem("sleep 3 && killall amidi > /dev/null");
    test::wsystem("cat " + temp_midi_data_location + " | xargs | sed 's/ /&\\n/3;P;D'", response);
    LOG(INFO) << "Received DIN MIDI messages:\n"
              << response;

    test::wsystem("echo \"" + response + "\" | grep -c .", response);
    test::wsystem("grep -c . " + temp_midi_data_location, response);
    LOG(INFO) << "Total number of received DIN MIDI messages: " << response;
    TEST_ASSERT(stoi(response) >= (MAX_NUMBER_OF_BUTTONS / 2));
#endif
#endif
}

#ifdef DMX_SUPPORTED
TEST_CASE(DMX)
{
    factoryReset();

    LOG(INFO) << "Starting OLA daemon";
    test::wsystem("olad -f --no-http");
    test::sleepMs(3000);

    auto verify = [](bool state) {
        std::string    cmd           = "ola_dev_info | grep -q " + std::string(BOARD_STRING);
        const uint32_t waitTimeMs    = 1000;
        const uint32_t stopWaitAfter = 22000;    // ola searches after 20sec, 2 more just in case
        uint32_t       totalWaitTime = 0;

        while (test::wsystem(cmd))
        {
            test::sleepMs(waitTimeMs);
            totalWaitTime += waitTimeMs;

            if (totalWaitTime == stopWaitAfter)
            {
                break;
            }
        }

        if (!state)
        {
            if (totalWaitTime == stopWaitAfter)
            {
                LOG(INFO) << "OLA didn't detect the board while DMX was disabled";
                return true;
            }
            else
            {
                LOG(ERROR) << "OLA detected the board while DMX was disabled";
                return false;
            }
        }
        else
        {
            if (totalWaitTime == stopWaitAfter)
            {
                LOG(ERROR) << "OLA didn't detect the board while DMX was enabled";
                return false;
            }
            else
            {
                LOG(INFO) << "OLA detected the board while DMX was enabled";
                return true;
            }
        }
    };

    LOG(INFO) << "DMX is disabled, checking if the board isn't detectable by OLA";
    TEST_ASSERT(verify(false));

    LOG(INFO) << "Enabling DMX";
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::dmx, static_cast<int>(System::dmxSetting_t::enabled), 1) == true);

    LOG(INFO) << "Checking if the board is detectable by OLA";
    TEST_ASSERT(verify(true));

    // midi part should remain functional as well
    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
}
#endif

#endif
#endif