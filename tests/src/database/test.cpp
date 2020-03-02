#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "stubs/database/DB_ReadWrite.h"
#include "database/Database.h"
#include "interface/digital/output/leds/LEDs.h"
#include "interface/display/Config.h"
#include "board/Board.h"
#include "OpenDeck/sysconfig/SysConfig.h"

namespace
{
    Database database = Database(DatabaseStub::read, DatabaseStub::write, EEPROM_SIZE - 3);
}

TEST_SETUP()
{
    //init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);
    TEST_ASSERT(database.isSignatureValid() == true);
}

TEST_CASE(ReadInitialValues)
{
    //verify default values
    database.factoryReset(LESSDB::factoryResetType_t::full);

    for (int i = 0; i < database.getSupportedPresets(); i++)
    {
        TEST_ASSERT(database.setPreset(i) == true);

        //MIDI block
        //feature section
        //all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(SysConfig::midiFeature_t::AMOUNT); i++)
            TEST_ASSERT(database.read(Database::Section::global_t::midiFeatures, i) == 0);

        //merge section
        //all values should be set to 0
        TEST_ASSERT(database.read(Database::Section::global_t::midiMerge, static_cast<size_t>(SysConfig::midiMerge_t::mergeType)) == 0);
        TEST_ASSERT(database.read(Database::Section::global_t::midiMerge, static_cast<size_t>(SysConfig::midiMerge_t::mergeUSBchannel)) == 0);
        TEST_ASSERT(database.read(Database::Section::global_t::midiMerge, static_cast<size_t>(SysConfig::midiMerge_t::mergeDINchannel)) == 0);

        //button block
        //type section
        //all values should be set to 0 (default type)
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT(database.read(Database::Section::button_t::type, i) == 0);

        //midi message section
        //all values should be set to 0 (default type)
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT(database.read(Database::Section::button_t::midiMessage, i) == 0);

        //midi id section
        //incremental values - first value should be 0, each successive value should be incremented by 1
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT(database.read(Database::Section::button_t::midiID, i) == i);

        //midi velocity section
        //all values should be set to 127
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT(database.read(Database::Section::button_t::velocity, i) == 127);

        //midi channel section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT(database.read(Database::Section::button_t::midiChannel, i) == 0);

        //encoders block
        //enable section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT(database.read(Database::Section::encoder_t::enable, i) == 0);

        //invert section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT(database.read(Database::Section::encoder_t::invert, i) == 0);

        //mode section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT(database.read(Database::Section::encoder_t::mode, i) == 0);

        //midi id section
        //incremental values - first value should be set to 0, each successive value should be incremented by 1
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT(database.read(Database::Section::encoder_t::midiID, i) == i);

        //midi channel section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT(database.read(Database::Section::encoder_t::midiChannel, i) == 0);

        //pulses per step section
        //all values should be set to 4
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT(database.read(Database::Section::encoder_t::pulsesPerStep, i) == 4);

        //analog block
        //enable section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT(database.read(Database::Section::analog_t::enable, i) == 0);

        //invert section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT(database.read(Database::Section::analog_t::invert, i) == 0);

        //type section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT(database.read(Database::Section::analog_t::invert, i) == 0);

        //midi id section
        //incremental values - first value should be set to 0, each successive value should be incremented by 1
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT(database.read(Database::Section::analog_t::midiID, i) == i);

        //lower limit section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT(database.read(Database::Section::analog_t::lowerLimit, i) == 0);

        //upper limit section
        //all values should be set to 16383
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT(database.read(Database::Section::analog_t::upperLimit, i) == 16383);

        //midi channel section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT(database.read(Database::Section::analog_t::midiChannel, i) == 0);

#ifdef LEDS_SUPPORTED
        //LED block
        //global section
        //all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(Interface::digital::output::LEDs::setting_t::AMOUNT); i++)
            TEST_ASSERT(database.read(Database::Section::leds_t::global, i) == 0);

        //activation id section
        //incremental values - first value should be set to 0, each successive value should be incremented by 1
        for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
            TEST_ASSERT(database.read(Database::Section::leds_t::activationID, i) == i);

        //rgb enable section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_RGB_LEDS; i++)
            TEST_ASSERT(database.read(Database::Section::leds_t::rgbEnable, i) == 0);

        //control type section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
            TEST_ASSERT(database.read(Database::Section::leds_t::controlType, i) == 0);

        //activation value section
        //all values should be set to 127
        for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
            TEST_ASSERT(database.read(Database::Section::leds_t::activationValue, i) == 127);

        //midi channel section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
            TEST_ASSERT(database.read(Database::Section::leds_t::midiChannel, i) == 0);
#endif

#ifdef DISPLAY_SUPPORTED
        //display block
        //feature section
        TEST_ASSERT(database.read(Database::Section::display_t::features, static_cast<size_t>(Interface::Display::feature_t::enable)) == 0);
        TEST_ASSERT(database.read(Database::Section::display_t::features, static_cast<size_t>(Interface::Display::feature_t::welcomeMsg)) == 0);
        TEST_ASSERT(database.read(Database::Section::display_t::features, static_cast<size_t>(Interface::Display::feature_t::vInfoMsg)) == 0);
        TEST_ASSERT(database.read(Database::Section::display_t::features, static_cast<size_t>(Interface::Display::feature_t::MIDIeventRetention)) == 0);
        TEST_ASSERT(database.read(Database::Section::display_t::features, static_cast<size_t>(Interface::Display::feature_t::MIDInotesAlternate)) == 0);

        int32_t eventTime = database.read(Database::Section::display_t::setting, static_cast<size_t>(Interface::Display::setting_t::MIDIeventTime));

        TEST_ASSERT(eventTime == MIN_MESSAGE_RETENTION_TIME);
//         TEST_ASSERT(database.read(Database::Section::display_t::setting, static_cast<size_t>(Interface::Display::setting_t::octaveNormalization)) == 0);
#endif
    }
}

