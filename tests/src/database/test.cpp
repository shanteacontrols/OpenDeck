#ifndef USB_LINK_MCU

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

    int32_t _dbReadRetVal;
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
        // MIDI feature section
        // all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(Protocol::MIDI::feature_t::AMOUNT); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::global_t::midiFeatures, i);
        }

#ifdef DMX_SUPPORTED
        // DMX section
        // all values should be set to 0
        DB_READ_VERIFY(0, Database::Config::Section::global_t::dmx, Protocol::DMX::setting_t::enable);
#endif

        // button block
        //----------------------------------
        // type section
        // all values should be set to 0 (default type)
        for (int i = 0; i < IO::Buttons::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::button_t::type, i);
        }

        // midi message section
        // all values should be set to 0 (default/note)
        for (int i = 0; i < IO::Buttons::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::button_t::midiMessage, i);
        }

        // midi id section
        for (size_t group = 0; group < IO::Buttons::Collection::groups(); group++)
        {
            for (size_t i = 0; i < IO::Buttons::Collection::size(group); i++)
            {
                DB_READ_VERIFY(i, Database::Config::Section::button_t::midiID, i + IO::Buttons::Collection::startIndex(group));
            }
        }

        // midi velocity section
        // all values should be set to 127
        for (int i = 0; i < IO::Buttons::Collection::size(); i++)
        {
            DB_READ_VERIFY(127, Database::Config::Section::button_t::velocity, i);
        }

        // midi channel section
        // all values should be set to 1
        for (int i = 0; i < IO::Buttons::Collection::size(); i++)
        {
            DB_READ_VERIFY(1, Database::Config::Section::button_t::midiChannel, i);
        }

        // encoders block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (int i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::encoder_t::enable, i);
        }

        // invert section
        // all values should be set to 0
        for (int i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::encoder_t::invert, i);
        }

        // mode section
        // all values should be set to 0
        for (int i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::encoder_t::mode, i);
        }

        // midi id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (int i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(i, Database::Config::Section::encoder_t::midiID, i);
        }

        // midi channel section
        // all values should be set to 1
        for (int i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(1, Database::Config::Section::encoder_t::midiChannel, i);
        }

        // pulses per step section
        // all values should be set to 4
        for (int i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(4, Database::Config::Section::encoder_t::pulsesPerStep, i);
        }

        // analog block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::analog_t::enable, i);
        }

        // invert section
        // all values should be set to 0
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::analog_t::invert, i);
        }

        // type section
        // all values should be set to 0
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::analog_t::invert, i);
        }

        // midi id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < IO::Analog::Collection::groups(); group++)
        {
            for (size_t i = 0; i < IO::Analog::Collection::size(group); i++)
            {
                DB_READ_VERIFY(i, Database::Config::Section::analog_t::midiID, i + IO::Analog::Collection::startIndex(group));
            }
        }

        // lower limit section
        // all values should be set to 0
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::analog_t::lowerLimit, i);
        }

        // upper limit section
        // all values should be set to 16383
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(16383, Database::Config::Section::analog_t::upperLimit, i);
        }

        // midi channel section
        // all values should be set to 1
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(1, Database::Config::Section::analog_t::midiChannel, i);
        }

        // lower offset section
        // all values should be set to 0
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::analog_t::lowerOffset, i);
        }

        // upper offset section
        // all values should be set to 0
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::analog_t::upperOffset, i);
        }

        // LED block
        //----------------------------------
        // global section
        // all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(IO::LEDs::setting_t::AMOUNT); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::leds_t::global, i);
        }

        // activation id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < IO::LEDs::Collection::groups(); group++)
        {
            for (size_t i = 0; i < IO::LEDs::Collection::size(group); i++)
            {
                DB_READ_VERIFY(i, Database::Config::Section::leds_t::activationID, i + IO::LEDs::Collection::startIndex(group));
            }
        }

        // rgb enable section
        // all values should be set to 0
        for (int i = 0; i < IO::LEDs::Collection::size() / 3 + (IO::Touchscreen::Collection::size() / 3); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::leds_t::rgbEnable, i);
        }

        // control type section
        // all values should be set to midiInNoteMultiVal
        for (int i = 0; i < IO::LEDs::Collection::size(); i++)
        {
            DB_READ_VERIFY(static_cast<uint32_t>(IO::LEDs::controlType_t::midiInNoteMultiVal), Database::Config::Section::leds_t::controlType, i);
        }

        // activation value section
        // all values should be set to 127
        for (int i = 0; i < IO::LEDs::Collection::size(); i++)
        {
            DB_READ_VERIFY(127, Database::Config::Section::leds_t::activationValue, i);
        }

        // midi channel section
        // all values should be set to 1
        for (int i = 0; i < IO::LEDs::Collection::size(); i++)
        {
            DB_READ_VERIFY(1, Database::Config::Section::leds_t::midiChannel, i);
        }

