/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/database.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OUTPUTS

#include "firmware/src/io/outputs/builder.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/global/midi_program.h"

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

    class OutputsTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_database_admin.factory_reset());
            ASSERT_EQ(0, _database_admin.current_preset());

            // OUTPUT tests exercise runtime update behavior directly, so unblock
            // the subsystem before init() requests its first refresh.
            io::Base::resume();

            // Outputs calls HWA only for digital out group - for the other groups controls is done via dispatcher.
            // Once init() is called, all Outputs should be turned off
            EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness_value.at(0)))
                .Times(outputs::Collection::size(outputs::GroupDigitalOutputs));

            _outputs._instance.init();
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
                    if (dispatch_message.source != signaling::IoEventSource::Output)
                    {
                        return;
                    }

                    _touchscreen_listener.on_message(dispatch_message);
                });

            k_msleep(50);
            _touchscreen_listener.event_log.clear();
        }

        void wait_for_touchscreen_output_events(size_t expected_count)
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

        static outputs::Brightness expected_brightness(uint8_t value)
        {
            if (value < 16)
            {
                return outputs::Brightness::Off;
            }

            return static_cast<outputs::Brightness>((value % 16 % 4) + 1);
        }

        static outputs::BlinkSpeed expected_blink_speed(uint8_t value)
        {
            if (value < 16)
            {
                return outputs::BlinkSpeed::NoBlink;
            }

            return static_cast<outputs::BlinkSpeed>(value % 16 / 4);
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

        // these tables should match with the one at https://github.com/shanteacontrols/OpenDeck/wiki/OUTPUT-control
        std::vector<outputs::Brightness> expected_brightness_value = {
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Off,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
            outputs::Brightness::Level25,
            outputs::Brightness::Level50,
            outputs::Brightness::Level75,
            outputs::Brightness::Level100,
        };

        std::vector<outputs::BlinkSpeed> expected_blink_speed_value = {
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms1000,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms500,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::Ms250,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
            outputs::BlinkSpeed::NoBlink,
        };

        tests::NoOpDatabaseHandlers _handlers;
        database::Builder           _builder_database;
        database::Admin&            _database_admin = _builder_database.instance();
        outputs::Builder            _outputs        = outputs::Builder(_database_admin);
        TouchscreenListener         _touchscreen_listener;
    };
}    // namespace

TEST_F(OutputsTest, MultiValue)
{
    if (!outputs::Collection::size(outputs::GroupDigitalOutputs))
    {
        return;
    }

    // MIDI_IN_NOTE_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < outputs::Collection::size(); i++)
    {
        ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, outputs::ControlType::MidiInNoteMultiVal));
    }

    for (size_t output = 0; output < outputs::Collection::size(outputs::GroupDigitalOutputs); output++)
    {
        EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(outputs::Collection::size(outputs::GroupDigitalOutputs));

        _outputs._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness(value)))
                .Times(1);

            notify_midi_in(midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

            ASSERT_EQ(expected_blink_speed(value), _outputs._instance.blink_speed(output));
        }
    }

    // MIDI_IN_CC_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < outputs::Collection::size(); i++)
    {
        ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, outputs::ControlType::MidiInCcMultiVal));
    }

    for (size_t output = 0; output < outputs::Collection::size(outputs::GroupDigitalOutputs); output++)
    {
        EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(outputs::Collection::size(outputs::GroupDigitalOutputs));

        _outputs._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness(value)))
                .Times(1);

            notify_midi_in(midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

            ASSERT_EQ(expected_blink_speed(value), _outputs._instance.blink_speed(output));
        }
    }

    // LOCAL_NOTE_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < outputs::Collection::size(); i++)
    {
        ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, outputs::ControlType::LocalNoteMultiVal));
    }

    for (size_t output = 0; output < outputs::Collection::size(outputs::GroupDigitalOutputs); output++)
    {
        EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(outputs::Collection::size(outputs::GroupDigitalOutputs));

        _outputs._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness(value)))
                .Times(1);

            notify_local(signaling::IoEventSource::Switch, midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

            ASSERT_EQ(expected_blink_speed(value), _outputs._instance.blink_speed(output));
        }
    }

    // same test for analog components
    for (size_t output = 0; output < outputs::Collection::size(outputs::GroupDigitalOutputs); output++)
    {
        EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(outputs::Collection::size(outputs::GroupDigitalOutputs));

        _outputs._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness(value)))
                .Times(1);

            notify_local(signaling::IoEventSource::Analog, midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

            ASSERT_EQ(expected_blink_speed(value), _outputs._instance.blink_speed(output));
        }
    }

    // LOCAL_CC_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < outputs::Collection::size(); i++)
    {
        ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, outputs::ControlType::LocalCcMultiVal));
    }

    for (size_t output = 0; output < outputs::Collection::size(outputs::GroupDigitalOutputs); output++)
    {
        EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(outputs::Collection::size(outputs::GroupDigitalOutputs));

        _outputs._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness(value)))
                .Times(1);

            notify_local(signaling::IoEventSource::Switch, midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

            ASSERT_EQ(expected_blink_speed(value), _outputs._instance.blink_speed(output));
        }
    }

    for (size_t output = 0; output < outputs::Collection::size(outputs::GroupDigitalOutputs); output++)
    {
        EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(outputs::Collection::size(outputs::GroupDigitalOutputs));

        _outputs._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness(value)))
                .Times(1);

            notify_local(signaling::IoEventSource::Analog, midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

            ASSERT_EQ(expected_blink_speed(value), _outputs._instance.blink_speed(output));
        }
    }
}

