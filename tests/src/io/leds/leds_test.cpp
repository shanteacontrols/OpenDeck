/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/database.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_LEDS

#include "io/leds/builder.h"
#include "util/configurable/configurable.h"
#include "global/midi_program.h"

using namespace opendeck::io;
using namespace opendeck::protocol;

namespace
{
    class TouchscreenListener
    {
        public:
        void on_message(const signaling::OscIoSignal& event)
        {
            event_log.push_back(event);
        }

        std::vector<signaling::OscIoSignal> event_log = {};
    };

    class LEDsTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_database_admin.factory_reset());
            ASSERT_EQ(0, _database_admin.current_preset());

            // LED tests exercise runtime update behavior directly, so unblock
            // the subsystem before init() requests its first refresh.
            io::Base::resume();

            // LEDs calls HWA only for digital out group - for the other groups controls is done via dispatcher.
            // Once init() is called, all LEDs should be turned off
            EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness_value.at(0)))
                .Times(leds::Collection::size(leds::GroupDigitalOutputs));

            _leds._instance.init();
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            signaling::clear_registry();
        }

        void wait_for_signal_dispatch()
        {
            k_msleep(1);
        }

        void subscribe_touchscreen_listener()
        {
            signaling::subscribe<signaling::OscIoSignal>(
                [this](const signaling::OscIoSignal& dispatch_message)
                {
                    if (dispatch_message.source != signaling::IoEventSource::Led)
                    {
                        return;
                    }

                    _touchscreen_listener.on_message(dispatch_message);
                });

            k_msleep(50);
            _touchscreen_listener.event_log.clear();
        }

        void wait_for_touchscreen_led_events(size_t expected_count)
        {
            constexpr uint32_t WAIT_STEP_MS    = 10;
            constexpr uint32_t WAIT_TIMEOUT_MS = 500;

            for (uint32_t waited = 0; waited < WAIT_TIMEOUT_MS; waited += WAIT_STEP_MS)
            {
                if (_touchscreen_listener.event_log.size() >= expected_count)
                {
                    return;
                }

                k_msleep(WAIT_STEP_MS);
            }

            ASSERT_GE(_touchscreen_listener.event_log.size(), expected_count);
        }

        static leds::Brightness expected_brightness(uint8_t value)
        {
            if (value < 16)
            {
                return leds::Brightness::Off;
            }

            return static_cast<leds::Brightness>((value % 16 % 4) + 1);
        }

        static leds::BlinkSpeed expected_blink_speed(uint8_t value)
        {
            if (value < 16)
            {
                return leds::BlinkSpeed::NoBlink;
            }

            return static_cast<leds::BlinkSpeed>(value % 16 / 4);
        }

        void notify_midi_in(midi::MessageType message, uint8_t channel, uint16_t index, uint16_t value)
        {
            uint8_t command = 0;
            uint8_t p1      = static_cast<uint8_t>(index);
            uint8_t p2      = static_cast<uint8_t>(value);

            switch (message)
            {
            case midi::MessageType::NoteOn:
            {
                command = UMP_MIDI_NOTE_ON;
            }
            break;

            case midi::MessageType::NoteOff:
            {
                command = UMP_MIDI_NOTE_OFF;
            }
            break;

            case midi::MessageType::ControlChange:
            {
                command = UMP_MIDI_CONTROL_CHANGE;
            }
            break;

            case midi::MessageType::ProgramChange:
            {
                command = UMP_MIDI_PROGRAM_CHANGE;
                p2      = 0;
            }
            break;

            default:
                return;
            }

            midi_ump packet = {};
            packet.data[0]  = (static_cast<uint32_t>(UMP_MT_MIDI1_CHANNEL_VOICE) << 28U) |
                              ((static_cast<uint32_t>(0U) & 0x0fU) << 24U) |
                              ((static_cast<uint32_t>(command) & 0x0fU) << 20U) |
                              ((static_cast<uint32_t>(channel - 1U) & 0x0fU) << 16U) |
                              ((static_cast<uint32_t>(p1) & 0x7fU) << 8U) |
                              (static_cast<uint32_t>(p2) & 0x7fU);

            signaling::publish(signaling::UmpSignal{
                .direction = signaling::SignalDirection::In,
                .packet    = packet,
            });

            wait_for_signal_dispatch();
        }

        void notify_local(signaling::IoEventSource source,
                          midi::MessageType        message,
                          uint8_t                  channel,
                          uint16_t                 index,
                          uint16_t                 value)
        {
            signaling::publish(signaling::MidiIoSignal{
                .source          = source,
                .component_index = 0,
                .channel         = channel,
                .index           = index,
                .value           = value,
                .message         = message,
            });

            wait_for_signal_dispatch();
        }

        void notify_program(uint8_t channel, uint16_t program)
        {
            signaling::publish(signaling::InternalProgram{
                .channel = channel,
                .index   = program,
                .value   = 0,
            });

            wait_for_signal_dispatch();
        }

        static constexpr size_t                 MIDI_CHANNEL  = 1;
        static constexpr std::array<uint8_t, 3> SAMPLE_VALUES = { 0, 64, 127 };

        // these tables should match with the one at https://github.com/shanteacontrols/OpenDeck/wiki/LED-control
        std::vector<leds::Brightness> expected_brightness_value = {
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Off,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
            leds::Brightness::Level25,
            leds::Brightness::Level50,
            leds::Brightness::Level75,
            leds::Brightness::Level100,
        };

        std::vector<leds::BlinkSpeed> expected_blink_speed_value = {
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms1000,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms500,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::Ms250,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
            leds::BlinkSpeed::NoBlink,
        };

        tests::NoOpDatabaseHandlers _handlers;
        database::Builder           _builder_database;
        database::Admin&            _database_admin = _builder_database.instance();
        leds::Builder               _leds           = leds::Builder(_database_admin);
        TouchscreenListener         _touchscreen_listener;
    };
}    // namespace

