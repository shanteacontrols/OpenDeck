#ifndef HW_USB_OVER_SERIAL_HOST

#include "framework/Framework.h"
#include "stubs/Database.h"
#include "database/Database.h"
#include "io/buttons/Buttons.h"
#include "io/buttons/Filter.h"
#include "io/encoders/Encoders.h"
#include "io/encoders/Filter.h"
#include "io/analog/Analog.h"
#include "io/analog/Filter.h"
#include "io/leds/LEDs.h"
#include "io/i2c/peripherals/display/Display.h"
#include "io/touchscreen/Touchscreen.h"
#include "protocol/dmx/DMX.h"
#include "protocol/midi/MIDI.h"

namespace
{
    class DatabaseTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_database._instance.init());
            ASSERT_TRUE(_database._instance.factoryReset());
            ASSERT_EQ(0, _database._instance.getPreset());
        }

        void TearDown() override
        {
            MIDIDispatcher.clear();
        }

        TestDatabase _database;
    };

    uint32_t _dbReadRetVal;
}    // namespace

// more detailed check
#define DB_READ_VERIFY(expected, section, value)                              \
    do                                                                        \
    {                                                                         \
        ASSERT_TRUE(_database._instance.read(section, value, _dbReadRetVal)); \
        ASSERT_EQ(expected, _dbReadRetVal);                                   \
    } while (0)

TEST_F(DatabaseTest, ReadInitialValues)
{
    for (int preset = 0; preset < _database._instance.getSupportedPresets(); preset++)
    {
        ASSERT_TRUE(_database._instance.setPreset(preset));

        // global block
        //----------------------------------
        // MIDI settings section
        // all values should be set to 0 except for the global channel which should be 1
        for (int i = 0; i < static_cast<uint8_t>(Protocol::MIDI::setting_t::AMOUNT); i++)
        {
            if (i == static_cast<int>(Protocol::MIDI::setting_t::GLOBAL_CHANNEL))
            {
                DB_READ_VERIFY(1, Database::Config::Section::global_t::MIDI_SETTINGS, i);
            }
            else
            {
                DB_READ_VERIFY(0, Database::Config::Section::global_t::MIDI_SETTINGS, i);
            }
        }

#ifdef HW_SUPPORT_DMX
        // DMX section
        // all values should be set to 0
        DB_READ_VERIFY(0, Database::Config::Section::global_t::DMX_SETTINGS, Protocol::DMX::setting_t::ENABLE);
#endif

        // button block
        //----------------------------------
        // type section
        // all values should be set to 0 (default type)
        for (size_t i = 0; i < IO::Buttons::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::button_t::TYPE, i);
        }

        // midi message section
        // all values should be set to 0 (default/note)
        for (size_t i = 0; i < IO::Buttons::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::button_t::MESSAGE_TYPE, i);
        }

        // midi id section
        for (size_t group = 0; group < IO::Buttons::Collection::groups(); group++)
        {
            for (size_t i = 0; i < IO::Buttons::Collection::size(group); i++)
            {
                DB_READ_VERIFY(i, Database::Config::Section::button_t::MIDI_ID, i + IO::Buttons::Collection::startIndex(group));
            }
        }

        // midi velocity section
        // all values should be set to 127
        for (size_t i = 0; i < IO::Buttons::Collection::size(); i++)
        {
            DB_READ_VERIFY(127, Database::Config::Section::button_t::VALUE, i);
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < IO::Buttons::Collection::size(); i++)
        {
            DB_READ_VERIFY(1, Database::Config::Section::button_t::CHANNEL, i);
        }

        // encoders block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::encoder_t::ENABLE, i);
        }

        // invert section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::encoder_t::INVERT, i);
        }

        // mode section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::encoder_t::MODE, i);
        }

        // midi id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(i, Database::Config::Section::encoder_t::MIDI_ID, i);
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(1, Database::Config::Section::encoder_t::CHANNEL, i);
        }

        // pulses per step section
        // all values should be set to 4
        for (size_t i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(4, Database::Config::Section::encoder_t::PULSES_PER_STEP, i);
        }

        // analog block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::analog_t::ENABLE, i);
        }

        // invert section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::analog_t::INVERT, i);
        }

        // type section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::analog_t::INVERT, i);
        }

        // midi id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < IO::Analog::Collection::groups(); group++)
        {
            for (size_t i = 0; i < IO::Analog::Collection::size(group); i++)
            {
                DB_READ_VERIFY(i, Database::Config::Section::analog_t::MIDI_ID, i + IO::Analog::Collection::startIndex(group));
            }
        }

        // lower limit section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::analog_t::LOWER_LIMIT, i);
        }

        // upper limit section
        // all values should be set to 16383
        for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(16383, Database::Config::Section::analog_t::UPPER_LIMIT, i);
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(1, Database::Config::Section::analog_t::CHANNEL, i);
        }

        // lower offset section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::analog_t::LOWER_OFFSET, i);
        }

        // upper offset section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::analog_t::UPPER_OFFSET, i);
        }

        // LED block
        //----------------------------------
        // global section
        // all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(IO::LEDs::setting_t::AMOUNT); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::leds_t::GLOBAL, i);
        }

        // activation id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < IO::LEDs::Collection::groups(); group++)
        {
            for (size_t i = 0; i < IO::LEDs::Collection::size(group); i++)
            {
                DB_READ_VERIFY(i, Database::Config::Section::leds_t::ACTIVATION_ID, i + IO::LEDs::Collection::startIndex(group));
            }
        }

        // rgb enable section
        // all values should be set to 0
        for (size_t i = 0; i < IO::LEDs::Collection::size() / 3 + (IO::Touchscreen::Collection::size() / 3); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::leds_t::RGB_ENABLE, i);
        }

        // control type section
        // all values should be set to MIDI_IN_NOTE_MULTI_VAL
        for (size_t i = 0; i < IO::LEDs::Collection::size(); i++)
        {
            DB_READ_VERIFY(static_cast<uint32_t>(IO::LEDs::controlType_t::MIDI_IN_NOTE_MULTI_VAL), Database::Config::Section::leds_t::CONTROL_TYPE, i);
        }

        // activation value section
        // all values should be set to 127
        for (size_t i = 0; i < IO::LEDs::Collection::size(); i++)
        {
            DB_READ_VERIFY(127, Database::Config::Section::leds_t::ACTIVATION_VALUE, i);
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < IO::LEDs::Collection::size(); i++)
        {
            DB_READ_VERIFY(1, Database::Config::Section::leds_t::CHANNEL, i);
        }

