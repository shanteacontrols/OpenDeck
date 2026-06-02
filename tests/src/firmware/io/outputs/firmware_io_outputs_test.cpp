/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/database.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OUTPUTS

#include "firmware/src/io/outputs/builder/builder.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/global/midi_program.h"

using namespace opendeck;
using namespace opendeck::firmware;

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

            // output tests exercise runtime update behavior directly, so unblock
            // the subsystem before init() requests its first refresh.
            io::Base::resume();

            // outputs calls HWA only for digital out group - for the other groups controls is done via dispatcher.
            // Once init() is called, all outputs should be turned off
            EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
                .Times(io::outputs::Collection::size(io::outputs::GroupDigitalOutputs));

            _outputs._instance.init();
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            signaling::clear_registry();
        }

        void wait_for_signal_dispatch()
        {
            ASSERT_TRUE(zlibs::utils::signaling::drain());
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

            wait_for_signal_dispatch();
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

        static uint8_t expected_level(uint8_t value)
        {
            return static_cast<uint8_t>((static_cast<uint32_t>(value) * (io::outputs::OUTPUT_LEVEL_MAX + 1U)) /
                                        (protocol::midi::MAX_VALUE_7BIT + 1U));
        }

        static io::outputs::PulseSpeed expected_pulse_speed(uint8_t value)
        {
            if (value < 16)
            {
                return io::outputs::PulseSpeed::NoPulse;
            }

            return static_cast<io::outputs::PulseSpeed>(value % 16 / 4);
        }

        void notify_midi_in(protocol::midi::MessageType message, uint8_t channel, uint16_t index, uint16_t value)
        {
            uint8_t command = 0;
            uint8_t p1      = static_cast<uint8_t>(index);
            uint8_t p2      = static_cast<uint8_t>(value);

            switch (message)
            {
            case protocol::midi::MessageType::NoteOn:
            {
                command = UMP_MIDI_NOTE_ON;
            }
            break;

            case protocol::midi::MessageType::NoteOff:
            {
                command = UMP_MIDI_NOTE_OFF;
            }
            break;

            case protocol::midi::MessageType::ControlChange:
            {
                command = UMP_MIDI_CONTROL_CHANGE;
            }
            break;

            case protocol::midi::MessageType::ProgramChange:
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

        void notify_local(signaling::IoEventSource    source,
                          protocol::midi::MessageType message,
                          uint8_t                     channel,
                          uint16_t                    index,
                          uint16_t                    value)
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

        void notify_osc_output(size_t index, int32_t value)
        {
            signaling::publish(signaling::OscIoSignal{
                .source          = signaling::IoEventSource::Output,
                .component_index = index,
                .int32_value     = value,
                .direction       = signaling::SignalDirection::In,
            });

            wait_for_signal_dispatch();
        }

        static constexpr size_t                 MIDI_CHANNEL  = 1;
        static constexpr std::array<uint8_t, 3> SAMPLE_VALUES = { 0, 64, 127 };

        struct PulseSpeedCase
        {
            uint8_t                 value;
            io::outputs::PulseSpeed speed;
        };

        static constexpr std::array<PulseSpeedCase, 10> PULSE_SPEED_CASES = { {
            { 0, io::outputs::PulseSpeed::NoPulse },
            { 15, io::outputs::PulseSpeed::NoPulse },
            { 16, io::outputs::PulseSpeed::Ms1000 },
            { 19, io::outputs::PulseSpeed::Ms1000 },
            { 20, io::outputs::PulseSpeed::Ms500 },
            { 23, io::outputs::PulseSpeed::Ms500 },
            { 24, io::outputs::PulseSpeed::Ms250 },
            { 27, io::outputs::PulseSpeed::Ms250 },
            { 28, io::outputs::PulseSpeed::NoPulse },
            { 127, io::outputs::PulseSpeed::NoPulse },
        } };

        tests::NoOpDatabaseHandlers _handlers;
        database::Builder           _builder_database;
        database::Admin&            _database_admin = _builder_database.instance();
        io::outputs::Builder        _outputs        = io::outputs::Builder(_database_admin);
        TouchscreenListener         _touchscreen_listener;
    };
}    // namespace

TEST_F(OutputsTest, MultiValue)
{
    if (!io::outputs::Collection::size(io::outputs::GroupDigitalOutputs))
    {
        return;
    }

    // MIDI_IN_NOTE_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < io::outputs::Collection::size(); i++)
    {
        ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, io::outputs::ControlType::MidiInNoteMultiVal));
    }

    for (size_t output = 0; output < io::outputs::Collection::size(io::outputs::GroupDigitalOutputs); output++)
    {
        EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
            .Times(io::outputs::Collection::size(io::outputs::GroupDigitalOutputs));

        _outputs._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_outputs._hwa, set_level(_, expected_level(value)))
                .Times(1);

            notify_midi_in(protocol::midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

            ASSERT_EQ(expected_pulse_speed(value), _outputs._instance.pulse_speed(output));
        }
    }

    // MIDI_IN_CC_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < io::outputs::Collection::size(); i++)
    {
        ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, io::outputs::ControlType::MidiInCcMultiVal));
    }

    for (size_t output = 0; output < io::outputs::Collection::size(io::outputs::GroupDigitalOutputs); output++)
    {
        EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
            .Times(io::outputs::Collection::size(io::outputs::GroupDigitalOutputs));

        _outputs._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_outputs._hwa, set_level(_, expected_level(value)))
                .Times(1);

            notify_midi_in(protocol::midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

            ASSERT_EQ(expected_pulse_speed(value), _outputs._instance.pulse_speed(output));
        }
    }

    // LOCAL_NOTE_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < io::outputs::Collection::size(); i++)
    {
        ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, io::outputs::ControlType::LocalNoteMultiVal));
    }

    for (size_t output = 0; output < io::outputs::Collection::size(io::outputs::GroupDigitalOutputs); output++)
    {
        EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
            .Times(io::outputs::Collection::size(io::outputs::GroupDigitalOutputs));

        _outputs._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_outputs._hwa, set_level(_, expected_level(value)))
                .Times(1);

            notify_local(signaling::IoEventSource::Switch, protocol::midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

            ASSERT_EQ(expected_pulse_speed(value), _outputs._instance.pulse_speed(output));
        }
    }

    // same test for analog components
    for (size_t output = 0; output < io::outputs::Collection::size(io::outputs::GroupDigitalOutputs); output++)
    {
        EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
            .Times(io::outputs::Collection::size(io::outputs::GroupDigitalOutputs));

        _outputs._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_outputs._hwa, set_level(_, expected_level(value)))
                .Times(1);

            notify_local(signaling::IoEventSource::Analog, protocol::midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

            ASSERT_EQ(expected_pulse_speed(value), _outputs._instance.pulse_speed(output));
        }
    }

    // LOCAL_CC_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < io::outputs::Collection::size(); i++)
    {
        ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, io::outputs::ControlType::LocalCcMultiVal));
    }

    for (size_t output = 0; output < io::outputs::Collection::size(io::outputs::GroupDigitalOutputs); output++)
    {
        EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
            .Times(io::outputs::Collection::size(io::outputs::GroupDigitalOutputs));

        _outputs._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_outputs._hwa, set_level(_, expected_level(value)))
                .Times(1);

            notify_local(signaling::IoEventSource::Switch, protocol::midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

            ASSERT_EQ(expected_pulse_speed(value), _outputs._instance.pulse_speed(output));
        }
    }

    for (size_t output = 0; output < io::outputs::Collection::size(io::outputs::GroupDigitalOutputs); output++)
    {
        EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
            .Times(io::outputs::Collection::size(io::outputs::GroupDigitalOutputs));

        _outputs._instance.set_all_off();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_outputs._hwa, set_level(_, expected_level(value)))
                .Times(1);

            notify_local(signaling::IoEventSource::Analog, protocol::midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

            ASSERT_EQ(expected_pulse_speed(value), _outputs._instance.pulse_speed(output));
        }
    }
}

