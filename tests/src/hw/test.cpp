#ifdef HW_TESTING

#include "unity/Framework.h"
#include <string>
#include <filesystem>
#include "helpers/Misc.h"
#include "helpers/MIDI.h"
#include "application/database/Database.h"
#include "stubs/database/DB_ReadWrite.h"

namespace
{
    const std::string flash_cmd = "gdb -nx --batch \
    -ex 'target extended-remote /dev/ttyBmpGdb' \
    -ex 'monitor swdp_scan' \
    -ex 'attach 1' \
    -ex 'load' \
    -ex 'compare-sections' \
    -ex 'kill' ";

    const std::string bmp_dev_vid_pid          = "1d50:6018";
    const std::string opendeck_dev_vid_pid     = "1209:8472";
    const std::string opendeck_dfu_vid_pid     = "1209:8473";
    const std::string handshake_req            = "F0 00 53 43 00 00 01 F7";
    const std::string handshake_ack            = "F0 00 53 43 01 00 01 F7";
    const std::string factory_reset_req        = "F0 00 53 43 00 00 44 F7";
    const std::string btldr_req                = "F0 00 53 43 00 00 55 F7";
    const std::string backup_req               = "F0 00 53 43 00 00 1B F7";
    const std::string usb_power_off_cmd        = "uhubctl -a off -l 1-1";
    const std::string usb_power_on_cmd         = "uhubctl -a on -l 1-1";
    const std::string sysex_fw_update_delay_ms = "5";
    const std::string sysex_restore_delay_ms   = "100";
    const std::string startup_delay_s          = "5";
    const std::string rapid_reboot_repeat_s    = "1.2";
    const std::string fw_build_dir             = "../src/build/merged/";
    const std::string fw_build_type_subdir     = "release/";

    DBstorageMock dbStorageMock;
    Database      database = Database(dbStorageMock, false);
}    // namespace

TEST_SETUP()
{
    //verify that black magic probe is connected to system
    std::string response;
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("lsusb -v 2>/dev/null | grep -q '" + bmp_dev_vid_pid + "'", response));

    //dummy db - used only to retrieve correct amount of supported presets
    TEST_ASSERT(database.init() == true);
}

TEST_CASE(FlashAndBoot)
{
    std::string response;
    std::string hexPath = fw_build_dir + OD_BOARD + "/" + fw_build_type_subdir + OD_BOARD + ".hex";

    if (!std::filesystem::exists(hexPath))
    {
        printf("hex file not found, aborting\n");
        exit(1);
    }

    TEST_ASSERT_EQUAL_INT(0, test::wsystem(flash_cmd + hexPath, response));
    //verify that opendeck device is present now
    test::wsystem("sleep " + startup_delay_s, response);

    if (test::wsystem("lsusb -v 2>/dev/null | grep -q '" + opendeck_dev_vid_pid + "'", response) != 0)
    {
        printf("OpenDeck device not found after flashing, aborting\n");
        exit(1);
    }
}

TEST_CASE(FactoryReset)
{
    std::string response;

    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
    MIDIHelper::sendRawSysEx(factory_reset_req);

    //once factory reset is initiated, board is rebooted
    test::wsystem("sleep " + startup_delay_s, response);

    if (test::wsystem("lsusb -v 2>/dev/null | grep -q '" + opendeck_dev_vid_pid + "'", response) != 0)
    {
        printf("OpenDeck device not found after factory reset, aborting\n");
        exit(1);
    }
}

