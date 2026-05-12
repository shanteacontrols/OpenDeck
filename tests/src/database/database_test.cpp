/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/database.h"
#include "database/builder.h"
#include "database/layout.h"
#include "io/digital/switches/common.h"
#include "io/digital/encoders/common.h"
#include "io/analog/common.h"
#include "io/outputs/common.h"
#include "io/i2c/peripherals/display/common.h"
#include "io/touchscreen/common.h"
#include "protocol/midi/midi.h"
#include "protocol/mdns/common.h"
#include "protocol/osc/common.h"
#include "util/configurable/configurable.h"

namespace
{
    class CountingHandlers : public database::Handlers
    {
        public:
        void preset_change(uint8_t) override
        {}

        void factory_reset_start() override
        {
            factory_reset_start_count++;
        }

        void factory_reset_done() override
        {
            factory_reset_done_count++;
        }

        void initialized() override
        {
            initialized_count++;
        }

        size_t factory_reset_start_count = 0;
        size_t factory_reset_done_count  = 0;
        size_t initialized_count         = 0;
    };

    class DatabaseTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_database.instance().init(_handlers));
            ASSERT_TRUE(_database.instance().factory_reset());
            ASSERT_EQ(0, _database.instance().current_preset());
        }

        void TearDown() override
        {
            ConfigHandler.clear();
        }

        tests::NoOpDatabaseHandlers _handlers;
        database::Builder           _database;
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

#define DB_COMMON_READ_VERIFY(expected, value)                                                   \
    do                                                                                           \
    {                                                                                            \
        ASSERT_TRUE(_database.instance().read(database::Config::Section::Common::CommonSettings, \
                                              value,                                             \
                                              _dbReadRetVal));                                   \
        ASSERT_EQ(expected, _dbReadRetVal);                                                      \
    } while (0)

