#include <gtest/gtest.h>
#include "../stubs/database/DB_ReadWrite.h"
#include "../../application/database/Database.h"
#include "../../application/interface/digital/output/leds/DataTypes.h"
#include "../../application/interface/display/Config.h"

class DatabaseTest : public ::testing::Test
{
    protected:
    virtual void SetUp()
    {
        //init checks - no point in running further tests if these conditions fail
        database.init();
        EXPECT_TRUE(database.getDBsize() < LESSDB_SIZE);
        EXPECT_TRUE(database.isSignatureValid());
    }

    virtual void TearDown()
    {
        
    }

    Database database = Database(DatabaseStub::memoryRead, DatabaseStub::memoryWrite);
};

TEST_F(DatabaseTest, ReadInitialValues)
{
    //verify default values
    database.factoryReset(initFull);

    for (int i=0; i<database.getSupportedPresets(); i++)
    {
        EXPECT_EQ(database.setPreset(i), true);

        //MIDI block
        //feature section
        //all values should be set to 0
        for (int i=0; i<MIDI_FEATURES; i++)
            EXPECT_EQ(database.read(DB_BLOCK_GLOBAL, dbSection_global_midiFeatures, i), 0);

        //merge section
        //all values should be set to 0
        EXPECT_EQ(database.read(DB_BLOCK_GLOBAL, dbSection_global_midiMerge, midiMergeType), 0);
        EXPECT_EQ(database.read(DB_BLOCK_GLOBAL, dbSection_global_midiMerge, midiMergeUSBchannel), 0);
        EXPECT_EQ(database.read(DB_BLOCK_GLOBAL, dbSection_global_midiMerge, midiMergeDINchannel), 0);

        //button block
        //type section
        //all values should be set to 0 (default type)
        for (int i=0; i<MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_BUTTONS, dbSection_buttons_type, i), 0);

        //midi message section
        //all values should be set to 0 (default type)
        for (int i=0; i<MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, i), 0);

        //midi id section
        //incremental values - first value should be 0, each successive value should be incremented by 1
        for (int i=0; i<MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiID, i), i);

        //midi velocity section
        //all values should be set to 127
        for (int i=0; i<MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_BUTTONS, dbSection_buttons_velocity, i), 127);

        //midi channel section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_BUTTONS, dbSection_buttons_midiChannel, i), 0);

        //encoders block
        //enable section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ENCODERS, dbSection_encoders_enable, i), 0);

        //invert section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ENCODERS, dbSection_encoders_invert, i), 0);

        //mode section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ENCODERS, dbSection_encoders_mode, i), 0);

        //midi id section
        //incremental values - first value should be set to 0, each successive value should be incremented by 1
        for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ENCODERS, dbSection_encoders_midiID, i), i);

        //midi channel section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ENCODERS, dbSection_encoders_midiChannel, i), 0);

        //pulses per step section
        //all values should be set to 4
        for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ENCODERS, dbSection_encoders_pulsesPerStep, i), 4);

        //analog block
        //enable section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ANALOG, dbSection_analog_enable, i), 0);

        //invert section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ANALOG, dbSection_analog_invert, i), 0);

        //type section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ANALOG, dbSection_analog_invert, i), 0);

        //midi id section
        //incremental values - first value should be set to 0, each successive value should be incremented by 1
        for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ANALOG, dbSection_analog_midiID, i), i);

        //lower limit section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ANALOG, dbSection_analog_lowerLimit, i), 0);

        //upper limit section
        //all values should be set to 16383
        for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i), 16383);

        //midi channel section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
            EXPECT_EQ(database.read(DB_BLOCK_ANALOG, dbSection_analog_midiChannel, i), 0);

        //LED block
        //global section
        //all values should be set to 0
        for (int i=0; i<LED_GLOBAL_PARAMETERS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_LEDS, dbSection_leds_global, i), 0);

        //activation id section
        //incremental values - first value should be set to 0, each successive value should be incremented by 1
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_LEDS, dbSection_leds_activationID, i), i);

        //rgb enable section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_LEDS, dbSection_leds_rgbEnable, i), 0);

        //control type section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_LEDS, dbSection_leds_controlType, i), 0);

        //activation value section
        //all values should be set to 127
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_LEDS, dbSection_leds_activationValue, i), 127);

        //midi channel section
        //all values should be set to 0
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            EXPECT_EQ(database.read(DB_BLOCK_LEDS, dbSection_leds_midiChannel, i), 0);

        //display block
        //feature section
        //this section uses custom values
        EXPECT_EQ(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureEnable), 0);
        EXPECT_EQ(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureWelcomeMsg), 0);
        EXPECT_EQ(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureVInfoMsg), 0);
        EXPECT_EQ(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureMIDIeventRetention), 0);
        EXPECT_EQ(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureMIDInotesAlternate), 0);
        EXPECT_EQ(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureMIDIeventTime), MIN_MESSAGE_RETENTION_TIME);
        EXPECT_EQ(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureOctaveNormalization), 0);

        //hw section
        //this section uses custom values
        //all values should be set to 0
        EXPECT_EQ(database.read(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwController), DISPLAY_CONTROLLERS);
        EXPECT_EQ(database.read(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwResolution), DISPLAY_RESOLUTIONS);
    }
}