TEST_CASE(DatabaseInitialValues)
{
    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));

    for (int preset = 0; preset < database.getSupportedPresets(); preset++)
    {
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::presets, 0, preset) == true);
        TEST_ASSERT_EQUAL_UINT32(preset, MIDIHelper::readFromBoard(System::Section::global_t::presets, 0));

        //MIDI block
        //----------------------------------
        //feature section
        //all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(System::midiFeature_t::AMOUNT); i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::global_t::midiFeatures, i));

        //merge section
        //all values should be set to 0
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::global_t::midiMerge, static_cast<size_t>(System::midiMerge_t::mergeType)));
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::global_t::midiMerge, static_cast<size_t>(System::midiMerge_t::mergeUSBchannel)));
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::global_t::midiMerge, static_cast<size_t>(System::midiMerge_t::mergeDINchannel)));

        //button block
        //----------------------------------
        //type section
        //all values should be set to 0 (default type)
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::button_t::type, i));

        //midi message section
        //all values should be set to 0 (default/note)
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::button_t::midiMessage, i));

        //midi id section
        //incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        //(physical/analog/touchscreen)
        for (int i = 0; i < 1; i++)
            TEST_ASSERT_EQUAL_UINT32(i, MIDIHelper::readFromBoard(System::Section::button_t::midiID, i));

        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(i, MIDIHelper::readFromBoard(System::Section::button_t::midiID, MAX_NUMBER_OF_BUTTONS + i));

        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(i, MIDIHelper::readFromBoard(System::Section::button_t::midiID, MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + i));

        //midi velocity section
        //all values should be set to 127
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(127, MIDIHelper::readFromBoard(System::Section::button_t::velocity, i));

        //midi channel section
        //all values should be set to 0
        //note: midi channels are in range 1-16 via sysex and written in range 0-15 in db
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::button_t::midiChannel, i));

        //encoders block
        //----------------------------------
        //enable section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::encoder_t::enable, i));

        //invert section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::encoder_t::invert, i));

        //mode section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::encoder_t::mode, i));

        //midi id section
        //incremental values - first value should be set to MAX_NUMBER_OF_ANALOG, each successive value should be incremented by 1
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ANALOG + i, MIDIHelper::readFromBoard(System::Section::encoder_t::midiID, i));

        //midi channel section
        //all values should be set to 0
        //note: midi channels are in range 1-16 via sysex and written in range 0-15 in db
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::encoder_t::midiChannel, i));

        //pulses per step section
        //all values should be set to 4
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT_EQUAL_UINT32(4, MIDIHelper::readFromBoard(System::Section::encoder_t::pulsesPerStep, i));

        //analog block
        //----------------------------------
        //enable section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::analog_t::enable, i));

        //invert section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::analog_t::invert, i));

        //type section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::analog_t::invert, i));

        //midi id section
        //incremental values - first value should be set to 0, each successive value should be incremented by 1
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(i, MIDIHelper::readFromBoard(System::Section::analog_t::midiID, i));

        //lower limit section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::analog_t::lowerLimit, i));

        //upper limit section
        //all values should be set to 16383
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(16383, MIDIHelper::readFromBoard(System::Section::analog_t::upperLimit, i));

        //midi channel section
        //all values should be set to 0
        //note: midi channels are in range 1-16 via sysex and written in range 0-15 in db
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::analog_t::midiChannel, i));

        //LED block
        //----------------------------------
        //global section
        //all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(IO::LEDs::setting_t::AMOUNT); i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::leds_t::global, i));

        //activation id section
        //incremental values - first value should be set to 0, each successive value should be incremented by 1 for each group
        //(physical/touchscreen)
        for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
            TEST_ASSERT_EQUAL_UINT32(i, MIDIHelper::readFromBoard(System::Section::leds_t::activationID, i));

        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(i, MIDIHelper::readFromBoard(System::Section::leds_t::activationID, MAX_NUMBER_OF_LEDS + i));

        //rgb enable section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_RGB_LEDS + (MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS / 3); i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::leds_t::rgbEnable, i));

        //control type section
        //all values should be set to midiInNoteMultiVal
        for (int i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(IO::LEDs::controlType_t::midiInNoteMultiVal), MIDIHelper::readFromBoard(System::Section::leds_t::controlType, i));

        //activation value section
        //all values should be set to 127
        for (int i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(127, MIDIHelper::readFromBoard(System::Section::leds_t::activationValue, i));

        //midi channel section
        //all values should be set to 0
        //note: midi channels are in range 1-16 via sysex and written in range 0-15 in db
        for (int i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::leds_t::midiChannel, i));

#ifdef DISPLAY_SUPPORTED
        //display block
        //----------------------------------
        //feature section
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::display_t::features, static_cast<size_t>(IO::Display::feature_t::enable)));
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::display_t::features, static_cast<size_t>(IO::Display::feature_t::welcomeMsg)));
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::display_t::features, static_cast<size_t>(IO::Display::feature_t::vInfoMsg)));
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::display_t::features, static_cast<size_t>(IO::Display::feature_t::MIDInotesAlternate)));

        //setting section
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::display_t::setting, static_cast<size_t>(IO::Display::setting_t::MIDIeventTime)));
        TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::display_t::setting, static_cast<size_t>(IO::Display::setting_t::octaveNormalization)));