TEST_F(OutputsTest, SingleValue)
{
    if (!outputs::Collection::size(outputs::GroupDigitalOutputs))
    {
        return;
    }

    // MIDI_IN_NOTE_SINGLE_VAL
    //----------------------------------

    for (auto activation_value : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < outputs::Collection::size(); i++)
        {
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, outputs::ControlType::MidiInNoteSingleVal));
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ActivationValue, i, activation_value));
        }

        for (size_t output = 0; output < outputs::Collection::size(outputs::GroupDigitalOutputs); output++)
        {
            EXPECT_CALL(_outputs._hwa, set_state(_, outputs::Brightness::Off))
                .Times(outputs::Collection::size(outputs::GroupDigitalOutputs));

            _outputs._instance.set_all_off();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_outputs._hwa, set_state(_, value == activation_value ? outputs::Brightness::Level100 : outputs::Brightness::Off))
                    .Times(1);

                notify_midi_in(midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

                ASSERT_EQ(outputs::BlinkSpeed::NoBlink, _outputs._instance.blink_speed(output));
            }
        }
    }

    // MIDI_IN_CC_SINGLE_VAL
    //----------------------------------

    for (auto activation_value : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < outputs::Collection::size(); i++)
        {
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, outputs::ControlType::MidiInCcSingleVal));
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ActivationValue, i, activation_value));
        }

        for (size_t output = 0; output < outputs::Collection::size(outputs::GroupDigitalOutputs); output++)
        {
            EXPECT_CALL(_outputs._hwa, set_state(_, outputs::Brightness::Off))
                .Times(outputs::Collection::size(outputs::GroupDigitalOutputs));

            _outputs._instance.set_all_off();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_outputs._hwa, set_state(_, value == activation_value ? outputs::Brightness::Level100 : outputs::Brightness::Off))
                    .Times(1);

                notify_midi_in(midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

                ASSERT_EQ(outputs::BlinkSpeed::NoBlink, _outputs._instance.blink_speed(output));
            }
        }
    }

    // LOCAL_NOTE_SINGLE_VAL
    //----------------------------------

    for (auto activation_value : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < outputs::Collection::size(); i++)
        {
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, outputs::ControlType::LocalNoteSingleVal));
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ActivationValue, i, activation_value));
        }

        for (size_t output = 0; output < outputs::Collection::size(outputs::GroupDigitalOutputs); output++)
        {
            EXPECT_CALL(_outputs._hwa, set_state(_, outputs::Brightness::Off))
                .Times(outputs::Collection::size(outputs::GroupDigitalOutputs));

            _outputs._instance.set_all_off();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_outputs._hwa, set_state(_, value == activation_value ? outputs::Brightness::Level100 : outputs::Brightness::Off))
                    .Times(1);

                notify_local(signaling::IoEventSource::Switch, midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

                ASSERT_EQ(outputs::BlinkSpeed::NoBlink, _outputs._instance.blink_speed(output));
            }
        }
    }

    // LOCAL_CC_SINGLE_VAL
    //----------------------------------

    for (auto activation_value : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < outputs::Collection::size(); i++)
        {
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, outputs::ControlType::LocalCcSingleVal));
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ActivationValue, i, activation_value));
        }

        for (size_t output = 0; output < outputs::Collection::size(outputs::GroupDigitalOutputs); output++)
        {
            EXPECT_CALL(_outputs._hwa, set_state(_, outputs::Brightness::Off))
                .Times(outputs::Collection::size(outputs::GroupDigitalOutputs));

            _outputs._instance.set_all_off();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_outputs._hwa, set_state(_, value == activation_value ? outputs::Brightness::Level100 : outputs::Brightness::Off))
                    .Times(1);

                notify_local(signaling::IoEventSource::Switch, midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

                ASSERT_EQ(outputs::BlinkSpeed::NoBlink, _outputs._instance.blink_speed(output));
            }
        }
    }
}

