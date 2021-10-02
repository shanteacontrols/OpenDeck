#ifdef HW_TESTING

#include "unity/Framework.h"
#include <string>
#include <filesystem>
#include "helpers/Misc.h"
#include "helpers/MIDI.h"
#include "helpers/Serial.h"
#include "application/database/Database.h"
#include "stubs/database/DB_ReadWrite.h"

namespace
{
    const std::string flash_cmd = "make -C ../src flash";

    enum class powerCycleType_t : uint8_t
    {
        standard,
        standardWithDeviceCheck
    };

    const std::string handshake_req             = "F0 00 53 43 00 00 01 F7";
    const std::string reboot_req                = "F0 00 53 43 00 00 7F F7";
    const std::string handshake_ack             = "F0 00 53 43 01 00 01 F7";
    const std::string factory_reset_req         = "F0 00 53 43 00 00 44 F7";
    const std::string btldr_req                 = "F0 00 53 43 00 00 55 F7";
    const std::string backup_req                = "F0 00 53 43 00 00 1B F7";
    const std::string usb_power_off_cmd         = "uhubctl -a off -l 1-1.4.4 > /dev/null && sleep 2 && uhubctl -a off -l 1-1.4 > /dev/null && sleep 2 && uhubctl -a off -l 1-1 > /dev/null";
    const std::string usb_power_on_cmd          = "uhubctl -a on -l 1-1 > /dev/null && sleep 2 && uhubctl -a on -l 1-1.4 > /dev/null && sleep 2 && uhubctl -a on -l 1-1.4 > /dev/null";
    const std::string sysex_fw_update_delay_ms  = "5";
    const std::string startup_delay_s           = "10";
    const std::string fw_build_dir              = "../src/build/merged/";
    const std::string fw_build_type_subdir      = "release/";
    const std::string temp_midi_data_location   = "/tmp/temp_midi_data";
    const std::string backup_file_location      = "/tmp/backup.txt";
    const std::string stm_flash_port            = "ttyBmpGdb";
    const std::string avr_flash_port_mega2560   = "ttyACM2";
    const std::string avr_flash_port_mega16u2   = "ttyACM3";
    const std::string avr_serial_port           = "ttyUSB0";
    const std::string opendeck2_dmx_serial_port = "ttyACM4";
    const std::string mega2560_dmx_serial_port  = "ttyACM5";

    DBstorageMock dbStorageMock;
    Database      database = Database(dbStorageMock, false);

    void reboot(bool sendHandshake = true)
    {
        TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
        MIDIHelper::sendRawSysEx(reboot_req);
        test::wsystem("sleep " + startup_delay_s);

        if (!MIDIHelper::devicePresent())
        {
            printf("OpenDeck device not found after reboot, aborting\n");
            exit(1);
        }

        if (sendHandshake)
            TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
    }

    void factoryReset()
    {
        TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
        MIDIHelper::sendRawSysEx(factory_reset_req);

        test::wsystem("sleep " + startup_delay_s);

        if (!MIDIHelper::devicePresent())
        {
            printf("OpenDeck device not found after factory reset, aborting\n");
            exit(1);
        }
    }

    void bootloaderMode()
    {
        TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
        MIDIHelper::sendRawSysEx(btldr_req);

        test::wsystem("sleep " + startup_delay_s);

        if (!MIDIHelper::devicePresent(true))
        {
            printf("OpenDeck DFU device not found after bootloader request, aborting\n");
            exit(1);
        }
    }

    void cyclePower(powerCycleType_t powerCycleType)
    {
        auto cycle = [&]() {
            TEST_ASSERT_EQUAL_INT(0, test::wsystem(usb_power_off_cmd));
            TEST_ASSERT_EQUAL_INT(0, test::wsystem(usb_power_on_cmd));

            test::wsystem("sleep " + startup_delay_s);
        };

        if (powerCycleType != powerCycleType_t::standardWithDeviceCheck)
        {
            cycle();
        }
        else
        {
            //ensure the device is present
            do
            {
                cycle();
            } while (!MIDIHelper::devicePresent());
        }
    }