#ifdef HW_SUPPORT_I2C
        // i2c block
        //----------------------------------
        // display section
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::DISPLAY, IO::Display::setting_t::ENABLE);
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::DISPLAY, IO::Display::setting_t::DEVICE_INFO_MSG);
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::DISPLAY, IO::Display::setting_t::CONTROLLER);
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::DISPLAY, IO::Display::setting_t::RESOLUTION);
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::DISPLAY, IO::Display::setting_t::EVENT_TIME);
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::DISPLAY, IO::Display::setting_t::MIDI_NOTES_ALTERNATE);
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::DISPLAY, IO::Display::setting_t::OCTAVE_NORMALIZATION);
#endif

#ifdef HW_SUPPORT_TOUCHSCREEN
        // touchscreen block
        //----------------------------------
        // setting section
        // all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(IO::Touchscreen::setting_t::AMOUNT); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::SETTING, i);
        }

        // x position section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::X_POS, i);
        }

        // y position section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::Y_POS, i);
        }

        // width section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::WIDTH, i);
        }

        // height section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::HEIGHT, i);
        }

        // on screen section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::ON_SCREEN, i);
        }

        // off screen section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::OFF_SCREEN, i);
        }

        // page switch enabled section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::PAGE_SWITCH_ENABLED, i);
        }

        // page switch index section
        // all values should be set to 0
        for (size_t i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::PAGE_SWITCH_INDEX, i);
        }
