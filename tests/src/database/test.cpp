#ifndef USB_LINK_MCU

#include "unity/Framework.h"
#include "stubs/database/DB_ReadWrite.h"
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
    DBstorageMock dbStorageMock;
    Database      database = Database(dbStorageMock, true);
}    // namespace

TEST_CASE(ReadInitialValues)
{
    // init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);

    for (int preset = 0; preset < database.getSupportedPresets(); preset++)
    {
        TEST_ASSERT(database.setPreset(preset) == true);

        // global block
        //----------------------------------
        // MIDI feature section
        // all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(Protocol::MIDI::feature_t::AMOUNT); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::global_t::midiFeatures, i));
        }

#ifdef DMX_SUPPORTED
        // DMX section
        // all values should be set to 0
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::global_t::dmx, Protocol::DMX::setting_t::enable));
#endif

        // button block
        //----------------------------------
        // type section
        // all values should be set to 0 (default type)
        for (int i = 0; i < IO::Buttons::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::button_t::type, i));
        }

        // midi message section
        // all values should be set to 0 (default/note)
        for (int i = 0; i < IO::Buttons::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::button_t::midiMessage, i));
        }

        // midi id section
        for (size_t group = 0; group < IO::Buttons::Collection::groups(); group++)
        {
            for (size_t i = 0; i < IO::Buttons::Collection::size(group); i++)
            {
                TEST_ASSERT_EQUAL_UINT32(i, database.read(Database::Section::button_t::midiID, i + IO::Buttons::Collection::startIndex(group)));
            }
        }

        // midi velocity section
        // all values should be set to 127
        for (int i = 0; i < IO::Buttons::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(127, database.read(Database::Section::button_t::velocity, i));
        }

        // midi channel section
        // all values should be set to 1
        for (int i = 0; i < IO::Buttons::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(1, database.read(Database::Section::button_t::midiChannel, i));
        }

        // encoders block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (int i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::encoder_t::enable, i));
        }

        // invert section
        // all values should be set to 0
        for (int i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::encoder_t::invert, i));
        }

        // mode section
        // all values should be set to 0
        for (int i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::encoder_t::mode, i));
        }

        // midi id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (int i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(i, database.read(Database::Section::encoder_t::midiID, i));
        }

        // midi channel section
        // all values should be set to 1
        for (int i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(1, database.read(Database::Section::encoder_t::midiChannel, i));
        }

        // pulses per step section
        // all values should be set to 4
        for (int i = 0; i < IO::Encoders::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(4, database.read(Database::Section::encoder_t::pulsesPerStep, i));
        }

        // analog block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::analog_t::enable, i));
        }

        // invert section
        // all values should be set to 0
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::analog_t::invert, i));
        }

        // type section
        // all values should be set to 0
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::analog_t::invert, i));
        }

        // midi id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < IO::Analog::Collection::groups(); group++)
        {
            for (size_t i = 0; i < IO::Analog::Collection::size(group); i++)
            {
                TEST_ASSERT_EQUAL_UINT32(i, database.read(Database::Section::analog_t::midiID, i + IO::Analog::Collection::startIndex(group)));
            }
        }

        // lower limit section
        // all values should be set to 0
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::analog_t::lowerLimit, i));
        }

        // upper limit section
        // all values should be set to 16383
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(16383, database.read(Database::Section::analog_t::upperLimit, i));
        }

        // midi channel section
        // all values should be set to 1
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(1, database.read(Database::Section::analog_t::midiChannel, i));
        }

        // lower offset section
        // all values should be set to 0
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::analog_t::lowerOffset, i));
        }

        // upper offset section
        // all values should be set to 0
        for (int i = 0; i < IO::Analog::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::analog_t::upperOffset, i));
        }

        // LED block
        //----------------------------------
        // global section
        // all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(IO::LEDs::setting_t::AMOUNT); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::leds_t::global, i));
        }

        // activation id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < IO::LEDs::Collection::groups(); group++)
        {
            for (size_t i = 0; i < IO::LEDs::Collection::size(group); i++)
            {
                TEST_ASSERT_EQUAL_UINT32(i, database.read(Database::Section::leds_t::activationID, i + IO::LEDs::Collection::startIndex(group)));
            }
        }

        // rgb enable section
        // all values should be set to 0
        for (int i = 0; i < IO::LEDs::Collection::size() / 3 + (IO::Touchscreen::Collection::size() / 3); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::leds_t::rgbEnable, i));
        }

        // control type section
        // all values should be set to midiInNoteMultiVal
        for (int i = 0; i < IO::LEDs::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(IO::LEDs::controlType_t::midiInNoteMultiVal), database.read(Database::Section::leds_t::controlType, i));
        }

        // activation value section
        // all values should be set to 127
        for (int i = 0; i < IO::LEDs::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(127, database.read(Database::Section::leds_t::activationValue, i));
        }

        // midi channel section
        // all values should be set to 1
        for (int i = 0; i < IO::LEDs::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(1, database.read(Database::Section::leds_t::midiChannel, i));
        }