TEST_F(LEDsTest, MultiValue)
{
    if (!leds::Collection::size(leds::GroupDigitalOutputs))
    {
        return;
    }

    // MIDI_IN_NOTE_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < leds::Collection::size(); i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ControlType, i, leds::ControlType::MidiInNoteMultiVal));
    }

    for (size_t led = 0; led < leds::Collection::size(leds::GroupDigitalOutputs); led++)
    {
        EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(leds::Collection::size(leds::GroupDigitalOutputs));

        _leds._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness(value)))
                .Times(1);

            notify_midi_in(midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

            ASSERT_EQ(expected_blink_speed(value), _leds._instance.blink_speed(led));
        }
    }

    // MIDI_IN_CC_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < leds::Collection::size(); i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ControlType, i, leds::ControlType::MidiInCcMultiVal));
    }

    for (size_t led = 0; led < leds::Collection::size(leds::GroupDigitalOutputs); led++)
    {
        EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(leds::Collection::size(leds::GroupDigitalOutputs));

        _leds._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness(value)))
                .Times(1);

            notify_midi_in(midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

            ASSERT_EQ(expected_blink_speed(value), _leds._instance.blink_speed(led));
        }
    }

    // LOCAL_NOTE_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < leds::Collection::size(); i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ControlType, i, leds::ControlType::LocalNoteMultiVal));
    }

    for (size_t led = 0; led < leds::Collection::size(leds::GroupDigitalOutputs); led++)
    {
        EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(leds::Collection::size(leds::GroupDigitalOutputs));

        _leds._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness(value)))
                .Times(1);

            notify_local(signaling::IoEventSource::Button, midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

            ASSERT_EQ(expected_blink_speed(value), _leds._instance.blink_speed(led));
        }
    }

    // same test for analog components
    for (size_t led = 0; led < leds::Collection::size(leds::GroupDigitalOutputs); led++)
    {
        EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(leds::Collection::size(leds::GroupDigitalOutputs));

        _leds._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness(value)))
                .Times(1);

            notify_local(signaling::IoEventSource::Analog, midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

            ASSERT_EQ(expected_blink_speed(value), _leds._instance.blink_speed(led));
        }
    }

    // LOCAL_CC_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < leds::Collection::size(); i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ControlType, i, leds::ControlType::LocalCcMultiVal));
    }

    for (size_t led = 0; led < leds::Collection::size(leds::GroupDigitalOutputs); led++)
    {
        EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(leds::Collection::size(leds::GroupDigitalOutputs));

        _leds._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness(value)))
                .Times(1);

            notify_local(signaling::IoEventSource::Button, midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

            ASSERT_EQ(expected_blink_speed(value), _leds._instance.blink_speed(led));
        }
    }

    for (size_t led = 0; led < leds::Collection::size(leds::GroupDigitalOutputs); led++)
    {
        EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(leds::Collection::size(leds::GroupDigitalOutputs));

        _leds._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness(value)))
                .Times(1);

            notify_local(signaling::IoEventSource::Analog, midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

            ASSERT_EQ(expected_blink_speed(value), _leds._instance.blink_speed(led));
        }
    }
}