#ifdef I2C_SUPPORTED
        // i2c block
        //----------------------------------
        // display section
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::display, IO::Display::setting_t::enable);
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::display, IO::Display::setting_t::deviceInfoMsg);
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::display, IO::Display::setting_t::controller);
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::display, IO::Display::setting_t::resolution);
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::display, IO::Display::setting_t::MIDIeventTime);
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::display, IO::Display::setting_t::MIDInotesAlternate);
        DB_READ_VERIFY(0, Database::Config::Section::i2c_t::display, IO::Display::setting_t::octaveNormalization);
#endif

#ifdef TOUCHSCREEN_SUPPORTED
        // touchscreen block
        //----------------------------------
        // setting section
        // all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(IO::Touchscreen::setting_t::AMOUNT); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::setting, i);
        }

        // x position section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::xPos, i);
        }

        // y position section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::yPos, i);
        }

        // width section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::width, i);
        }

        // height section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::height, i);
        }

        // on screen section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::onScreen, i);
        }

        // off screen section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::offScreen, i);
        }

        // page switch enabled section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::pageSwitchEnabled, i);
        }

        // page switch index section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::pageSwitchIndex, i);
        }

        // analog page section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::analogPage, i);
        }

        // analog start x coordinate section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::analogStartXCoordinate, i);
        }

        // analog end x coordinate section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::analogEndXCoordinate, i);
        }

        // analog start y coordinate section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::analogStartYCoordinate, i);
        }

        // analog end y coordinate section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::analogEndYCoordinate, i);
        }

        // analog type section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::analogType, i);
        }

        // analog reset on release section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, Database::Config::Section::touchscreen_t::analogResetOnRelease, i);
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
            ASSERT_TRUE(_database._instance.update(Database::Config::Section::analog_t::midiID, 0, 127));
        }

        ASSERT_TRUE(_database._instance.setPreset(0));

        if (IO::Analog::Collection::size())
        {
            ASSERT_EQ(0, _database._instance.read(Database::Config::Section::analog_t::midiID, 0));
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
    ASSERT_TRUE(_database._instance.update(Database::Config::Section::button_t::midiID, 0, 114));
#endif

#ifdef ENCODERS_SUPPORTED
    ASSERT_TRUE(_database._instance.update(Database::Config::Section::encoder_t::midiChannel, 0, 11));
#endif

#ifdef I2C_SUPPORTED
    ASSERT_TRUE(_database._instance.update(Database::Config::Section::i2c_t::display,
                                           IO::Display::setting_t::controller,
                                           IO::Display::displayController_t::ssd1306));
#endif

    _database._instance.setPresetPreserveState(true);
    _database._instance.factoryReset();

    // expect default values
#ifdef BUTTONS_SUPPORTED
    DB_READ_VERIFY(0, Database::Config::Section::button_t::midiID, 0);
#endif

#ifdef ENCODERS_SUPPORTED
    DB_READ_VERIFY(1, Database::Config::Section::encoder_t::midiChannel, 0);
#endif

#ifdef I2C_SUPPORTED
    DB_READ_VERIFY(static_cast<int32_t>(IO::Display::displayController_t::invalid),
                   Database::Config::Section::i2c_t::display,
                   IO::Display::setting_t::controller);
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
        ASSERT_FALSE(_database._instance.read(Database::Config::Section::leds_t::rgbEnable, i));
    }

    ASSERT_TRUE(_database._instance.update(Database::Config::Section::leds_t::controlType, 0, IO::LEDs::controlType_t::pcSingleVal));

    // rgb state shouldn't change
    for (int i = 0; i < IO::LEDs::Collection::size() / 3; i++)
    {
        DB_READ_VERIFY(0, Database::Config::Section::leds_t::rgbEnable, i);
    }
}
#endif
#endif