#ifdef I2C_SUPPORTED
        // i2c block
        //----------------------------------
        // display section
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::i2c_t::display, IO::Display::setting_t::enable));
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::i2c_t::display, IO::Display::setting_t::deviceInfoMsg));
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::i2c_t::display, IO::Display::setting_t::controller));
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::i2c_t::display, IO::Display::setting_t::resolution));
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::i2c_t::display, IO::Display::setting_t::MIDIeventTime));
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::i2c_t::display, IO::Display::setting_t::MIDInotesAlternate));
        TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::i2c_t::display, IO::Display::setting_t::octaveNormalization));
#endif

#ifdef TOUCHSCREEN_SUPPORTED
        // touchscreen block
        //----------------------------------
        // setting section
        // all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(IO::Touchscreen::setting_t::AMOUNT); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::setting, i));
        }

        // x position section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::xPos, i));
        }

        // y position section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::yPos, i));
        }

        // width section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::width, i));
        }

        // height section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::height, i));
        }

        // on screen section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::onScreen, i));
        }

        // off screen section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::offScreen, i));
        }

        // page switch enabled section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::pageSwitchEnabled, i));
        }

        // page switch index section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::pageSwitchIndex, i));
        }

        // analog page section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::analogPage, i));
        }

        // analog start x coordinate section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::analogStartXCoordinate, i));
        }

        // analog end x coordinate section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::analogEndXCoordinate, i));
        }

        // analog start y coordinate section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::analogStartYCoordinate, i));
        }

        // analog end y coordinate section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::analogEndYCoordinate, i));
        }

        // analog type section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::analogType, i));
        }

        // analog reset on release section
        // all values should be set to 0
        for (int i = 0; i < IO::Touchscreen::Collection::size(); i++)
        {
            TEST_ASSERT_EQUAL_UINT32(0, database.read(Database::Section::touchscreen_t::analogResetOnRelease, i));
        }
#endif
    }
}

TEST_CASE(Presets)
{
    // init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);

    // verify that first preset is active
    TEST_ASSERT(database.getPreset() == 0);

    size_t activePreset = 0;

    for (activePreset = 0; activePreset < database.getSupportedPresets(); activePreset++)
    {
        TEST_ASSERT(database.setPreset(activePreset) == true);
        TEST_ASSERT(database.getPreset() == activePreset);
    }

    // preset preservation is disabled by default which means that on init, preset should be reset back to 0
    TEST_ASSERT(database.init() == true);
    TEST_ASSERT(database.getPreset() == 0);

    if (database.getSupportedPresets() > 1)
    {
        // try setting value in preset 1, switch back to preset 0 and verify preset 0 still contains default value
        TEST_ASSERT(database.setPreset(1) == true);

        if (IO::Analog::Collection::size())
        {
            TEST_ASSERT(database.update(Database::Section::analog_t::midiID, 0, 127) == true);
        }

        TEST_ASSERT(database.setPreset(0) == true);

        if (IO::Analog::Collection::size())
        {
            TEST_ASSERT(database.read(Database::Section::analog_t::midiID, 0) == 0);
        }
    }

    // enable preset preservation, perform factory reset and verify that preservation is disabled
    database.setPresetPreserveState(true);
    TEST_ASSERT(database.getPresetPreserveState() == true);
    TEST_ASSERT(database.factoryReset() == true);
    TEST_ASSERT(database.init() == true);
    TEST_ASSERT(database.getPresetPreserveState() == false);
}

TEST_CASE(FactoryReset)
{
    // init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);

    // start with clean state
    database.factoryReset();

    // change several values
#ifdef BUTTONS_SUPPORTED
    TEST_ASSERT(database.update(Database::Section::button_t::midiID, 0, 114) == true);
#endif

#ifdef ENCODERS_SUPPORTED
    TEST_ASSERT(database.update(Database::Section::encoder_t::midiChannel, 0, 11) == true);
#endif

#ifdef I2C_SUPPORTED
    TEST_ASSERT(database.update(Database::Section::i2c_t::display,
                                IO::Display::setting_t::controller,
                                IO::Display::displayController_t::ssd1306) == true);
#endif

    database.setPresetPreserveState(true);

    database.factoryReset();

    // expect default values
#ifdef BUTTONS_SUPPORTED
    TEST_ASSERT(database.read(Database::Section::button_t::midiID, 0) == 0);
#endif

#ifdef ENCODERS_SUPPORTED
    TEST_ASSERT(database.read(Database::Section::encoder_t::midiChannel, 0) == 1);
#endif

#ifdef I2C_SUPPORTED
    TEST_ASSERT(database.read(Database::Section::i2c_t::display,
                              IO::Display::setting_t::controller) == static_cast<int32_t>(IO::Display::displayController_t::invalid));
#endif

    TEST_ASSERT(database.getPresetPreserveState() == false);
}

#ifdef LEDS_SUPPORTED
TEST_CASE(LEDs)
{
    // init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);

    // regression test
    // by default, rgb state should be disabled
    for (int i = 0; i < IO::LEDs::Collection::size() / 3; i++)
    {
        TEST_ASSERT(database.read(Database::Section::leds_t::rgbEnable, i) == false);
    }

    TEST_ASSERT(database.update(Database::Section::leds_t::controlType, 0, IO::LEDs::controlType_t::pcSingleVal) == true);

    // rgb state shouldn't change
    for (int i = 0; i < IO::LEDs::Collection::size() / 3; i++)
    {
        TEST_ASSERT(database.read(Database::Section::leds_t::rgbEnable, i) == false);
    }
}
#endif
#endif