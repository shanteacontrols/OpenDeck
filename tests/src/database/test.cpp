/*

Copyright Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef PROJECT_TARGET_USB_OVER_SERIAL_HOST

#include "tests/common.h"
#include "application/database/builder_test.h"
#include "application/io/buttons/buttons.h"
#include "application/io/encoders/encoders.h"
#include "application/io/analog/analog.h"
#include "application/io/leds/leds.h"
#include "application/io/i2c/peripherals/display/display.h"
#include "application/io/touchscreen/touchscreen.h"
#include "application/protocol/midi/midi.h"
#include "application/util/configurable/configurable.h"

namespace
{
    class DatabaseTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_database.instance().init());
            ASSERT_TRUE(_database.instance().factoryReset());
            ASSERT_EQ(0, _database.instance().getPreset());
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            MidiDispatcher.clear();
        }

        database::BuilderTest _database;
    };

    uint32_t _dbReadRetVal;
}    // namespace

// more detailed check
#define DB_READ_VERIFY(expected, section, value)                               \
    do                                                                         \
    {                                                                          \
        ASSERT_TRUE(_database.instance().read(section, value, _dbReadRetVal)); \
        ASSERT_EQ(expected, _dbReadRetVal);                                    \
    } while (0)

TEST_F(DatabaseTest, ReadInitialValues)
{
    for (int preset = 0; preset < _database.instance().getSupportedPresets(); preset++)
    {
        ASSERT_TRUE(_database.instance().setPreset(preset));

        // system block
        //----------------------------------
        // system settings section
        for (int i = 0; i < static_cast<uint8_t>(database::Config::systemSetting_t::CUSTOM_SYSTEM_SETTING_END); i++)
        {
            if (i == static_cast<int>(database::Config::systemSetting_t::ACTIVE_PRESET))
            {
                DB_READ_VERIFY(preset, database::Config::Section::system_t::SYSTEM_SETTINGS, i);
            }
            else
            {
                DB_READ_VERIFY(0, database::Config::Section::system_t::SYSTEM_SETTINGS, i);
            }
        }

        // global block
        //----------------------------------
        // MIDI settings section
        // all values should be set to 0 except for the global channel which should be 1
        for (int i = 0; i < static_cast<uint8_t>(protocol::midi::setting_t::AMOUNT); i++)
        {
            if (i == static_cast<int>(protocol::midi::setting_t::GLOBAL_CHANNEL))
            {
                DB_READ_VERIFY(1, database::Config::Section::global_t::MIDI_SETTINGS, i);
            }
            else
            {
                DB_READ_VERIFY(0, database::Config::Section::global_t::MIDI_SETTINGS, i);
            }
        }

        // button block
        //----------------------------------
        // type section
        // all values should be set to 0 (default type)
        for (size_t i = 0; i < io::buttons::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::button_t::TYPE, i);
        }

        // midi message section
        // all values should be set to 0 (default/note)
        for (size_t i = 0; i < io::buttons::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::button_t::MESSAGE_TYPE, i);
        }

        // midi id section
        for (size_t group = 0; group < io::buttons::Collection::GROUPS(); group++)
        {
            for (size_t i = 0; i < io::buttons::Collection::SIZE(group); i++)
            {
                DB_READ_VERIFY(i, database::Config::Section::button_t::MIDI_ID, i + io::buttons::Collection::START_INDEX(group));
            }
        }

        // midi velocity section
        // all values should be set to 127
        for (size_t i = 0; i < io::buttons::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(127, database::Config::Section::button_t::VALUE, i);
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < io::buttons::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(1, database::Config::Section::button_t::CHANNEL, i);
        }

        // encoders block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (size_t i = 0; i < io::encoders::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::encoder_t::ENABLE, i);
        }

        // invert section
        // all values should be set to 0
        for (size_t i = 0; i < io::encoders::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::encoder_t::INVERT, i);
        }

        // mode section
        // all values should be set to 0
        for (size_t i = 0; i < io::encoders::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::encoder_t::MODE, i);
        }

        // midi id 1 section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t i = 0; i < io::encoders::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(i, database::Config::Section::encoder_t::MIDI_ID_1, i);
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < io::encoders::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(1, database::Config::Section::encoder_t::CHANNEL, i);
        }

        // pulses per step section
        // all values should be set to 4
        for (size_t i = 0; i < io::encoders::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(4, database::Config::Section::encoder_t::PULSES_PER_STEP, i);
        }

        // lower limit section
        // all values should be set to 0
        for (size_t i = 0; i < io::encoders::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::encoder_t::LOWER_LIMIT, i);
        }

        // upper limit section
        // all values should be set to 16383
        for (size_t i = 0; i < io::encoders::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(16383, database::Config::Section::encoder_t::UPPER_LIMIT, i);
        }

        // repeated value section
        // all values should be set to 127
        for (size_t i = 0; i < io::encoders::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(127, database::Config::Section::encoder_t::REPEATED_VALUE, i);
        }

        // midi id 2 section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t i = 0; i < io::encoders::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(i, database::Config::Section::encoder_t::MIDI_ID_2, i);
        }

        // analog block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (size_t i = 0; i < io::analog::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::analog_t::ENABLE, i);
        }

        // invert section
        // all values should be set to 0
        for (size_t i = 0; i < io::analog::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::analog_t::INVERT, i);
        }

        // type section
        // all values should be set to 0
        for (size_t i = 0; i < io::analog::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::analog_t::INVERT, i);
        }

        // midi id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < io::analog::Collection::GROUPS(); group++)
        {
            for (size_t i = 0; i < io::analog::Collection::SIZE(group); i++)
            {
                DB_READ_VERIFY(i, database::Config::Section::analog_t::MIDI_ID, i + io::analog::Collection::START_INDEX(group));
            }
        }

        // lower limit section
        // all values should be set to 0
        for (size_t i = 0; i < io::analog::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::analog_t::LOWER_LIMIT, i);
        }

        // upper limit section
        // all values should be set to 16383
        for (size_t i = 0; i < io::analog::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(16383, database::Config::Section::analog_t::UPPER_LIMIT, i);
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < io::analog::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(1, database::Config::Section::analog_t::CHANNEL, i);
        }

        // lower offset section
        // all values should be set to 0
        for (size_t i = 0; i < io::analog::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::analog_t::LOWER_OFFSET, i);
        }

        // upper offset section
        // all values should be set to 0
        for (size_t i = 0; i < io::analog::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::analog_t::UPPER_OFFSET, i);
        }

        // LED block
        //----------------------------------
        // global section
        // all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(io::leds::setting_t::AMOUNT); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::leds_t::GLOBAL, i);
        }

        // activation id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < io::leds::Collection::GROUPS(); group++)
        {
            for (size_t i = 0; i < io::leds::Collection::SIZE(group); i++)
            {
                DB_READ_VERIFY(i, database::Config::Section::leds_t::ACTIVATION_ID, i + io::leds::Collection::START_INDEX(group));
            }
        }

        // rgb enable section
        // all values should be set to 0
        for (size_t i = 0; i < io::leds::Collection::SIZE() / 3 + (io::touchscreen::Collection::SIZE() / 3); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::leds_t::RGB_ENABLE, i);
        }

        // control type section
        // all values should be set to MIDI_IN_NOTE_MULTI_VAL
        for (size_t i = 0; i < io::leds::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(static_cast<uint32_t>(io::leds::controlType_t::MIDI_IN_NOTE_MULTI_VAL), database::Config::Section::leds_t::CONTROL_TYPE, i);
        }

        // activation value section
        // all values should be set to 127
        for (size_t i = 0; i < io::leds::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(127, database::Config::Section::leds_t::ACTIVATION_VALUE, i);
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < io::leds::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(1, database::Config::Section::leds_t::CHANNEL, i);
        }

#ifdef PROJECT_TARGET_SUPPORT_I2C
        // i2c block
        //----------------------------------
        // display section
        DB_READ_VERIFY(0, database::Config::Section::i2c_t::DISPLAY, io::i2c::display::setting_t::ENABLE);
        DB_READ_VERIFY(0, database::Config::Section::i2c_t::DISPLAY, io::i2c::display::setting_t::DEVICE_INFO_MSG);
        DB_READ_VERIFY(0, database::Config::Section::i2c_t::DISPLAY, io::i2c::display::setting_t::CONTROLLER);
        DB_READ_VERIFY(0, database::Config::Section::i2c_t::DISPLAY, io::i2c::display::setting_t::RESOLUTION);
        DB_READ_VERIFY(0, database::Config::Section::i2c_t::DISPLAY, io::i2c::display::setting_t::EVENT_TIME);
        DB_READ_VERIFY(0, database::Config::Section::i2c_t::DISPLAY, io::i2c::display::setting_t::MIDI_NOTES_ALTERNATE);
        DB_READ_VERIFY(0, database::Config::Section::i2c_t::DISPLAY, io::i2c::display::setting_t::OCTAVE_NORMALIZATION);
#endif

#ifdef PROJECT_TARGET_SUPPORT_TOUCHSCREEN
        // touchscreen block
        //----------------------------------
        // setting section
        // all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(io::touchscreen::setting_t::AMOUNT); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::touchscreen_t::SETTING, i);
        }

        // x position section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::touchscreen_t::X_POS, i);
        }

        // y position section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::touchscreen_t::Y_POS, i);
        }

        // width section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::touchscreen_t::WIDTH, i);
        }

        // height section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::touchscreen_t::HEIGHT, i);
        }

        // on screen section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::touchscreen_t::ON_SCREEN, i);
        }

        // off screen section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::touchscreen_t::OFF_SCREEN, i);
        }

        // page switch enabled section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::touchscreen_t::PAGE_SWITCH_ENABLED, i);
        }

        // page switch index section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::SIZE(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::touchscreen_t::PAGE_SWITCH_INDEX, i);
        }
#endif
    }
}

TEST_F(DatabaseTest, Presets)
{
    size_t activePreset = 0;

    for (activePreset = 0; activePreset < _database.instance().getSupportedPresets(); activePreset++)
    {
        ASSERT_TRUE(_database.instance().setPreset(activePreset));
        ASSERT_EQ(activePreset, _database.instance().getPreset());
    }

    // preset preservation is disabled by default which means that on init, preset should be reset back to 0
    ASSERT_TRUE(_database.instance().init());
    ASSERT_EQ(0, _database.instance().getPreset());

    if (_database.instance().getSupportedPresets() > 1)
    {
        // try setting value in preset 1, switch back to preset 0 and verify preset 0 still contains default value
        ASSERT_TRUE(_database.instance().setPreset(1));

        if (io::analog::Collection::SIZE())
        {
            ASSERT_TRUE(_database.instance().update(database::Config::Section::analog_t::MIDI_ID, 0, 127));
        }

        ASSERT_TRUE(_database.instance().setPreset(0));

        if (io::analog::Collection::SIZE())
        {
            ASSERT_EQ(0, _database.instance().read(database::Config::Section::analog_t::MIDI_ID, 0));
        }
    }

    // enable preset preservation, perform factory reset and verify that preservation is disabled
    _database.instance().setPresetPreserveState(true);
    ASSERT_TRUE(_database.instance().getPresetPreserveState());
    ASSERT_TRUE(_database.instance().factoryReset());
    ASSERT_TRUE(_database.instance().init());
    ASSERT_FALSE(_database.instance().getPresetPreserveState());
}

TEST_F(DatabaseTest, FactoryReset)
{
    // change several values
#ifdef BUTTONS_SUPPORTED
    ASSERT_TRUE(_database.instance().update(database::Config::Section::button_t::MIDI_ID, 0, 114));
#endif

#ifdef ENCODERS_SUPPORTED
    ASSERT_TRUE(_database.instance().update(database::Config::Section::encoder_t::CHANNEL, 0, 11));
#endif

#ifdef PROJECT_TARGET_SUPPORT_I2C
    ASSERT_TRUE(_database.instance().update(database::Config::Section::i2c_t::DISPLAY,
                                            io::i2c::display::setting_t::CONTROLLER,
                                            io::i2c::display::displayController_t::SSD1306));
#endif

    _database.instance().setPresetPreserveState(true);
    _database.instance().factoryReset();

    // expect default values
#ifdef BUTTONS_SUPPORTED
    DB_READ_VERIFY(0, database::Config::Section::button_t::MIDI_ID, 0);
#endif

#ifdef ENCODERS_SUPPORTED
    DB_READ_VERIFY(1, database::Config::Section::encoder_t::CHANNEL, 0);
#endif

#ifdef PROJECT_TARGET_SUPPORT_I2C
    DB_READ_VERIFY(static_cast<int32_t>(io::i2c::display::displayController_t::INVALID),
                   database::Config::Section::i2c_t::DISPLAY,
                   io::i2c::display::setting_t::CONTROLLER);
#endif

    ASSERT_FALSE(_database.instance().getPresetPreserveState());
}

#ifdef LEDS_SUPPORTED
TEST_F(DatabaseTest, LEDs)
{
    // regression test
    // by default, rgb state should be disabled
    for (size_t i = 0; i < io::leds::Collection::SIZE() / 3; i++)
    {
        ASSERT_FALSE(_database.instance().read(database::Config::Section::leds_t::RGB_ENABLE, i));
    }

    ASSERT_TRUE(_database.instance().update(database::Config::Section::leds_t::CONTROL_TYPE, 0, io::leds::controlType_t::PC_SINGLE_VAL));

    // rgb state shouldn't change
    for (size_t i = 0; i < io::leds::Collection::SIZE() / 3; i++)
    {
        DB_READ_VERIFY(0, database::Config::Section::leds_t::RGB_ENABLE, i);
    }
}
#endif
#endif