TEST_F(DatabaseTest, Presets)
{
    database.init();
    database.factoryReset(initFull);

    //verify that first preset is active
    EXPECT_EQ(database.getPreset(), 0);

    for (int i=0; i<database.getSupportedPresets(); i++)
    {
        EXPECT_EQ(database.setPreset(i), true);
        EXPECT_EQ(database.getPreset(), i);
    }

    if (database.getSupportedPresets() > 1)
    {
        //try setting value in preset 1, switch back to preset 0 and verify preset 0 still contains default value
        EXPECT_EQ(database.setPreset(1), true);
        EXPECT_TRUE(database.update(DB_BLOCK_ANALOG, dbSection_analog_midiID, 5, 1));
        EXPECT_EQ(database.setPreset(0), true);
        EXPECT_EQ(database.read(DB_BLOCK_ANALOG, dbSection_analog_midiID, 5), 5);
    }
}

TEST_F(DatabaseTest, FactoryReset)
{
    database.init();
    database.factoryReset(initFull);

    //change random values
    EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiID, 5, 1));
    EXPECT_TRUE(database.update(DB_BLOCK_ENCODERS, dbSection_analog_midiChannel, 2, 11));
    EXPECT_TRUE(database.update(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwController, displayController_ssd1306));

    database.factoryReset(initFull);

    //expect default values
    EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiID, 5, 5));
    EXPECT_TRUE(database.update(DB_BLOCK_ENCODERS, dbSection_analog_midiChannel, 2, 0));
    EXPECT_TRUE(database.update(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwController, DISPLAY_CONTROLLERS));

    database.setPresetPreserveState(true);
    EXPECT_TRUE(database.getPresetPreserveState());
}

TEST_F(DatabaseTest, LEDs)
{
    database.init();
    database.factoryReset(initFull);

    //regression test
    //by default, rgb state should be disabled
    for (int i=0; i<MAX_NUMBER_OF_RGB_LEDS; i++)
        EXPECT_FALSE(database.read(DB_BLOCK_LEDS, dbSection_leds_rgbEnable, i));

    EXPECT_TRUE(database.update(DB_BLOCK_LEDS, dbSection_leds_controlType, 0, ledControlLocal_PCStateOnly));

    //rgb state shouldn't change
    for (int i=0; i<MAX_NUMBER_OF_RGB_LEDS; i++)
        EXPECT_FALSE(database.read(DB_BLOCK_LEDS, dbSection_leds_rgbEnable, i));
}