TEST_F(OutputsTest, PulseSpeedBoundaries)
{
    if (!io::outputs::Collection::size(io::outputs::GroupDigitalOutputs))
    {
        return;
    }

    constexpr size_t OUTPUT_INDEX = 0;

    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, OUTPUT_INDEX, io::outputs::ControlType::MidiInNoteMultiVal));

    for (const auto& test_case : PULSE_SPEED_CASES)
    {
        EXPECT_CALL(_outputs._hwa, set_level(_, expected_level(test_case.value)))
            .Times(1);

        notify_midi_in(protocol::midi::MessageType::NoteOn, MIDI_CHANNEL, OUTPUT_INDEX, test_case.value);

        ASSERT_EQ(test_case.speed, _outputs._instance.pulse_speed(OUTPUT_INDEX));
    }
}

TEST_F(OutputsTest, SingleValue)
{
    if (!io::outputs::Collection::size(io::outputs::GroupDigitalOutputs))
    {
        return;
    }

    // MIDI_IN_NOTE_SINGLE_VAL
    //----------------------------------

    for (auto activation_value : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < io::outputs::Collection::size(); i++)
        {
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, io::outputs::ControlType::MidiInNoteSingleVal));
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ActivationValue, i, activation_value));
        }

        for (size_t output = 0; output < io::outputs::Collection::size(io::outputs::GroupDigitalOutputs); output++)
        {
            EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
                .Times(io::outputs::Collection::size(io::outputs::GroupDigitalOutputs));

            _outputs._instance.set_all_off();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_outputs._hwa, set_level(_, value == activation_value ? io::outputs::OUTPUT_LEVEL_MAX : io::outputs::OUTPUT_LEVEL_MIN))
                    .Times(1);

                notify_midi_in(protocol::midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

                ASSERT_EQ(io::outputs::PulseSpeed::NoPulse, _outputs._instance.pulse_speed(output));
            }
        }
    }

    // MIDI_IN_CC_SINGLE_VAL
    //----------------------------------

    for (auto activation_value : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < io::outputs::Collection::size(); i++)
        {
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, io::outputs::ControlType::MidiInCcSingleVal));
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ActivationValue, i, activation_value));
        }

        for (size_t output = 0; output < io::outputs::Collection::size(io::outputs::GroupDigitalOutputs); output++)
        {
            EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
                .Times(io::outputs::Collection::size(io::outputs::GroupDigitalOutputs));

            _outputs._instance.set_all_off();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_outputs._hwa, set_level(_, value == activation_value ? io::outputs::OUTPUT_LEVEL_MAX : io::outputs::OUTPUT_LEVEL_MIN))
                    .Times(1);

                notify_midi_in(protocol::midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

                ASSERT_EQ(io::outputs::PulseSpeed::NoPulse, _outputs._instance.pulse_speed(output));
            }
        }
    }

    // LOCAL_NOTE_SINGLE_VAL
    //----------------------------------

    for (auto activation_value : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < io::outputs::Collection::size(); i++)
        {
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, io::outputs::ControlType::LocalNoteSingleVal));
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ActivationValue, i, activation_value));
        }

        for (size_t output = 0; output < io::outputs::Collection::size(io::outputs::GroupDigitalOutputs); output++)
        {
            EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
                .Times(io::outputs::Collection::size(io::outputs::GroupDigitalOutputs));

            _outputs._instance.set_all_off();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_outputs._hwa, set_level(_, value == activation_value ? io::outputs::OUTPUT_LEVEL_MAX : io::outputs::OUTPUT_LEVEL_MIN))
                    .Times(1);

                notify_local(signaling::IoEventSource::Switch, protocol::midi::MessageType::NoteOn, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

                ASSERT_EQ(io::outputs::PulseSpeed::NoPulse, _outputs._instance.pulse_speed(output));
            }
        }
    }

    // LOCAL_CC_SINGLE_VAL
    //----------------------------------

    for (auto activation_value : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < io::outputs::Collection::size(); i++)
        {
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, io::outputs::ControlType::LocalCcSingleVal));
            ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ActivationValue, i, activation_value));
        }

        for (size_t output = 0; output < io::outputs::Collection::size(io::outputs::GroupDigitalOutputs); output++)
        {
            EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
                .Times(io::outputs::Collection::size(io::outputs::GroupDigitalOutputs));

            _outputs._instance.set_all_off();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_outputs._hwa, set_level(_, value == activation_value ? io::outputs::OUTPUT_LEVEL_MAX : io::outputs::OUTPUT_LEVEL_MIN))
                    .Times(1);

                notify_local(signaling::IoEventSource::Switch, protocol::midi::MessageType::ControlChange, MIDI_CHANNEL, static_cast<uint16_t>(output), value);

                ASSERT_EQ(io::outputs::PulseSpeed::NoPulse, _outputs._instance.pulse_speed(output));
            }
        }
    }
}