TEST_F(OutputsTest, SingleOutputState)
{
    if (!outputs::Collection::size(outputs::GroupDigitalOutputs))
    {
        return;
    }

    static constexpr size_t MIDI_ID = 0;

    // By default, outputs are configured to react on MIDI Note On.
    // Note 0 should turn the first OUTPUT on
    EXPECT_CALL(_outputs._hwa, set_state(MIDI_ID, outputs::Brightness::Level100))
        .Times(1);

    notify_midi_in(midi::MessageType::NoteOn, MIDI_CHANNEL, MIDI_ID, 127);

    EXPECT_CALL(_outputs._hwa, set_state(MIDI_ID, outputs::Brightness::Off))
        .Times(1);

    // now turn the OUTPUT off
    notify_midi_in(midi::MessageType::NoteOn, MIDI_CHANNEL, MIDI_ID, 0);

    if (outputs::Collection::size(outputs::GroupDigitalOutputs) < 3)
    {
        return;
    }

    // configure RGB OUTPUT 0
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::RgbEnable, 0, 1));

    // now turn it on - expect three Outputs to be on

    EXPECT_CALL(_outputs._hwa, set_state(_outputs._hwa.rgb_component_from_rgb(0, outputs::RgbComponent::R), outputs::Brightness::Level100))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_state(_outputs._hwa.rgb_component_from_rgb(0, outputs::RgbComponent::G), outputs::Brightness::Level100))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_state(_outputs._hwa.rgb_component_from_rgb(0, outputs::RgbComponent::B), outputs::Brightness::Level100))
        .Times(1);

    notify_midi_in(midi::MessageType::NoteOn, MIDI_CHANNEL, MIDI_ID, 127);
}

TEST_F(OutputsTest, ProgramChangeWithOffset)
{
    if (outputs::Collection::size(outputs::GroupDigitalOutputs) < 4)
    {
        return;
    }

    // configure first four Outputs to indicate program change
    static constexpr size_t PC_OUTPUTS = 4;

    for (size_t i = 0; i < PC_OUTPUTS; i++)
    {
        ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, outputs::ControlType::PcSingleVal));
    }

    // notify program change
    uint8_t program = 0;

    // first OUTPUT should be on, rest is off
    EXPECT_CALL(_outputs._hwa, set_state(_, outputs::Brightness::Level100))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_state(_, outputs::Brightness::Off))
        .Times(3);

    notify_program(MIDI_CHANNEL, program);

    // now increase the program by 1
    program++;

    // second OUTPUT should be on, rest is off
    EXPECT_CALL(_outputs._hwa, set_state(0, outputs::Brightness::Off))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_state(1, outputs::Brightness::Level100))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_state(2, outputs::Brightness::Off))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_state(3, outputs::Brightness::Off))
        .Times(1);

    notify_program(MIDI_CHANNEL, program);

    // change the MIDI program offset
    MidiProgram.set_offset(1);

    // nothing should change yet
    // second OUTPUT should be on, rest is off
    EXPECT_CALL(_outputs._hwa, set_state(0, outputs::Brightness::Off))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_state(1, outputs::Brightness::Level100))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_state(2, outputs::Brightness::Off))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_state(3, outputs::Brightness::Off))
        .Times(1);

    notify_program(MIDI_CHANNEL, program);

    // enable OUTPUT sync with offset
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::Global, outputs::Setting::UseMidiProgramOffset, 1));

    // notify the program 1 again
    // this time, due to the offset, first OUTPUT should be on, and the rest should be off
    // when sync is active, all activation IDs for Outputs that use program change message type are incremented by the program offset
    EXPECT_CALL(_outputs._hwa, set_state(_, outputs::Brightness::Level100))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_state(_, outputs::Brightness::Off))
        .Times(3);

    notify_program(MIDI_CHANNEL, program);
}