    void flash()
    {
#ifdef STM32_EMU_EEPROM
        std::string flashTarget = " TARGET=" + std::string(BOARD_STRING);
        std::string flashPort   = " PORT=" + stm_flash_port;
        TEST_ASSERT_EQUAL_INT(0, test::wsystem(flash_cmd + flashTarget + flashPort));
#else
        std::string flashTarget_mega2560 = " TARGET=mega2560";
        std::string flashTarget_mega16u2 = " TARGET=mega16u2";
        std::string flashPort_mega2560   = " PORT=" + avr_flash_port_mega2560;
        std::string flashPort_mega16u2   = " PORT=" + avr_flash_port_mega16u2;

        TEST_ASSERT_EQUAL_INT(0, test::wsystem(flash_cmd + flashTarget_mega2560 + flashPort_mega2560));
        TEST_ASSERT_EQUAL_INT(0, test::wsystem(flash_cmd + flashTarget_mega16u2 + flashPort_mega16u2));
#endif

        //delay some time to allow eeprom init
        test::wsystem("sleep " + startup_delay_s);

        cyclePower(powerCycleType_t::standardWithDeviceCheck);
    }
}    // namespace

TEST_SETUP()
{
    //dummy db - used only to retrieve correct amount of supported presets
    TEST_ASSERT(database.init() == true);
    cyclePower(powerCycleType_t::standard);
    MIDIHelper::flush();
}

TEST_TEARDOWN()
{
    test::wsystem("rm -f " + temp_midi_data_location);
    test::wsystem("rm -f " + backup_file_location);
    test::wsystem("killall amidi > /dev/null 2>&1");
}

#ifdef HW_TEST_FLASH
TEST_CASE(FlashAndBoot)
{
    flash();
}
#endif

TEST_CASE(DatabaseInitialValues)
{
#ifndef HW_TEST_FLASH
    cyclePower(powerCycleType_t::standardWithDeviceCheck);
#endif

    factoryReset();
    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));

    //check only first and the last preset
    for (int preset = 0; preset < database.getSupportedPresets(); preset += (database.getSupportedPresets() - 1))
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

TEST_CASE(ValuesAfterFlashing)
{
    factoryReset();
    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));

    //change few random values
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::presets, 0, 1) == true);    //active preset 1
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::presets, 1, 1) == true);    //preserve preset
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::analog_t::midiID, 4, 15) == true);
#ifdef ENCODERS_SUPPORTED
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::encoder_t::pulsesPerStep, 1, 2) == true);
#endif
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::button_t::velocity, 0, 126) == true);

    auto verify = []() {
        TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::global_t::presets, 0));
        TEST_ASSERT_EQUAL_UINT32(1, MIDIHelper::readFromBoard(System::Section::global_t::presets, 1));
        TEST_ASSERT_EQUAL_UINT32(15, MIDIHelper::readFromBoard(System::Section::analog_t::midiID, 4));
#ifdef ENCODERS_SUPPORTED
        TEST_ASSERT_EQUAL_UINT32(2, MIDIHelper::readFromBoard(System::Section::encoder_t::pulsesPerStep, 1));
#endif
        TEST_ASSERT_EQUAL_UINT32(126, MIDIHelper::readFromBoard(System::Section::button_t::velocity, 0));
    };

    verify();
    flash();
    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));

    //verify all values again
    verify();
}

TEST_CASE(FwUpdate)
{
#ifdef STM32_EMU_EEPROM
    std::string syxPath = fw_build_dir + BOARD_STRING + "/" + fw_build_type_subdir + BOARD_STRING + ".sysex.syx";
#else
    std::string syxPath = fw_build_dir + std::string("mega2560") + "/" + fw_build_type_subdir + std::string("mega2560") + ".sysex.syx";
#endif

    if (!std::filesystem::exists(syxPath))
    {
        printf(".syx file not found, aborting\n");
        exit(1);
    }

    bootloaderMode();

    std::string cmd = std::string("amidi -p ") + MIDIHelper::amidiPort(true) + " -s " + syxPath + " -i " + sysex_fw_update_delay_ms;
    TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd));

    test::wsystem("sleep " + startup_delay_s);

    if (!MIDIHelper::devicePresent())
    {
        printf("OpenDeck device not found after firmware update, aborting\n");
        exit(1);
    }
}