TEST_F(OutputsTest, SingleOutputState)
{
    if (!io::outputs::Collection::size(io::outputs::GroupDigitalOutputs))
    {
        return;
    }

    static constexpr size_t MIDI_ID = 0;

    // By default, outputs are configured to react on MIDI Note On.
    // Note 0 should turn the first output on
    EXPECT_CALL(_outputs._hwa, set_level(MIDI_ID, io::outputs::OUTPUT_LEVEL_MAX))
        .Times(1);

    notify_midi_in(protocol::midi::MessageType::NoteOn, MIDI_CHANNEL, MIDI_ID, 127);

    EXPECT_CALL(_outputs._hwa, set_level(MIDI_ID, io::outputs::OUTPUT_LEVEL_MIN))
        .Times(1);

    // now turn the output off
    notify_midi_in(protocol::midi::MessageType::NoteOn, MIDI_CHANNEL, MIDI_ID, 0);
}

TEST_F(OutputsTest, OscOutputLevel)
{
    if (!io::outputs::Collection::size(io::outputs::GroupDigitalOutputs))
    {
        return;
    }

    constexpr size_t OUTPUT_INDEX = 0;

    EXPECT_CALL(_outputs._hwa, set_level(OUTPUT_INDEX, io::outputs::OUTPUT_LEVEL_MIN))
        .Times(1);
    notify_osc_output(OUTPUT_INDEX, -1);

    EXPECT_CALL(_outputs._hwa, set_level(OUTPUT_INDEX, 64))
        .Times(1);
    notify_osc_output(OUTPUT_INDEX, 64);

    EXPECT_CALL(_outputs._hwa, set_level(OUTPUT_INDEX, io::outputs::OUTPUT_LEVEL_MAX))
        .Times(1);
    notify_osc_output(OUTPUT_INDEX, 127);
}