TEST_F(DatabaseTest, ReadInitialValues)
{
    const auto expected_signature = static_cast<uint16_t>(database::AppLayout::common_uid() ^
                                                          database::AppLayout::preset_uid() ^
                                                          static_cast<uint16_t>(OPENDECK_TARGET_UID) ^
                                                          static_cast<uint16_t>(_database.instance().supported_presets()));

    for (int preset = 0; preset < _database.instance().supported_presets(); preset++)
    {
        ASSERT_TRUE(_database.instance().set_preset(preset));

        // common block
        //----------------------------------
        DB_COMMON_READ_VERIFY(preset, database::Config::CommonSetting::ActivePreset);
        DB_COMMON_READ_VERIFY(0, database::Config::CommonSetting::PresetPreserve);

        for (int i = static_cast<int>(database::Config::CommonSetting::CustomCommonSettingStart);
             i < static_cast<int>(database::Config::CommonSetting::CustomCommonSettingEnd);
             i++)
        {
            DB_COMMON_READ_VERIFY(0, i);
        }

        DB_COMMON_READ_VERIFY(expected_signature, database::Config::CommonSetting::Uid);

        // global block
        //----------------------------------
        // MIDI settings section
        // all values should be set to 0 except for the global channel which should be 1
        for (int i = 0; i < static_cast<uint8_t>(protocol::midi::Setting::Count); i++)
        {
            if (i == static_cast<int>(protocol::midi::Setting::GlobalChannel))
            {
                DB_READ_VERIFY(1, database::Config::Section::Global::MidiSettings, i);
            }
            else
            {
                DB_READ_VERIFY(0, database::Config::Section::Global::MidiSettings, i);
            }
        }

        // OSC settings section
        // all values should be set to 0 when OSC is not enabled for the test target
        for (int i = 0; i < static_cast<uint8_t>(protocol::osc::Setting::Count); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Global::OscSettings, i);
        }

        // switch block
        //----------------------------------
        // type section
        // all values should be set to 0 (default type)
        for (size_t i = 0; i < io::switches::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Switch::Type, i);
        }

        // midi message section
        // all values should be set to 0 (default/note)
        for (size_t i = 0; i < io::switches::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Switch::MessageType, i);
        }

        // midi id section
        for (size_t group = 0; group < io::switches::Collection::groups(); group++)
        {
            for (size_t i = 0; i < io::switches::Collection::size(group); i++)
            {
                DB_READ_VERIFY(i, database::Config::Section::Switch::MidiId, i + io::switches::Collection::start_index(group));
            }
        }

        // midi velocity section
        // all values should be set to 127
        for (size_t i = 0; i < io::switches::Collection::size(); i++)
        {
            DB_READ_VERIFY(127, database::Config::Section::Switch::Value, i);
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < io::switches::Collection::size(); i++)
        {
            DB_READ_VERIFY(1, database::Config::Section::Switch::Channel, i);
        }

        // encoders block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (size_t i = 0; i < io::encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Encoder::Enable, i);
        }

        // invert section
        // all values should be set to 0
        for (size_t i = 0; i < io::encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Encoder::Invert, i);
        }

        // mode section
        // all values should be set to 0
        for (size_t i = 0; i < io::encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Encoder::Mode, i);
        }

        // midi id 1 section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t i = 0; i < io::encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(i, database::Config::Section::Encoder::MidiId1, i);
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < io::encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(1, database::Config::Section::Encoder::Channel, i);
        }

        // lower limit section
        // all values should be set to 0
        for (size_t i = 0; i < io::encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Encoder::LowerLimit, i);
        }

        // upper limit section
        // all values should be set to 16383
        for (size_t i = 0; i < io::encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(16383, database::Config::Section::Encoder::UpperLimit, i);
        }

        // repeated value section
        // all values should be set to 127
        for (size_t i = 0; i < io::encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(127, database::Config::Section::Encoder::RepeatedValue, i);
        }

        // midi id 2 section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t i = 0; i < io::encoders::Collection::size(); i++)
        {
            DB_READ_VERIFY(i, database::Config::Section::Encoder::MidiId2, i);
        }

        // analog block
        //----------------------------------
        // enable section
        // all values should be set to 0
        for (size_t i = 0; i < io::analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Analog::Enable, i);
        }

        // invert section
        // all values should be set to 0
        for (size_t i = 0; i < io::analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Analog::Invert, i);
        }

        // type section
        // all values should be set to 0
        for (size_t i = 0; i < io::analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Analog::Invert, i);
        }

        // midi id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < io::analog::Collection::groups(); group++)
        {
            for (size_t i = 0; i < io::analog::Collection::size(group); i++)
            {
                DB_READ_VERIFY(i, database::Config::Section::Analog::MidiId, i + io::analog::Collection::start_index(group));
            }
        }

        // lower limit section
        // all values should be set to 0
        for (size_t i = 0; i < io::analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Analog::LowerLimit, i);
        }

        // upper limit section
        // all values should be set to 16383
        for (size_t i = 0; i < io::analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(16383, database::Config::Section::Analog::UpperLimit, i);
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < io::analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(1, database::Config::Section::Analog::Channel, i);
        }

        // lower offset section
        // all values should be set to 0
        for (size_t i = 0; i < io::analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Analog::LowerOffset, i);
        }

        // upper offset section
        // all values should be set to 0
        for (size_t i = 0; i < io::analog::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Analog::UpperOffset, i);
        }

        // OUTPUT block
        //----------------------------------
        // global section
        // all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(io::outputs::Setting::Count); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Outputs::Global, i);
        }

        // activation id section
        // incremental values - first value should be 0, each successive value should be incremented by 1 for each group
        for (size_t group = 0; group < io::outputs::Collection::groups(); group++)
        {
            for (size_t i = 0; i < io::outputs::Collection::size(group); i++)
            {
                DB_READ_VERIFY(i, database::Config::Section::Outputs::ActivationId, i + io::outputs::Collection::start_index(group));
            }
        }

        // rgb enable section
        // all values should be set to 0
        for (size_t i = 0; i < io::outputs::Collection::size() / 3 + (io::touchscreen::Collection::size() / 3); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Outputs::RgbEnable, i);
        }

        // control type section
        // all values should be set to MIDI_IN_NOTE_MULTI_VAL
        for (size_t i = 0; i < io::outputs::Collection::size(); i++)
        {
            DB_READ_VERIFY(static_cast<uint32_t>(io::outputs::ControlType::MidiInNoteMultiVal), database::Config::Section::Outputs::ControlType, i);
        }

        // activation value section
        // all values should be set to 127
        for (size_t i = 0; i < io::outputs::Collection::size(); i++)
        {
            DB_READ_VERIFY(127, database::Config::Section::Outputs::ActivationValue, i);
        }

        // midi channel section
        // all values should be set to 1
        for (size_t i = 0; i < io::outputs::Collection::size(); i++)
        {
            DB_READ_VERIFY(1, database::Config::Section::Outputs::Channel, i);
        }