TEST_CASE(BackupAndRestore)
{
    factoryReset();
    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));

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

    std::string cmd = std::string("amidi -p ") + MIDIHelper::amidiPort() + " -S \"" + backup_req + "\" -d -t 5 > " + backup_file_location;
    TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd));

    factoryReset();
    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));

    //verify that the defaults are active again
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

    //now restore backup

    //remove everything before the first line containing F0 00 53 43 01 00 1B F7
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("sed -i '0,/^F0 00 53 43 01 00 1B F7$/d' " + backup_file_location));

    //...and also after the last line containing the same
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("sed -i '/^F0 00 53 43 01 00 1B F7$/Q' " + backup_file_location));

    //send backup
    std::ifstream backupStream(backup_file_location);
    std::string   line;

    while (getline(backupStream, line))
    {
        TEST_ASSERT(MIDIHelper::sendRawSysEx(line) != std::string(""));
    }

    //verify that the custom values are active again
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
    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));

    auto changePreset = [&](bool redirect) {
        //once the preset is changed, the board should forcefully resend all the button states

        if (redirect)
            cmd = std::string("amidi -p ") + MIDIHelper::amidiPort() + " -S \"F0 00 53 43 00 00 01 00 00 02 00 00 00 00 F7\" -d -t 5 > " + temp_midi_data_location;
        else
            cmd = std::string("amidi -p ") + MIDIHelper::amidiPort() + " -S \"F0 00 53 43 00 00 01 00 00 02 00 00 00 00 F7\" -d -t 2";

        TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd, response));
    };

    changePreset(true);

    //drop empty lines
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("sed -i '/^$/d' " + temp_midi_data_location));

    //verify line count
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("grep -c . " + temp_midi_data_location, response));

    //raspberry pi has slow USB MIDI and drops some packets
    //half of expected results is okay
    TEST_ASSERT(stoi(response) >= (MAX_NUMBER_OF_BUTTONS / 2));
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("rm " + temp_midi_data_location));

    //run the same test for DIN MIDI

    reboot();

#if FW_UID == 0x7a382913
    //opendeck2 is the only board which has DIN MIDI connectors

    //rasp pi has weird issues with midi
    //open monitoring interface for a while and let it dump all the existing data first
    cmd = std::string("amidi -p $(amidi -l | grep -E 'ESI MIDIMATE eX MIDI 1'") + std::string(" | grep -Eo 'hw:\\S*')") + " -d -t 2";
    TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd, response));

    auto monitor = [&]() {
        cmd = std::string("amidi -p $(amidi -l | grep -E 'ESI MIDIMATE eX MIDI 1'") + std::string(" | grep -Eo 'hw:\\S*')") + " -d > " + temp_midi_data_location + " &";
        TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd));
    };

    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
    monitor();
    changePreset(false);

    cmd = std::string("killall amidi");
    TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd, response));

    //verify line count - since DIN MIDI isn't enabled, total count should be 0
    test::wsystem("grep -c . " + temp_midi_data_location, response);
    TEST_ASSERT_EQUAL_INT(0, stoi(response));
    TEST_ASSERT_EQUAL_INT(0, test::wsystem("rm " + temp_midi_data_location));

    //now enable DIN MIDI, reboot the board, repeat the test and verify that messages are received on DIN MIDI as well
    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled), 1) == true);
    reboot();
    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
    monitor();
    changePreset(false);

    test::wsystem("sleep 3 && killall amidi > /dev/null");
    test::wsystem("grep -c . " + temp_midi_data_location, response);
    printf("Total number of received DIN MIDI messages: %d\n", stoi(response));
    TEST_ASSERT(stoi(response) >= (MAX_NUMBER_OF_BUTTONS / 2));