TEST_F(LEDsTest, SingleValue)
{
    if (!leds::Collection::size(leds::GroupDigitalOutputs))
    {
        return;
    }

    // MIDI_IN_NOTE_SINGLE_VAL
    //----------------------------------

    for (auto activation_value : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < leds::Collection::size(); i++)
        {
            ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ControlType, i, leds::ControlType::MidiInNoteSingleVal));
            ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ActivationValue, i, activation_value));
        }

        for (size_t led = 0; led < leds::Collection::size(leds::GroupDigitalOutputs); led++)
        {
            EXPECT_CALL(_leds._hwa, set_state(_, leds::Brightness::Off))
                .Times(leds::Collection::size(leds::GroupDigitalOutputs));

            _leds._instance.set_all_off();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_leds._hwa, set_state(_, value == activation_value ? leds::Brightness::Level100 : leds::Brightness::Off))
                    .Times(1);

                notify_midi_in(midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

                ASSERT_EQ(leds::BlinkSpeed::NoBlink, _leds._instance.blink_speed(led));
            }
        }
    }

    // MIDI_IN_CC_SINGLE_VAL
    //----------------------------------

    for (auto activation_value : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < leds::Collection::size(); i++)
        {
            ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ControlType, i, leds::ControlType::MidiInCcSingleVal));
            ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ActivationValue, i, activation_value));
        }

        for (size_t led = 0; led < leds::Collection::size(leds::GroupDigitalOutputs); led++)
        {
            EXPECT_CALL(_leds._hwa, set_state(_, leds::Brightness::Off))
                .Times(leds::Collection::size(leds::GroupDigitalOutputs));

            _leds._instance.set_all_off();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_leds._hwa, set_state(_, value == activation_value ? leds::Brightness::Level100 : leds::Brightness::Off))
                    .Times(1);

                notify_midi_in(midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

                ASSERT_EQ(leds::BlinkSpeed::NoBlink, _leds._instance.blink_speed(led));
            }
        }
    }

    // LOCAL_NOTE_SINGLE_VAL
    //----------------------------------

    for (auto activation_value : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < leds::Collection::size(); i++)
        {
            ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ControlType, i, leds::ControlType::LocalNoteSingleVal));
            ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ActivationValue, i, activation_value));
        }

        for (size_t led = 0; led < leds::Collection::size(leds::GroupDigitalOutputs); led++)
        {
            EXPECT_CALL(_leds._hwa, set_state(_, leds::Brightness::Off))
                .Times(leds::Collection::size(leds::GroupDigitalOutputs));

            _leds._instance.set_all_off();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_leds._hwa, set_state(_, value == activation_value ? leds::Brightness::Level100 : leds::Brightness::Off))
                    .Times(1);

                notify_local(signaling::IoEventSource::Button, midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

                ASSERT_EQ(leds::BlinkSpeed::NoBlink, _leds._instance.blink_speed(led));
            }
        }
    }

    // LOCAL_CC_SINGLE_VAL
    //----------------------------------

    for (auto activation_value : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < leds::Collection::size(); i++)
        {
            ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ControlType, i, leds::ControlType::LocalCcSingleVal));
            ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ActivationValue, i, activation_value));
        }

        for (size_t led = 0; led < leds::Collection::size(leds::GroupDigitalOutputs); led++)
        {
            EXPECT_CALL(_leds._hwa, set_state(_, leds::Brightness::Off))
                .Times(leds::Collection::size(leds::GroupDigitalOutputs));

            _leds._instance.set_all_off();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_leds._hwa, set_state(_, value == activation_value ? leds::Brightness::Level100 : leds::Brightness::Off))
                    .Times(1);

                notify_local(signaling::IoEventSource::Button, midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

                ASSERT_EQ(leds::BlinkSpeed::NoBlink, _leds._instance.blink_speed(led));
            }
        }
    }
}