TEST_CASE(Presets)
{
    database.factoryReset(LESSDB::factoryResetType_t::full);

    //verify that first preset is active
    TEST_ASSERT(database.getPreset() == 0);

    for (int i = 0; i < database.getSupportedPresets(); i++)
    {
        TEST_ASSERT(database.setPreset(i) == true);
        TEST_ASSERT(database.getPreset() == i);
    }

    if (database.getSupportedPresets() > 1)
    {
        //try setting value in preset 1, switch back to preset 0 and verify preset 0 still contains default value
        TEST_ASSERT(database.setPreset(1) == true);
        TEST_ASSERT(database.update(Database::Section::analog_t::midiID, 0, 127) == true);
        TEST_ASSERT(database.setPreset(0) == true);
        TEST_ASSERT(database.read(Database::Section::analog_t::midiID, 0) == 0);
    }

    //enable preset preservation, perform factory reset and verify that preservation is disabled
    database.setPresetPreserveState(true);
    TEST_ASSERT(database.getPresetPreserveState() == true);
    database.factoryReset(LESSDB::factoryResetType_t::full);
    database.init();
    TEST_ASSERT(database.getPresetPreserveState() == false);
}

TEST_CASE(FactoryReset)
{
    database.factoryReset(LESSDB::factoryResetType_t::full);

    //change several values
    TEST_ASSERT(database.update(Database::Section::button_t::midiID, 0, 114) == true);
    TEST_ASSERT(database.update(Database::Section::encoder_t::midiChannel, 0, 11) == true);

#ifdef DISPLAY_SUPPORTED
    TEST_ASSERT(database.update(Database::Section::display_t::setting,
                                static_cast<size_t>(Interface::Display::setting_t::controller),
                                static_cast<size_t>(U8X8::displayController_t::ssd1306)) == true);
#endif

    database.setPresetPreserveState(true);

    database.factoryReset(LESSDB::factoryResetType_t::full);

    //expect default values
    TEST_ASSERT(database.read(Database::Section::button_t::midiID, 0) == 0);
    TEST_ASSERT(database.read(Database::Section::encoder_t::midiChannel, 0) == 0);

#ifdef DISPLAY_SUPPORTED
    TEST_ASSERT(database.read(Database::Section::display_t::setting,
                              static_cast<size_t>(Interface::Display::setting_t::controller)) == static_cast<int32_t>(U8X8::displayController_t::invalid));
#endif

    TEST_ASSERT(database.getPresetPreserveState() == false);
}

#ifdef LEDS_SUPPORTED
TEST_CASE(LEDs)
{
    //regression test
    //by default, rgb state should be disabled
    for (int i = 0; i < MAX_NUMBER_OF_RGB_LEDS; i++)
        TEST_ASSERT(database.read(Database::Section::leds_t::rgbEnable, i) == false);

    TEST_ASSERT(database.update(Database::Section::leds_t::controlType, 0, static_cast<int32_t>(Interface::digital::output::LEDs::controlType_t::localPCforStateNoBlink)) == true);

    //rgb state shouldn't change
    for (int i = 0; i < MAX_NUMBER_OF_RGB_LEDS; i++)
        TEST_ASSERT(database.read(Database::Section::leds_t::rgbEnable, i) == false);
}
#endif