#ifdef PROJECT_TARGET_SUPPORT_I2C
        // i2c block
        //----------------------------------
        // display section
        DB_READ_VERIFY(0, database::Config::Section::I2c::Display, io::i2c::display::Setting::Enable);
        DB_READ_VERIFY(0, database::Config::Section::I2c::Display, io::i2c::display::Setting::DeviceInfoMsg);
        DB_READ_VERIFY(0, database::Config::Section::I2c::Display, io::i2c::display::Setting::Controller);
        DB_READ_VERIFY(0, database::Config::Section::I2c::Display, io::i2c::display::Setting::Resolution);
        DB_READ_VERIFY(0, database::Config::Section::I2c::Display, io::i2c::display::Setting::EventTime);
        DB_READ_VERIFY(0, database::Config::Section::I2c::Display, io::i2c::display::Setting::MidiNotesAlternate);
        DB_READ_VERIFY(0, database::Config::Section::I2c::Display, io::i2c::display::Setting::OctaveNormalization);
#endif

#ifdef PROJECT_TARGET_SUPPORT_TOUCHSCREEN
        // touchscreen block
        //----------------------------------
        // setting section
        // all values should be set to 0
        for (int i = 0; i < static_cast<uint8_t>(io::touchscreen::Setting::Count); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Touchscreen::Setting, i);
        }

        // x position section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Touchscreen::XPos, i);
        }

        // y position section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Touchscreen::YPos, i);
        }

        // width section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Touchscreen::Width, i);
        }

        // height section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Touchscreen::Height, i);
        }

        // on screen section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Touchscreen::OnScreen, i);
        }

        // off screen section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Touchscreen::OffScreen, i);
        }

        // page switch enabled section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Touchscreen::PageSwitchEnabled, i);
        }

        // page switch index section
        // all values should be set to 0
        for (size_t i = 0; i < io::touchscreen::Collection::size(); i++)
        {
            DB_READ_VERIFY(0, database::Config::Section::Touchscreen::PageSwitchIndex, i);
        }
#endif
    }
}

TEST(DatabaseRegressionTest, InitWithMissingFactorySnapshotFallsBackToFactoryReset)
{
    database::Builder builder;
    CountingHandlers  handlers;
    uint32_t          read_value = 0;

    ASSERT_TRUE(builder.instance().init(handlers));

    EXPECT_EQ(1U, handlers.factory_reset_start_count);
    EXPECT_EQ(1U, handlers.factory_reset_done_count);
    EXPECT_EQ(1U, handlers.initialized_count);

    EXPECT_TRUE(builder.instance().is_initialized());
    EXPECT_EQ(0, builder.instance().current_preset());

    ASSERT_TRUE(builder.instance().read(database::Config::Section::Common::CommonSettings,
                                        database::Config::CommonSetting::Uid,
                                        read_value));
    EXPECT_NE(0U, read_value);
}

TEST_F(DatabaseTest, CommonSectionsDoNotAlias)
{
    uint32_t common_value   = 0;
    uint32_t hostname_value = 0;

    ASSERT_TRUE(_database.instance().update(database::Config::Section::Common::CommonSettings,
                                            database::Config::CommonSetting::CustomCommonSettingStart,
                                            42));
    ASSERT_TRUE(_database.instance().update(database::Config::Section::Common::MdnsHostname, 0, 'o'));

    ASSERT_TRUE(_database.instance().read(database::Config::Section::Common::CommonSettings,
                                          database::Config::CommonSetting::CustomCommonSettingStart,
                                          common_value));
    ASSERT_TRUE(_database.instance().read(database::Config::Section::Common::MdnsHostname, 0, hostname_value));

    EXPECT_EQ(42U, common_value);
    EXPECT_EQ(static_cast<uint32_t>('o'), hostname_value);

    for (size_t i = 1; i < protocol::mdns::CUSTOM_HOSTNAME_DB_SIZE; i++)
    {
        ASSERT_TRUE(_database.instance().read(database::Config::Section::Common::MdnsHostname, i, hostname_value));
        EXPECT_EQ(0U, hostname_value);
    }
}