TEST_F(OutputsTest, SysExStateControlsOutput)
{
    if (!io::outputs::Collection::size(io::outputs::GroupDigitalOutputs))
    {
        return;
    }

    static constexpr size_t OUTPUT_INDEX = 0;
    uint16_t                state        = 0;

    EXPECT_CALL(_outputs._hwa, set_level(OUTPUT_INDEX, io::outputs::OUTPUT_LEVEL_MAX))
        .Times(1);

    ASSERT_EQ(sys::Config::Status::Ack,
              ConfigHandler.set(sys::Config::Block::Outputs,
                                static_cast<uint8_t>(sys::Config::Section::Outputs::State),
                                OUTPUT_INDEX,
                                1));

    ASSERT_EQ(sys::Config::Status::Ack,
              ConfigHandler.get(sys::Config::Block::Outputs,
                                static_cast<uint8_t>(sys::Config::Section::Outputs::State),
                                OUTPUT_INDEX,
                                state));
    ASSERT_EQ(1, state);

    EXPECT_CALL(_outputs._hwa, set_level(OUTPUT_INDEX, io::outputs::OUTPUT_LEVEL_MIN))
        .Times(1);

    ASSERT_EQ(sys::Config::Status::Ack,
              ConfigHandler.set(sys::Config::Block::Outputs,
                                static_cast<uint8_t>(sys::Config::Section::Outputs::State),
                                OUTPUT_INDEX,
                                0));

    ASSERT_EQ(sys::Config::Status::Ack,
              ConfigHandler.get(sys::Config::Block::Outputs,
                                static_cast<uint8_t>(sys::Config::Section::Outputs::State),
                                OUTPUT_INDEX,
                                state));
    ASSERT_EQ(0, state);
}