TEST_F(OutputsTest, StaticOutputsOnInitially)
{
    constexpr size_t OUTPUT_INDEX = 0;
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, OUTPUT_INDEX, outputs::ControlType::Static));

    if constexpr (outputs::Collection::size(outputs::GroupDigitalOutputs) != 0)
    {
        // Once init() is called, all Outputs should be turned off
        EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness_value.at(0)))
            .Times(outputs::Collection::size(outputs::GroupDigitalOutputs));

        // OUTPUT_INDEX should be turned on
        EXPECT_CALL(_outputs._hwa, set_state(OUTPUT_INDEX, outputs::Brightness::Level100))
            .Times(1);
    }
    else if constexpr (outputs::Collection::size(outputs::GroupTouchscreenComponents) != 0)
    {
        subscribe_touchscreen_listener();
    }

    _outputs._instance.init();

    if constexpr (outputs::Collection::size(outputs::GroupTouchscreenComponents) != 0 &&
                  outputs::Collection::size(outputs::GroupDigitalOutputs) == 0)
    {
        constexpr size_t EXPECTED_COMPONENT_INDEX = outputs::Collection::start_index(outputs::GroupTouchscreenComponents) + OUTPUT_INDEX;

        wait_for_touchscreen_output_events(outputs::Collection::size(outputs::GroupTouchscreenComponents));

        const auto matching_event = std::find_if(_touchscreen_listener.event_log.begin(),
                                                 _touchscreen_listener.event_log.end(),
                                                 [](const signaling::OscIoSignal& event)
                                                 {
                                                     return event.component_index == EXPECTED_COMPONENT_INDEX &&
                                                            event.int32_value == static_cast<int32_t>(outputs::Brightness::Level100);
                                                 });

        ASSERT_NE(_touchscreen_listener.event_log.end(), matching_event);
    }
}

TEST_F(OutputsTest, GlobalChannel)
{
    if (!outputs::Collection::size(outputs::GroupDigitalOutputs))
    {
        return;
    }

    constexpr auto OUTPUT_INDEX    = 0;
    const auto     default_channel = _outputs._database.read(database::Config::Section::Outputs::Channel, OUTPUT_INDEX);
    const auto     global_channel  = default_channel + 1;
    constexpr auto ON_VALUE        = 127;

    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Global::MidiSettings, midi::Setting::GlobalChannel, global_channel));
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Global::MidiSettings, midi::Setting::UseGlobalChannel, true));
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, OUTPUT_INDEX, outputs::ControlType::MidiInNoteMultiVal));

    EXPECT_CALL(_outputs._hwa, set_state(_, _))
        .Times(0);

    // this shouldn't turn the output on because global channel is used instead of the default one

    notify_midi_in(midi::MessageType::NoteOn, default_channel, OUTPUT_INDEX, ON_VALUE);

    // verify that the output OUTPUT_INDEX is turned on with global channel
    EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness(ON_VALUE)))
        .Times(1);

    notify_midi_in(midi::MessageType::NoteOn, global_channel, OUTPUT_INDEX, ON_VALUE);

    // disable the global channel but now use omni channel instead
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Global::MidiSettings, midi::Setting::UseGlobalChannel, false));
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::Channel, OUTPUT_INDEX, midi::OMNI_CHANNEL));

    for (size_t i = 0; i < 16; i++)
    {
        // the output should be turned on for every received channel
        EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness(ON_VALUE)))
            .Times(1);

        notify_midi_in(midi::MessageType::NoteOn, i, OUTPUT_INDEX, ON_VALUE);
    }

    // same test, but this time use omni as global channel
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Global::MidiSettings, midi::Setting::GlobalChannel, midi::OMNI_CHANNEL));
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Global::MidiSettings, midi::Setting::UseGlobalChannel, true));
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::Channel, OUTPUT_INDEX, 1));

    for (size_t i = 0; i < 16; i++)
    {
        // the output should be turned on for every received channel
        EXPECT_CALL(_outputs._hwa, set_state(_, expected_brightness(ON_VALUE)))
            .Times(1);

        notify_midi_in(midi::MessageType::NoteOn, i, OUTPUT_INDEX, ON_VALUE);
    }
}

#else
TEST(OutputsTest, SkippedWhenPresetDoesNotSupportOutputs)
{
    SUCCEED();
}
#endif