#endif
    }
}

TEST_F(DatabaseTest, Presets)
{
    size_t activePreset = 0;

    for (activePreset = 0; activePreset < _database._instance.getSupportedPresets(); activePreset++)
    {
        ASSERT_TRUE(_database._instance.setPreset(activePreset));
        ASSERT_EQ(activePreset, _database._instance.getPreset());
    }

    // preset preservation is disabled by default which means that on init, preset should be reset back to 0
    ASSERT_TRUE(_database._instance.init());
    ASSERT_EQ(0, _database._instance.getPreset());

    if (_database._instance.getSupportedPresets() > 1)
    {
        // try setting value in preset 1, switch back to preset 0 and verify preset 0 still contains default value
        ASSERT_TRUE(_database._instance.setPreset(1));

        if (IO::Analog::Collection::size())
        {
            ASSERT_TRUE(_database._instance.update(Database::Config::Section::analog_t::MIDI_ID, 0, 127));
        }

        ASSERT_TRUE(_database._instance.setPreset(0));

        if (IO::Analog::Collection::size())
        {
            ASSERT_EQ(0, _database._instance.read(Database::Config::Section::analog_t::MIDI_ID, 0));
        }
    }

    // enable preset preservation, perform factory reset and verify that preservation is disabled
    _database._instance.setPresetPreserveState(true);
    ASSERT_TRUE(_database._instance.getPresetPreserveState());
    ASSERT_TRUE(_database._instance.factoryReset());
    ASSERT_TRUE(_database._instance.init());
    ASSERT_FALSE(_database._instance.getPresetPreserveState());
}

TEST_F(DatabaseTest, FactoryReset)
{
    // change several values
#ifdef BUTTONS_SUPPORTED
    ASSERT_TRUE(_database._instance.update(Database::Config::Section::button_t::MIDI_ID, 0, 114));
#endif

#ifdef ENCODERS_SUPPORTED
    ASSERT_TRUE(_database._instance.update(Database::Config::Section::encoder_t::CHANNEL, 0, 11));
#endif

#ifdef HW_SUPPORT_I2C
    ASSERT_TRUE(_database._instance.update(Database::Config::Section::i2c_t::DISPLAY,
                                           IO::Display::setting_t::CONTROLLER,
                                           IO::Display::displayController_t::SSD1306));
#endif

    _database._instance.setPresetPreserveState(true);
    _database._instance.factoryReset();

    // expect default values
#ifdef BUTTONS_SUPPORTED
    DB_READ_VERIFY(0, Database::Config::Section::button_t::MIDI_ID, 0);
#endif

#ifdef ENCODERS_SUPPORTED
    DB_READ_VERIFY(1, Database::Config::Section::encoder_t::CHANNEL, 0);
#endif

#ifdef HW_SUPPORT_I2C
    DB_READ_VERIFY(static_cast<int32_t>(IO::Display::displayController_t::INVALID),
                   Database::Config::Section::i2c_t::DISPLAY,
                   IO::Display::setting_t::CONTROLLER);
#endif

    ASSERT_FALSE(_database._instance.getPresetPreserveState());
}

#ifdef LEDS_SUPPORTED
TEST_F(DatabaseTest, LEDs)
{
    // regression test
    // by default, rgb state should be disabled
    for (size_t i = 0; i < IO::LEDs::Collection::size() / 3; i++)
    {
        ASSERT_FALSE(_database._instance.read(Database::Config::Section::leds_t::RGB_ENABLE, i));
    }

    ASSERT_TRUE(_database._instance.update(Database::Config::Section::leds_t::CONTROL_TYPE, 0, IO::LEDs::controlType_t::PC_SINGLE_VAL));

    // rgb state shouldn't change
    for (size_t i = 0; i < IO::LEDs::Collection::size() / 3; i++)
    {
        DB_READ_VERIFY(0, Database::Config::Section::leds_t::RGB_ENABLE, i);
    }
}
#endif
#endif