#endif

#ifdef TOUCHSCREEN_SUPPORTED
        //touchscreen block
        //----------------------------------
        //setting section
        //all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(IO::Touchscreen::setting_t::AMOUNT); i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::setting, i));

        //x position section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::xPos, i));

        //y position section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::yPos, i));

        //width section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::width, i));

        //height section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::height, i));

        //on screen section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::onScreen, i));

        //off screen section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::offScreen, i));

        //page switch enabled section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::pageSwitchEnabled, i));

        //page switch index section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::pageSwitchIndex, i));

        //analog page section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogPage, i));

        //analog start x coordinate section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogStartXCoordinate, i));

        //analog end x coordinate section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogEndXCoordinate, i));

        //analog start y coordinate section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogStartYCoordinate, i));

        //analog end y coordinate section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogEndYCoordinate, i));

        //analog type section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogType, i));

        //analog reset on release section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, MIDIHelper::readFromBoard(System::Section::touchscreen_t::analogResetOnRelease, i));
#endif
    }
}

TEST_CASE(ValuesAfterReboots)
{
    std::string response;

    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));

    //change few random values
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::presets, 0, 1) == true);    //active preset 1
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::presets, 1, 1) == true);    //preserve preset
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::analog_t::midiID, 4, 15) == true);
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::encoder_t::pulsesPerStep, 1, 2) == true);
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::button_t::velocity, 0, 126) == true);

    auto verify = []() {
        TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::global_t::presets, 0));
        TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::global_t::presets, 1));
        TEST_ASSERT_EQUAL_UINT32(15, MIDIHelper::readFromBoard(System::Section::analog_t::midiID, 4));
        TEST_ASSERT_EQUAL_UINT32(2, MIDIHelper::readFromBoard(System::Section::encoder_t::pulsesPerStep, 1));
        TEST_ASSERT_EQUAL_UINT32(126, MIDIHelper::readFromBoard(System::Section::button_t::velocity, 0));
    };

    verify();

    //now perform several rapid reboots
    //note: in the past these kind of reboots have been known to cause factory reset on the board
    for (size_t i = 0; i < 10; i++)
    {
        TEST_ASSERT_EQUAL_INT(0, test::wsystem(usb_power_off_cmd, response));
        test::wsystem("sleep " + rapid_reboot_repeat_s, response);

        TEST_ASSERT_EQUAL_INT(0, test::wsystem(usb_power_on_cmd, response));
        test::wsystem("sleep " + rapid_reboot_repeat_s, response);
    }

    test::wsystem("sleep " + startup_delay_s, response);
    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));

    //verify all values again
    verify();
}

TEST_CASE(FwUpdate)
{
    std::string response;
    std::string syxPath = fw_build_dir + OD_BOARD + "/" + fw_build_type_subdir + OD_BOARD + ".sysex.syx";

    if (!std::filesystem::exists(syxPath))
    {
        printf(".syx file not found, aborting\n");
        exit(1);
    }

    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
    MIDIHelper::sendRawSysEx(btldr_req);

    test::wsystem("sleep " + startup_delay_s, response);

    if (test::wsystem("lsusb -v 2>/dev/null | grep -q '" + opendeck_dfu_vid_pid + "'", response) != 0)
    {
        printf("OpenDeck DFU device not found after bootloader request, aborting\n");
        exit(1);
    }

    //perform fw update
    std::string cmd = std::string("amidi -p $(amidi -l | grep -E 'OpenDeck DFU'") + std::string(" | grep -Eo 'hw:\\S*') -s ") + syxPath + " -i " + sysex_fw_update_delay_ms;
    TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd, response));

    test::wsystem("sleep " + startup_delay_s, response);

    if (test::wsystem("lsusb -v 2>/dev/null | grep -q '" + opendeck_dev_vid_pid + "'", response) != 0)
    {
        printf("OpenDeck device not found after firmware update, aborting\n");
        exit(1);
    }
}