TEST_F(LEDsTest, SingleLEDstate)
{
    if (!leds::Collection::size(leds::GroupDigitalOutputs))
    {
        return;
    }

    static constexpr size_t MIDI_ID = 0;

    // By default, leds are configured to react on MIDI Note On.
    // Note 0 should turn the first LED on
    EXPECT_CALL(_leds._hwa, set_state(MIDI_ID, leds::Brightness::Level100))
        .Times(1);

    notify_midi_in(midi::MessageType::NoteOn, MIDI_CHANNEL, MIDI_ID, 127);

    EXPECT_CALL(_leds._hwa, set_state(MIDI_ID, leds::Brightness::Off))
        .Times(1);

    // now turn the LED off
    notify_midi_in(midi::MessageType::NoteOn, MIDI_CHANNEL, MIDI_ID, 0);

    if (leds::Collection::size(leds::GroupDigitalOutputs) < 3)
    {
        return;
    }

    // configure RGB LED 0
    ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::RgbEnable, 0, 1));

    // now turn it on - expect three LEDs to be on

    EXPECT_CALL(_leds._hwa, set_state(_leds._hwa.rgb_component_from_rgb(0, leds::RgbComponent::R), leds::Brightness::Level100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, set_state(_leds._hwa.rgb_component_from_rgb(0, leds::RgbComponent::G), leds::Brightness::Level100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, set_state(_leds._hwa.rgb_component_from_rgb(0, leds::RgbComponent::B), leds::Brightness::Level100))
        .Times(1);

    notify_midi_in(midi::MessageType::NoteOn, MIDI_CHANNEL, MIDI_ID, 127);
}

TEST_F(LEDsTest, ProgramChangeWithOffset)
{
    if (leds::Collection::size(leds::GroupDigitalOutputs) < 4)
    {
        return;
    }

    // configure first four LEDs to indicate program change
    static constexpr size_t PC_LEDS = 4;

    for (size_t i = 0; i < PC_LEDS; i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ControlType, i, leds::ControlType::PcSingleVal));
    }

    // notify program change
    uint8_t program = 0;

    // first LED should be on, rest is off
    EXPECT_CALL(_leds._hwa, set_state(_, leds::Brightness::Level100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, set_state(_, leds::Brightness::Off))
        .Times(3);

    notify_program(MIDI_CHANNEL, program);

    // now increase the program by 1
    program++;

    // second LED should be on, rest is off
    EXPECT_CALL(_leds._hwa, set_state(0, leds::Brightness::Off))
        .Times(1);

    EXPECT_CALL(_leds._hwa, set_state(1, leds::Brightness::Level100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, set_state(2, leds::Brightness::Off))
        .Times(1);

    EXPECT_CALL(_leds._hwa, set_state(3, leds::Brightness::Off))
        .Times(1);

    notify_program(MIDI_CHANNEL, program);

    // change the MIDI program offset
    MidiProgram.set_offset(1);

    // nothing should change yet
    // second LED should be on, rest is off
    EXPECT_CALL(_leds._hwa, set_state(0, leds::Brightness::Off))
        .Times(1);

    EXPECT_CALL(_leds._hwa, set_state(1, leds::Brightness::Level100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, set_state(2, leds::Brightness::Off))
        .Times(1);

    EXPECT_CALL(_leds._hwa, set_state(3, leds::Brightness::Off))
        .Times(1);

    notify_program(MIDI_CHANNEL, program);

    // enable LED sync with offset
    ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::Global, leds::Setting::UseMidiProgramOffset, 1));

    // notify the program 1 again
    // this time, due to the offset, first LED should be on, and the rest should be off
    // when sync is active, all activation IDs for LEDs that use program change message type are incremented by the program offset
    EXPECT_CALL(_leds._hwa, set_state(_, leds::Brightness::Level100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, set_state(_, leds::Brightness::Off))
        .Times(3);

    notify_program(MIDI_CHANNEL, program);
}

TEST_F(LEDsTest, StaticLEDsOnInitially)
{
    constexpr size_t LED_INDEX = 0;
    ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ControlType, LED_INDEX, leds::ControlType::Static));

    if constexpr (leds::Collection::size(leds::GroupDigitalOutputs) != 0)
    {
        // Once init() is called, all LEDs should be turned off
        EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(leds::Collection::size(leds::GroupDigitalOutputs));

        // LED_INDEX should be turned on
        EXPECT_CALL(_leds._hwa, set_state(LED_INDEX, leds::Brightness::Level100))
            .Times(1);
    }
    else if constexpr (leds::Collection::size(leds::GroupTouchscreenComponents) != 0)
    {
        subscribe_touchscreen_listener();
    }

    _leds._instance.init();

    if constexpr (leds::Collection::size(leds::GroupTouchscreenComponents) != 0 &&
                  leds::Collection::size(leds::GroupDigitalOutputs) == 0)
    {
        constexpr size_t EXPECTED_COMPONENT_INDEX = leds::Collection::start_index(leds::GroupTouchscreenComponents) + LED_INDEX;

        wait_for_touchscreen_led_events(leds::Collection::size(leds::GroupTouchscreenComponents));

        const auto matching_event = std::find_if(_touchscreen_listener.event_log.begin(),
                                                 _touchscreen_listener.event_log.end(),
                                                 [](const signaling::OscIoSignal& event)
                                                 {
                                                     return event.component_index == EXPECTED_COMPONENT_INDEX &&
                                                            event.int32_value == static_cast<int32_t>(leds::Brightness::Level100);
                                                 });

        ASSERT_NE(_touchscreen_listener.event_log.end(), matching_event);
    }
}