TEST_F(OutputsTest, ProgramChangeWithOffset)
{
    if (io::outputs::Collection::size(io::outputs::GroupDigitalOutputs) < 4)
    {
        return;
    }

    // configure first four outputs to indicate program change
    static constexpr size_t PC_OUTPUTS = 4;

    for (size_t i = 0; i < PC_OUTPUTS; i++)
    {
        ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, i, io::outputs::ControlType::PcSingleVal));
    }

    // notify program change
    uint8_t program = 0;

    // first output should be on, rest is off
    EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MAX))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
        .Times(3);

    notify_program(MIDI_CHANNEL, program);

    // now increase the program by 1
    program++;

    // second output should be on, rest is off
    EXPECT_CALL(_outputs._hwa, set_level(0, io::outputs::OUTPUT_LEVEL_MIN))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_level(1, io::outputs::OUTPUT_LEVEL_MAX))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_level(2, io::outputs::OUTPUT_LEVEL_MIN))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_level(3, io::outputs::OUTPUT_LEVEL_MIN))
        .Times(1);

    notify_program(MIDI_CHANNEL, program);

    // change the MIDI program offset
    MidiProgram.set_offset(1);

    // nothing should change yet
    // second output should be on, rest is off
    EXPECT_CALL(_outputs._hwa, set_level(0, io::outputs::OUTPUT_LEVEL_MIN))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_level(1, io::outputs::OUTPUT_LEVEL_MAX))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_level(2, io::outputs::OUTPUT_LEVEL_MIN))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_level(3, io::outputs::OUTPUT_LEVEL_MIN))
        .Times(1);

    notify_program(MIDI_CHANNEL, program);

    // enable output sync with offset
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::Global, io::outputs::Setting::UseMidiProgramOffset, 1));

    // notify the program 1 again
    // this time, due to the offset, first output should be on, and the rest should be off
    // when sync is active, all activation IDs for outputs that use program change message type are incremented by the program offset
    EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MAX))
        .Times(1);

    EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
        .Times(3);

    notify_program(MIDI_CHANNEL, program);
}