TEST_F(DatabaseTest, Presets)
{
    size_t active_preset = 0;

    for (active_preset = 0; active_preset < _database.instance().supported_presets(); active_preset++)
    {
        ASSERT_TRUE(_database.instance().set_preset(active_preset));
        ASSERT_EQ(active_preset, _database.instance().current_preset());
    }

    // preset preservation is disabled by default which means that on init, preset should be reset back to 0
    ASSERT_TRUE(_database.instance().init(_handlers));
    ASSERT_EQ(0, _database.instance().current_preset());

    if (_database.instance().supported_presets() > 1)
    {
        // try setting value in preset 1, switch back to preset 0 and verify preset 0 still contains default value
        ASSERT_TRUE(_database.instance().set_preset(1));

        if (io::analog::Collection::size())
        {
            ASSERT_TRUE(_database.instance().update(database::Config::Section::Analog::MidiId, 0, 127));
        }

        ASSERT_TRUE(_database.instance().set_preset(0));

        if (io::analog::Collection::size())
        {
            ASSERT_EQ(0, _database.instance().read(database::Config::Section::Analog::MidiId, 0));
        }
    }

    // enable preset preservation, perform factory reset and verify that preservation is disabled
    _database.instance().set_preset_preserve_state(true);
    ASSERT_TRUE(_database.instance().preset_preserve_state());
    ASSERT_TRUE(_database.instance().factory_reset());
    ASSERT_TRUE(_database.instance().init(_handlers));
    ASSERT_FALSE(_database.instance().preset_preserve_state());
}

TEST_F(DatabaseTest, FactoryReset)
{
    // change several values
#ifdef PROJECT_TARGET_SUPPORT_SWITCHES
    ASSERT_TRUE(_database.instance().update(database::Config::Section::Switch::MidiId, 0, 114));
#endif

#ifdef PROJECT_TARGET_SUPPORT_ENCODERS
    ASSERT_TRUE(_database.instance().update(database::Config::Section::Encoder::Channel, 0, 11));
#endif

#ifdef PROJECT_TARGET_SUPPORT_I2C
    ASSERT_TRUE(_database.instance().update(database::Config::Section::I2c::Display,
                                            io::i2c::display::Setting::Controller,
                                            io::i2c::display::DisplayController::Ssd1306));
#endif

    _database.instance().set_preset_preserve_state(true);
    _database.instance().factory_reset();

    // expect default values
#ifdef PROJECT_TARGET_SUPPORT_SWITCHES
    DB_READ_VERIFY(0, database::Config::Section::Switch::MidiId, 0);
#endif

#ifdef PROJECT_TARGET_SUPPORT_ENCODERS
    DB_READ_VERIFY(1, database::Config::Section::Encoder::Channel, 0);
#endif

#ifdef PROJECT_TARGET_SUPPORT_I2C
    DB_READ_VERIFY(static_cast<int32_t>(io::i2c::display::DisplayController::Invalid),
                   database::Config::Section::I2c::Display,
                   io::i2c::display::Setting::Controller);
#endif

    ASSERT_FALSE(_database.instance().preset_preserve_state());
}

#ifdef PROJECT_TARGET_SUPPORT_OUTPUTS
TEST_F(DatabaseTest, Outputs)
{
    // regression test
    // by default, rgb state should be disabled
    for (size_t i = 0; i < io::outputs::Collection::size() / 3; i++)
    {
        ASSERT_FALSE(_database.instance().read(database::Config::Section::Outputs::RgbEnable, i));
    }

    ASSERT_TRUE(_database.instance().update(database::Config::Section::Outputs::ControlType, 0, io::outputs::ControlType::PC_SINGLE_VAL));

    // rgb state shouldn't change
    for (size_t i = 0; i < io::outputs::Collection::size() / 3; i++)
    {
        DB_READ_VERIFY(0, database::Config::Section::Outputs::RgbEnable, i);
    }
}
#endif