TEST_F(LEDsTest, GlobalChannel)
{
    if (!leds::Collection::size(leds::GroupDigitalOutputs))
    {
        return;
    }

    constexpr auto LED_INDEX       = 0;
    const auto     default_channel = _leds._database.read(database::Config::Section::Leds::Channel, LED_INDEX);
    const auto     global_channel  = default_channel + 1;
    constexpr auto ON_VALUE        = 127;

    ASSERT_TRUE(_leds._database.update(database::Config::Section::Global::MidiSettings, midi::Setting::GlobalChannel, global_channel));
    ASSERT_TRUE(_leds._database.update(database::Config::Section::Global::MidiSettings, midi::Setting::UseGlobalChannel, true));
    ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::ControlType, LED_INDEX, leds::ControlType::MidiInNoteMultiVal));

    EXPECT_CALL(_leds._hwa, set_state(_, _))
        .Times(0);

    // this shouldn't turn the led on because global channel is used instead of the default one

    notify_midi_in(midi::MessageType::NoteOn, default_channel, LED_INDEX, ON_VALUE);

    // verify that the led LED_INDEX is turned on with global channel
    EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness(ON_VALUE)))
        .Times(1);

    notify_midi_in(midi::MessageType::NoteOn, global_channel, LED_INDEX, ON_VALUE);

    // disable the global channel but now use omni channel instead
    ASSERT_TRUE(_leds._database.update(database::Config::Section::Global::MidiSettings, midi::Setting::UseGlobalChannel, false));
    ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::Channel, LED_INDEX, midi::OMNI_CHANNEL));

    for (size_t i = 0; i < 16; i++)
    {
        // the led should be turned on for every received channel
        EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness(ON_VALUE)))
            .Times(1);

        notify_midi_in(midi::MessageType::NoteOn, i, LED_INDEX, ON_VALUE);
    }

    // same test, but this time use omni as global channel
    ASSERT_TRUE(_leds._database.update(database::Config::Section::Global::MidiSettings, midi::Setting::GlobalChannel, midi::OMNI_CHANNEL));
    ASSERT_TRUE(_leds._database.update(database::Config::Section::Global::MidiSettings, midi::Setting::UseGlobalChannel, true));
    ASSERT_TRUE(_leds._database.update(database::Config::Section::Leds::Channel, LED_INDEX, 1));

    for (size_t i = 0; i < 16; i++)
    {
        // the led should be turned on for every received channel
        EXPECT_CALL(_leds._hwa, set_state(_, expected_brightness(ON_VALUE)))
            .Times(1);

        notify_midi_in(midi::MessageType::NoteOn, i, LED_INDEX, ON_VALUE);
    }
}

#else
TEST(LedsTest, SkippedWhenPresetDoesNotSupportLeds)
{
    SUCCEED();
}
#endif