TEST_CASE(BackupAndRestore)
{
    std::string response;

    auto factoryReset = [&]() {
        TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));

        //first, perform factory reset
        MIDIHelper::sendRawSysEx(factory_reset_req);

        //once factory reset is initiated, board is rebooted
        test::wsystem("sleep " + startup_delay_s, response);

        if (test::wsystem("lsusb -v 2>/dev/null | grep -q '" + opendeck_dev_vid_pid + "'", response) != 0)
        {
            printf("OpenDeck device not found after factory reset, aborting\n");
            exit(1);
        }

        TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
    };

    factoryReset();

    std::vector<uint8_t> presets = { 1, 2, 3 };

    for (size_t i = 0; i < presets.size(); i++)
    {
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::presets, static_cast<int>(System::presetSetting_t::activePreset), presets.at(i)) == true);
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::analog_t::midiID, 4, 15) == true);
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::encoder_t::pulsesPerStep, 1, 2) == true);
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::button_t::velocity, 0, 126) == true);

        TEST_ASSERT_EQUAL_UINT32(presets.at(i), MIDIHelper::readFromBoard(System::Section::global_t::presets, static_cast<int>(System::presetSetting_t::activePreset)));
        TEST_ASSERT_EQUAL_UINT32(15, MIDIHelper::readFromBoard(System::Section::analog_t::midiID, 4));
        TEST_ASSERT_EQUAL_UINT32(2, MIDIHelper::readFromBoard(System::Section::encoder_t::pulsesPerStep, 1));
        TEST_ASSERT_EQUAL_UINT32(126, MIDIHelper::readFromBoard(System::Section::button_t::velocity, 0));
    }

    std::string cmd = std::string("amidi -p $(amidi -l | grep -E 'OpenDeck'") + std::string(" | grep -Eo 'hw:\\S*') -S ") + "'" + backup_req + "'" + " -d -t 5 > backup.txt";
    TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd, response));

    factoryReset();

    //verify that the defaults are active again
    for (size_t i = 0; i < presets.size(); i++)
    {
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::presets, static_cast<int>(System::presetSetting_t::activePreset), presets.at(i)) == true);
        TEST_ASSERT_EQUAL_UINT32(presets.at(i), MIDIHelper::readFromBoard(System::Section::global_t::presets, static_cast<int>(System::presetSetting_t::activePreset)));

        TEST_ASSERT_EQUAL_UINT32(4, MIDIHelper::readFromBoard(System::Section::analog_t::midiID, 4));
        TEST_ASSERT_EQUAL_UINT32(4, MIDIHelper::readFromBoard(System::Section::encoder_t::pulsesPerStep, 1));
        TEST_ASSERT_EQUAL_UINT32(127, MIDIHelper::readFromBoard(System::Section::button_t::velocity, 0));
    }

    //now restore backup

    //remove all newlines first from created backup
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("xargs echo < backup.txt > backup.txt.temp", response));

    //reverse hexdump - write all ascii bytes as raw bytes into the new file
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("cat backup.txt.temp | xxd -r -p > backup.syx", response));
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("rm backup.txt.temp backup.txt", response));

    //send backup
    cmd = std::string("amidi -p $(amidi -l | grep -E 'OpenDeck'") + std::string(" | grep -Eo 'hw:\\S*') -s backup.syx -i ") + sysex_restore_delay_ms + " -d -t 5";
    TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd, response));

    //verify that the custom values are active again
    for (size_t i = 0; i < presets.size(); i++)
    {
        TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::presets, static_cast<int>(System::presetSetting_t::activePreset), presets.at(i)) == true);
        TEST_ASSERT_EQUAL_UINT32(presets.at(i), MIDIHelper::readFromBoard(System::Section::global_t::presets, static_cast<int>(System::presetSetting_t::activePreset)));

        TEST_ASSERT_EQUAL_UINT32(15, MIDIHelper::readFromBoard(System::Section::analog_t::midiID, 4));
        TEST_ASSERT_EQUAL_UINT32(2, MIDIHelper::readFromBoard(System::Section::encoder_t::pulsesPerStep, 1));
        TEST_ASSERT_EQUAL_UINT32(126, MIDIHelper::readFromBoard(System::Section::button_t::velocity, 0));
    }
}

#endif