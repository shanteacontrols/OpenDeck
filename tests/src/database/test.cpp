#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "stubs/database/DB_ReadWrite.h"
#include "database/Database.h"
#include "io/leds/LEDs.h"
#include "io/display/Config.h"
#include "OpenDeck/sysconfig/SysConfig.h"

namespace
{
    class DBhandlers : public Database::Handlers
    {
        public:
        DBhandlers() {}

        void presetChange(uint8_t preset) override
        {
            if (presetChangeHandler != nullptr)
                presetChangeHandler(preset);
        }

        void factoryResetStart() override
        {
            if (factoryResetStartHandler != nullptr)
                factoryResetStartHandler();
        }

        void factoryResetDone() override
        {
            if (factoryResetDoneHandler != nullptr)
                factoryResetDoneHandler();
        }

        void initialized() override
        {
            if (initHandler != nullptr)
                initHandler();
        }

        //actions which these handlers should take depend on objects making
        //up the entire system to be initialized
        //therefore in interface we are calling these function pointers which
        // are set in application once we have all objects ready
        void (*presetChangeHandler)(uint8_t preset) = nullptr;
        void (*factoryResetStartHandler)()          = nullptr;
        void (*factoryResetDoneHandler)()           = nullptr;
        void (*initHandler)()                       = nullptr;
    } dbHandlers;

    DBstorageMock dbStorageMock;
    Database      database = Database(dbHandlers, dbStorageMock);
}    // namespace

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
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::global_t::midiFeatures, i));

        //merge section
        //all values should be set to 0
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::global_t::midiMerge, static_cast<size_t>(SysConfig::midiMerge_t::mergeType)));
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::global_t::midiMerge, static_cast<size_t>(SysConfig::midiMerge_t::mergeUSBchannel)));
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::global_t::midiMerge, static_cast<size_t>(SysConfig::midiMerge_t::mergeDINchannel)));

        //button block
        //type section
        //all values should be set to 0 (default type)
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::button_t::type, i));

        //midi message section
        //all values should be set to 0 (default type)
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::button_t::midiMessage, i));

        //midi id section
        //incremental values - first value should be 0, each successive value should be incremented by 1
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT_EQUAL_UINT32(i, database.read(Database::Section::button_t::midiID, i));

        //midi velocity section
        //all values should be set to 127
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT_EQUAL_UINT32(127, database.read(Database::Section::button_t::velocity, i));

        //midi channel section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::button_t::midiChannel, i));

        //encoders block
        //enable section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::encoder_t::enable, i));

        //invert section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::encoder_t::invert, i));

        //mode section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::encoder_t::mode, i));

        //midi id section
        //incremental values - first value should be set to MAX_NUMBER_OF_ANALOG, each successive value should be incremented by 1
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ANALOG + i, database.read(Database::Section::encoder_t::midiID, i));

        //midi channel section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::encoder_t::midiChannel, i));

        //pulses per step section
        //all values should be set to 4
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
            TEST_ASSERT_EQUAL_UINT32(4, database.read(Database::Section::encoder_t::pulsesPerStep, i));

        //analog block
        //enable section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::analog_t::enable, i));

        //invert section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::analog_t::invert, i));

        //type section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::analog_t::invert, i));

        //midi id section
        //incremental values - first value should be set to 0, each successive value should be incremented by 1
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(i, database.read(Database::Section::analog_t::midiID, i));

        //lower limit section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::analog_t::lowerLimit, i));

        //upper limit section
        //all values should be set to 16383
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(16383, database.read(Database::Section::analog_t::upperLimit, i));

        //midi channel section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::analog_t::midiChannel, i));

        //LED block
        //global section
        //all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(IO::LEDs::setting_t::AMOUNT); i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::leds_t::global, i));

        //activation id section
        //incremental values - first value should be set to 0, each successive value should be incremented by 1
        for (int i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT_EQUAL_UINT32(i, database.read(Database::Section::leds_t::activationID, i));

        //rgb enable section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_RGB_LEDS + (MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS / 3); i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::leds_t::rgbEnable, i));

        //control type section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::leds_t::controlType, i));

        //activation value section
        //all values should be set to 127
        for (int i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT_EQUAL_UINT32(127, database.read(Database::Section::leds_t::activationValue, i));

        //midi channel section
        //all values should be set to 0
        for (int i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS; i++)
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::leds_t::midiChannel, i));

#ifdef DISPLAY_SUPPORTED
        //display block
        //feature section
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::display_t::features, static_cast<size_t>(IO::Display::feature_t::enable)));
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::display_t::features, static_cast<size_t>(IO::Display::feature_t::welcomeMsg)));
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::display_t::features, static_cast<size_t>(IO::Display::feature_t::vInfoMsg)));
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::display_t::features, static_cast<size_t>(IO::Display::feature_t::MIDInotesAlternate)));

        //setting section
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::display_t::setting, static_cast<size_t>(IO::Display::setting_t::MIDIeventTime)));
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::display_t::setting, static_cast<size_t>(IO::Display::setting_t::octaveNormalization)));
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
#ifdef BUTTONS_SUPPORTED
    TEST_ASSERT(database.update(Database::Section::button_t::midiID, 0, 114) == true);
#endif

#ifdef ENCODERS_SUPPORTED
    TEST_ASSERT(database.update(Database::Section::encoder_t::midiChannel, 0, 11) == true);
#endif

#ifdef DISPLAY_SUPPORTED
    TEST_ASSERT(database.update(Database::Section::display_t::setting,
                                static_cast<size_t>(IO::Display::setting_t::controller),
                                static_cast<size_t>(IO::U8X8::displayController_t::ssd1306)) == true);
#endif

    database.setPresetPreserveState(true);

    database.factoryReset(LESSDB::factoryResetType_t::full);

    //expect default values
#ifdef BUTTONS_SUPPORTED
    TEST_ASSERT(database.read(Database::Section::button_t::midiID, 0) == 0);
#endif

#ifdef ENCODERS_SUPPORTED
    TEST_ASSERT(database.read(Database::Section::encoder_t::midiChannel, 0) == 0);
#endif

#ifdef DISPLAY_SUPPORTED
    TEST_ASSERT(database.read(Database::Section::display_t::setting,
                              static_cast<size_t>(IO::Display::setting_t::controller)) == static_cast<int32_t>(IO::U8X8::displayController_t::invalid));
#endif

    TEST_ASSERT(database.getPresetPreserveState() == false);
}

#if MAX_NUMBER_OF_LEDS > 0
TEST_CASE(LEDs)
{
    //regression test
    //by default, rgb state should be disabled
    for (int i = 0; i < MAX_NUMBER_OF_RGB_LEDS; i++)
        TEST_ASSERT(database.read(Database::Section::leds_t::rgbEnable, i) == false);

    TEST_ASSERT(database.update(Database::Section::leds_t::controlType, 0, static_cast<int32_t>(IO::LEDs::controlType_t::localPCforStateNoBlink)) == true);

    //rgb state shouldn't change
    for (int i = 0; i < MAX_NUMBER_OF_RGB_LEDS; i++)
        TEST_ASSERT(database.read(Database::Section::leds_t::rgbEnable, i) == false);
}
#endif