TEST_F(OutputsTest, StaticOutputsOnInitially)
{
    constexpr size_t OUTPUT_INDEX = 0;
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, OUTPUT_INDEX, io::outputs::ControlType::Static));

    if constexpr (io::outputs::Collection::size(io::outputs::GroupDigitalOutputs) != 0)
    {
        // Once init() is called, all outputs should be turned off
        EXPECT_CALL(_outputs._hwa, set_level(_, io::outputs::OUTPUT_LEVEL_MIN))
            .Times(io::outputs::Collection::size(io::outputs::GroupDigitalOutputs));

        // output at OUTPUT_INDEX should be turned on
        EXPECT_CALL(_outputs._hwa, set_level(OUTPUT_INDEX, io::outputs::OUTPUT_LEVEL_MAX))
            .Times(1);
    }
    else if constexpr (io::outputs::Collection::size(io::outputs::GroupTouchscreenComponents) != 0)
    {
        subscribe_touchscreen_listener();
    }

    _outputs._instance.init();

    if constexpr (io::outputs::Collection::size(io::outputs::GroupTouchscreenComponents) != 0 &&
                  io::outputs::Collection::size(io::outputs::GroupDigitalOutputs) == 0)
    {
        constexpr size_t EXPECTED_COMPONENT_INDEX = io::outputs::Collection::start_index(io::outputs::GroupTouchscreenComponents) + OUTPUT_INDEX;

        wait_for_touchscreen_output_events(io::outputs::Collection::size(io::outputs::GroupTouchscreenComponents));

        const auto matching_event = std::find_if(_touchscreen_listener.event_log.begin(),
                                                 _touchscreen_listener.event_log.end(),
                                                 [](const signaling::OscIoSignal& event)
                                                 {
                                                     return event.component_index == EXPECTED_COMPONENT_INDEX &&
                                                            event.int32_value == static_cast<int32_t>(io::outputs::OUTPUT_LEVEL_MAX);
                                                 });

        ASSERT_NE(_touchscreen_listener.event_log.end(), matching_event);
    }
}

TEST_F(OutputsTest, GlobalChannel)
{
    if (!io::outputs::Collection::size(io::outputs::GroupDigitalOutputs))
    {
        return;
    }

    constexpr auto OUTPUT_INDEX    = 0;
    const auto     default_channel = _outputs._database.read(database::Config::Section::Outputs::Channel, OUTPUT_INDEX);
    const auto     global_channel  = default_channel + 1;
    constexpr auto ON_VALUE        = 127;

    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Global::MidiSettings, protocol::midi::Setting::GlobalChannel, global_channel));
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Global::MidiSettings, protocol::midi::Setting::UseGlobalChannel, true));
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::ControlType, OUTPUT_INDEX, io::outputs::ControlType::MidiInNoteMultiVal));

    EXPECT_CALL(_outputs._hwa, set_level(_, _))
        .Times(0);

    // this shouldn't turn the output on because global channel is used instead of the default one

    notify_midi_in(protocol::midi::MessageType::NoteOn, default_channel, OUTPUT_INDEX, ON_VALUE);

    // verify that the output at OUTPUT_INDEX is turned on with global channel
    EXPECT_CALL(_outputs._hwa, set_level(_, expected_level(ON_VALUE)))
        .Times(1);

    notify_midi_in(protocol::midi::MessageType::NoteOn, global_channel, OUTPUT_INDEX, ON_VALUE);

    // disable the global channel but now use omni channel instead
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Global::MidiSettings, protocol::midi::Setting::UseGlobalChannel, false));
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::Channel, OUTPUT_INDEX, protocol::midi::OMNI_CHANNEL));

    for (size_t i = 0; i < 16; i++)
    {
        // the output should be turned on for every received channel
        EXPECT_CALL(_outputs._hwa, set_level(_, expected_level(ON_VALUE)))
            .Times(1);

        notify_midi_in(protocol::midi::MessageType::NoteOn, i, OUTPUT_INDEX, ON_VALUE);
    }

    // same test, but this time use omni as global channel
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Global::MidiSettings, protocol::midi::Setting::GlobalChannel, protocol::midi::OMNI_CHANNEL));
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Global::MidiSettings, protocol::midi::Setting::UseGlobalChannel, true));
    ASSERT_TRUE(_outputs._database.update(database::Config::Section::Outputs::Channel, OUTPUT_INDEX, 1));

    for (size_t i = 0; i < 16; i++)
    {
        // the output should be turned on for every received channel
        EXPECT_CALL(_outputs._hwa, set_level(_, expected_level(ON_VALUE)))
            .Times(1);

        notify_midi_in(protocol::midi::MessageType::NoteOn, i, OUTPUT_INDEX, ON_VALUE);
    }
}

#else
TEST(OutputsTest, SkippedWhenPresetDoesNotSupportOutputs)
{
    SUCCEED();
}
#endif