// #elif defined(DIN_MIDI_SUPPORTED)
//     //prepare serial port
//     cyclePower(powerCycleType_t::standardWithDeviceCheck);

//     int result = -1;
//     cmd        = std::string("stty -F /dev/" + avr_serial_port + " raw && stty -F /dev/" + avr_serial_port + " -echo -echoe -echok && stty -F /dev/" + avr_serial_port + " 19200 && sleep 3");

//     do
//     {
//         result = test::wsystem(cmd);

//         if (result != 0)
//             cyclePower(powerCycleType_t::standardWithDeviceCheck);
//     } while (result != 0);

//     auto monitor = [&]() {
//         //listen to serial port
//         cmd = std::string("stdbuf -i0 -o0 -e0 hexdump /dev/" + avr_serial_port + " -v -e '3/1 \"%02X \" \"\\n\"' > " + temp_midi_data_location + " &");
//         TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd, response));
//     };

//     monitor();

//     TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));

//     cmd = std::string("killall hexdump");
//     TEST_ASSERT_EQUAL_INT(0, test::wsystem(cmd, response));

//     //verify line count - since DIN MIDI isn't enabled, total count should be 0
//     test::wsystem("grep -c . " + temp_midi_data_location, response);
//     TEST_ASSERT_EQUAL_INT(0, stoi(response));
//     TEST_ASSERT_EQUAL_INT(0, test::wsystem("rm " + temp_midi_data_location));

//     //now enable DIN MIDI, reboot the board, repeat the test and verify that messages are received on DIN MIDI as well
//     TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled), 1) == true);
//     cyclePower(powerCycleType_t::standardWithDeviceCheck);
//     TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
//     monitor();
//     changePreset(false);

//     test::wsystem("sleep 3 && killall hexdump");
//     test::wsystem("grep -c . " + temp_midi_data_location, response);
//     printf("Total number of received DIN MIDI messages: %d\n", stoi(response));
//     TEST_ASSERT(stoi(response) >= (MAX_NUMBER_OF_BUTTONS / 2));
#endif
}

#ifdef DMX_SUPPORTED
TEST_CASE(DMX)
{
    factoryReset();
    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));

    //device manufacturer request
    std::string port = "/dev/" +
#if FW_UID == 0x7a382913
                       opendeck2_dmx_serial_port
#elif FW_UID == 0x09100012
                       mega2560_dmx_serial_port
#else
#error Compiling DMX test for unsupported board
#endif
        ;

    std::vector<uint8_t>
        response = SerialHelper::sendToBoard(port, { 0x7E, 0x4D, 0x00, 0x00, 0xE7 });

    std::vector<uint8_t> expectedResponse = {
        0x7E,
        0x4D,
        0x12,
        0x00,
        ESTA_ID & 0xFF,
        ESTA_ID >> 8 & 0xFF,
        'S',
        'h',
        'a',
        'n',
        't',
        'e',
        'a',
        ' ',
        'C',
        'o',
        'n',
        't',
        'r',
        'o',
        'l',
        's',
        0xE7
    };

    //nothing should be received since DMX isn't enabled
    TEST_ASSERT_EQUAL_UINT32(0, response.size());

    TEST_ASSERT(MIDIHelper::setSingleSysExReq(System::Section::global_t::dmx, static_cast<int>(System::dmxSetting_t::enabled), 1) == true);

    //CDC interface should now work - send request again in form of Enttec Widget API message
    test::wsystem("sleep 1");
    response = SerialHelper::sendToBoard(port, { 0x7E, 0x4D, 0x00, 0x00, 0xE7 });
    TEST_ASSERT(expectedResponse == response);

    //midi part should remain functional as well
    TEST_ASSERT(handshake_ack == MIDIHelper::sendRawSysEx(handshake_req));
}
#endif

#endif