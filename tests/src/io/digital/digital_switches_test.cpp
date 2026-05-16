/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/database.h"
#include "tests/helpers/misc.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_SWITCHES

#include "firmware/src/io/digital/builder/builder.h"
#include "firmware/src/util/configurable/configurable.h"

#include "zlibs/utils/misc/ring_buffer.h"
#include "zlibs/utils/misc/mutex.h"

#include <limits>

using namespace opendeck::io;
using namespace opendeck::protocol;

namespace
{
    struct TestEvent
    {
        size_t                 component_index = 0;
        uint8_t                channel         = 0;
        uint16_t               index           = 0;
        uint16_t               value           = 0;
        midi::MessageType      message         = midi::MessageType::Invalid;
        signaling::SystemEvent system_event    = {};
    };

    struct OscEvent
    {
        size_t   component_index = 0;
        uint16_t value           = 0;
    };

    class Listener
    {
        public:
        void push(const signaling::MidiIoSignal& signal)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);

            event_log.push_back(TestEvent{
                .component_index = signal.component_index,
                .channel         = signal.channel,
                .index           = signal.index,
                .value           = signal.value,
                .message         = signal.message,
            });
        }

        void push(const signaling::SystemSignal& signal)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);

            event_log.push_back(TestEvent{
                .component_index = 0,
                .channel         = 0,
                .index           = 0,
                .value           = signal.value,
                .message         = midi::MessageType::Invalid,
                .system_event    = signal.system_event,
            });
        }

        void push(const signaling::OscIoSignal& signal)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);

            osc_event_log.push_back(OscEvent{
                .component_index = signal.component_index,
                .value           = static_cast<uint16_t>(signal.int32_value.value_or(0)),
            });
        }

        void clear()
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            event_log.clear();
            osc_event_log.clear();
        }

        size_t size() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return event_log.size();
        }

        TestEvent at(size_t index) const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return event_log.at(index);
        }

        std::vector<TestEvent> snapshot() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return event_log;
        }

        std::vector<OscEvent> osc_snapshot() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return osc_event_log;
        }

        private:
        mutable zlibs::utils::misc::Mutex _mutex;

        public:
        std::vector<TestEvent> event_log     = {};
        std::vector<OscEvent>  osc_event_log = {};
    };

    class DigitalSwitchesTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            tests::resume_io();

            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_database_admin.factory_reset());
            ASSERT_EQ(0, _database_admin.current_preset());

            for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
            {
                ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type, i, switches::Type::Momentary));
                ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, i, switches::MessageType::Note));
                ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Value, i, 127));

                _digital._builderSwitches._instance.reset(i);
            }

            signaling::subscribe<signaling::MidiIoSignal>(
                [this](const signaling::MidiIoSignal& signal)
                {
                    if (signal.source == signaling::IoEventSource::Switch)
                    {
                        _listener.push(signal);
                    }
                });

            signaling::subscribe<signaling::SystemSignal>(
                [this](const signaling::SystemSignal& signal)
                {
                    _listener.push(signal);
                });

            signaling::subscribe<signaling::OscIoSignal>(
                [this](const signaling::OscIoSignal& signal)
                {
                    if (signal.source == signaling::IoEventSource::Switch)
                    {
                        _listener.push(signal);
                    }
                });

            k_msleep(20);
            _listener.clear();
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            signaling::clear_registry();
            _listener.clear();
        }

        void state_change_register_all(bool state)
        {
            _listener.clear();

            EXPECT_CALL(_digital._builderSwitches._hwa, state(_))
                .WillRepeatedly(Return(std::optional<bool>(state)));

            EXPECT_CALL(_digital._builderEncoders._hwa, state(_))
                .WillRepeatedly(Return(std::optional<uint8_t>(0b00)));

            _digital._instance.process_state_changes();
            wait_for_signals();
        }

        void state_change_register_single(size_t index, bool state)
        {
            ASSERT_LT(index, switches::Collection::size(switches::GroupDigitalInputs));

            _listener.clear();

            // Only the target switch transitions; all others stay released.
            // DigitalTest::process_state_changes calls _switches.process_state_changes(), so every
            // switch HWA is queried - we must avoid activating unrelated switches.
            EXPECT_CALL(_digital._builderSwitches._hwa, state(Ne(index)))
                .WillRepeatedly(Return(std::optional<bool>(false)));
            EXPECT_CALL(_digital._builderSwitches._hwa, state(Eq(index)))
                .WillRepeatedly(Return(std::optional<bool>(state)));

            EXPECT_CALL(_digital._builderEncoders._hwa, state(_))
                .WillRepeatedly(Return(std::optional<uint8_t>(0b00)));

            _digital._instance.process_state_changes();
            wait_for_signals();
        }

        void wait_for_signals()
        {
            static constexpr size_t STABLE_ITERATION_TARGET = 2;
            static constexpr size_t MAX_ITERATIONS          = 8;

            size_t stable_iterations = 0;
            size_t last_size         = std::numeric_limits<size_t>::max();
            size_t max_iterations    = 0;

            while ((stable_iterations < STABLE_ITERATION_TARGET) && (max_iterations < MAX_ITERATIONS))
            {
                const size_t current_size = _listener.size();

                if (current_size == last_size)
                {
                    stable_iterations++;
                }
                else
                {
                    stable_iterations = 0;
                    last_size         = current_size;
                }

                k_msleep(1);
                max_iterations++;
            }
        }

        static size_t digital_switch_count()
        {
            return switches::Collection::size(switches::GroupDigitalInputs);
        }

        tests::NoOpDatabaseHandlers _handlers;
        Listener                    _listener;
        database::Builder           _builder_database;
        database::Admin&            _database_admin = _builder_database.instance();
        io::digital::Builder        _digital        = io::digital::Builder(_database_admin);
    };
}    // namespace

TEST(SwitchesBufferTest, PreservesOrderAndDrains)
{
    static constexpr uint8_t                                       BUFFER_CAPACITY = 8;
    zlibs::utils::misc::RingBuffer<BUFFER_CAPACITY, true, uint8_t> buffer          = {};

    EXPECT_TRUE(buffer.is_empty());
    EXPECT_FALSE(buffer.remove().has_value());

    EXPECT_TRUE(buffer.insert(1));
    EXPECT_TRUE(buffer.insert(2));
    EXPECT_TRUE(buffer.insert(3));

    ASSERT_FALSE(buffer.is_empty());

    auto value = buffer.remove();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(1, value.value());

    value = buffer.remove();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(2, value.value());

    value = buffer.remove();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(3, value.value());

    EXPECT_FALSE(buffer.remove().has_value());
    EXPECT_TRUE(buffer.is_empty());
}

TEST(SwitchesBufferTest, OverwritesOldestUnreadSampleWhenFull)
{
    static constexpr uint8_t                                       BUFFER_CAPACITY = 8;
    zlibs::utils::misc::RingBuffer<BUFFER_CAPACITY, true, uint8_t> buffer          = {};
    constexpr uint8_t                                              OVERFLOW_COUNT  = 2;

    for (uint8_t i = 0; i < (BUFFER_CAPACITY + OVERFLOW_COUNT); i++)
    {
        EXPECT_TRUE(buffer.insert(i));
    }

    for (uint8_t expected = OVERFLOW_COUNT; expected < (BUFFER_CAPACITY + OVERFLOW_COUNT); expected++)
    {
        auto value = buffer.remove();
        ASSERT_TRUE(value.has_value());
        EXPECT_EQ(expected, value.value());
    }

    EXPECT_FALSE(buffer.remove().has_value());
}

TEST_F(DigitalSwitchesTest, Note)
{
    if (!DigitalSwitchesTest::digital_switch_count())
    {
        return;
    }

    auto test = [&](uint8_t channel, uint8_t velocity)
    {
        // set known state
        for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type, i, switches::Type::Momentary));
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, i, switches::MessageType::Note));
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Value, i, velocity));
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Channel, i, channel));

            _digital._builderSwitches._instance.reset(i);
        }

        auto verify_value = [&](bool state)
        {
            // verify all received messages
            for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
            {
                if (state)
                {
                    ASSERT_EQ(midi::MessageType::NoteOn, _listener.event_log.at(i).message);
                }
                else
                {
                    ASSERT_EQ(midi::MessageType::NoteOff, _listener.event_log.at(i).message);
                }

                ASSERT_EQ(state ? velocity : 0, _listener.event_log.at(i).value);
                ASSERT_EQ(channel, _listener.event_log.at(i).channel);

                // also verify MIDI ID
                // it should be equal to switch ID by default
                ASSERT_EQ(i, _listener.event_log.at(i).index);
            }
        };

        // simulate switch press
        state_change_register_all(true);
        ASSERT_EQ(switches::Collection::size(switches::GroupDigitalInputs), _listener.event_log.size());
        verify_value(true);

        // simulate switch release
        state_change_register_all(false);
        ASSERT_EQ(switches::Collection::size(switches::GroupDigitalInputs), _listener.event_log.size());
        verify_value(false);

        // try with the latching mode
        for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type, i, switches::Type::Latching));
        }

        state_change_register_all(true);
        ASSERT_EQ(switches::Collection::size(switches::GroupDigitalInputs), _listener.event_log.size());
        verify_value(true);

        // nothing should happen on release
        state_change_register_all(false);
        ASSERT_EQ(0, _listener.event_log.size());

        // press again, new messages should arrive
        state_change_register_all(true);
        verify_value(false);
    };

    const std::array<uint8_t, 3> channels   = { 1, 8, 16 };
    const std::array<uint8_t, 3> velocities = { 1, 64, 127 };

    for (auto channel : channels)
    {
        for (auto velocity : velocities)
        {
            test(channel, velocity);
        }
    }
}

TEST_F(DigitalSwitchesTest, MidiTypeOverrideDoesNotChangeOscTypeBehavior)
{
    if (!DigitalSwitchesTest::digital_switch_count())
    {
        return;
    }

    static constexpr size_t SWITCH_INDEX = 0;

    ASSERT_EQ(switches::Type::Latching,
              switches::Mapper::message_to_type(switches::MessageType::MmcRecord, switches::Type::Momentary));
    ASSERT_EQ(switches::Type::Momentary,
              switches::Mapper::message_to_type(switches::MessageType::Note, switches::Type::Momentary));

    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type, SWITCH_INDEX, switches::Type::Momentary));
    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, SWITCH_INDEX, switches::MessageType::MmcRecord));
    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Channel, SWITCH_INDEX, 1));

    _digital._builderSwitches._instance.reset(SWITCH_INDEX);

    state_change_register_single(SWITCH_INDEX, true);
    ASSERT_EQ(1, _listener.event_log.size());
    ASSERT_EQ(midi::MessageType::MmcRecordStart, _listener.event_log.at(0).message);

    auto osc_events = _listener.osc_snapshot();
    ASSERT_EQ(1, osc_events.size());
    ASSERT_EQ(SWITCH_INDEX, osc_events.at(0).component_index);
    ASSERT_EQ(1, osc_events.at(0).value);

    state_change_register_single(SWITCH_INDEX, false);
    ASSERT_EQ(0, _listener.event_log.size());

    osc_events = _listener.osc_snapshot();
    ASSERT_EQ(1, osc_events.size());
    ASSERT_EQ(SWITCH_INDEX, osc_events.at(0).component_index);
    ASSERT_EQ(0, osc_events.at(0).value);

    state_change_register_single(SWITCH_INDEX, true);
    ASSERT_EQ(1, _listener.event_log.size());
    ASSERT_EQ(midi::MessageType::MmcRecordStop, _listener.event_log.at(0).message);

    osc_events = _listener.osc_snapshot();
    ASSERT_EQ(1, osc_events.size());
    ASSERT_EQ(SWITCH_INDEX, osc_events.at(0).component_index);
    ASSERT_EQ(1, osc_events.at(0).value);
}

TEST_F(DigitalSwitchesTest, ProgramChange)
{
    if (!DigitalSwitchesTest::digital_switch_count())
    {
        return;
    }

    auto test_program_change = [&](uint8_t channel)
    {
        // set known state
        for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type, i, switches::Type::Momentary));
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, i, switches::MessageType::ProgramChange));
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Channel, i, channel));

            _digital._builderSwitches._instance.reset(i);
        }

        auto verify_message = [&]()
        {
            // verify all received messages are program change
            for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
            {
                ASSERT_EQ(midi::MessageType::ProgramChange, _listener.event_log.at(i).message);

                // program change value should always be set to 0
                ASSERT_EQ(0, _listener.event_log.at(i).value);

                // verify channel
                ASSERT_EQ(channel, _listener.event_log.at(i).channel);

                // also verify MIDI ID/program
                // it should be equal to switch ID by default
                ASSERT_EQ(i, _listener.event_log.at(i).index);
            }
        };

        // simulate switch press
        state_change_register_all(true);
        ASSERT_EQ(switches::Collection::size(switches::GroupDigitalInputs), _listener.event_log.size());
        verify_message();

        // program change shouldn't be sent on release
        state_change_register_all(false);
        ASSERT_EQ(0, _listener.event_log.size());

        // repeat the entire test again, but with switches configured as latching types
        // behaviour should be the same
        for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type, i, switches::Type::Latching));
        }

        state_change_register_all(true);
        ASSERT_EQ(switches::Collection::size(switches::GroupDigitalInputs), _listener.event_log.size());
        verify_message();

        state_change_register_all(false);
        ASSERT_EQ(0, _listener.event_log.size());
    };

    for (auto channel : std::array<uint8_t, 3>{ 1, 8, 16 })
    {
        test_program_change(channel);
    }

    // test PROGRAM_CHANGE_INC/PROGRAM_CHANGE_DEC
    _database_admin.factory_reset();
    state_change_register_all(false);

    auto configure_pc_switch = [&](size_t index, uint8_t channel, bool increase)
    {
        ASSERT_LT(index, DigitalSwitchesTest::digital_switch_count());
        ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type, index, switches::Type::Momentary));
        ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, index, increase ? switches::MessageType::ProgramChangeInc : switches::MessageType::ProgramChangeDec));
        ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Channel, index, channel));

        for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
        {
            _digital._builderSwitches._instance.reset(i);
        }
    };

    auto verify_program_change = [&](size_t index, uint8_t channel, uint8_t program)
    {
        ASSERT_EQ(midi::MessageType::ProgramChange, _listener.event_log.at(index).message);

        // program change value should always be set to 0
        ASSERT_EQ(0, _listener.event_log.at(index).value);

        // verify channel
        ASSERT_EQ(channel, _listener.event_log.at(index).channel);

        ASSERT_EQ(program, _listener.event_log.at(index).index);
    };

    static constexpr uint8_t CHANNEL = 1;

    configure_pc_switch(0, CHANNEL, true);

    // verify that the received program change was 1 for switch 0
    state_change_register_all(true);
    verify_program_change(0, CHANNEL, 1);
    state_change_register_all(false);

    // after this, verify that the received program change was 2 for switch 0
    state_change_register_all(true);
    verify_program_change(0, CHANNEL, 2);
    state_change_register_all(false);

    // after this, verify that the received program change was 3 for switch 0
    state_change_register_all(true);
    verify_program_change(0, CHANNEL, 3);
    state_change_register_all(false);

    // now, revert all switches back to default
    _database_admin.factory_reset();

    if (DigitalSwitchesTest::digital_switch_count() < 2)
    {
        return;
    }

    const size_t second_program_change_inc_switch = DigitalSwitchesTest::digital_switch_count() - 1;

    // configure some other switch to PROGRAM_CHANGE_INC
    configure_pc_switch(second_program_change_inc_switch, CHANNEL, true);

    // verify that the program change is continuing to increase
    state_change_register_all(true);
    verify_program_change(second_program_change_inc_switch, CHANNEL, 4);
    state_change_register_all(false);

    state_change_register_all(true);
    verify_program_change(second_program_change_inc_switch, CHANNEL, 5);
    state_change_register_all(false);

    // now configure two switches to send program change/inc
    configure_pc_switch(0, CHANNEL, true);

    state_change_register_all(true);
    // program change should be increased by 1, first by switch 0
    verify_program_change(0, CHANNEL, 6);
    // then by the second configured increment switch
    verify_program_change(second_program_change_inc_switch, CHANNEL, 7);
    state_change_register_all(false);

    // configure another switch to PROGRAM_CHANGE_INC, but on other channel
    configure_pc_switch(1, 4, true);

    state_change_register_all(true);
    // program change should be increased by 1, first by switch 0
    verify_program_change(0, CHANNEL, 8);
    // then by the second configured increment switch
    verify_program_change(second_program_change_inc_switch, CHANNEL, 9);
    // program change should be sent on channel 4 by switch 1
    verify_program_change(1, 4, 1);
    state_change_register_all(false);

    // revert to default again
    _database_admin.factory_reset();

    // now configure switch 0 for PROGRAM_CHANGE_DEC
    configure_pc_switch(0, CHANNEL, false);

    state_change_register_all(true);
    // program change should decrease by 1
    verify_program_change(0, CHANNEL, 8);
    state_change_register_all(false);

    state_change_register_all(true);
    // program change should decrease by 1 again
    verify_program_change(0, CHANNEL, 7);
    state_change_register_all(false);

    // configure another switch for PROGRAM_CHANGE_DEC
    configure_pc_switch(1, CHANNEL, false);

    state_change_register_all(true);
    // switch 0 should decrease the value by 1
    verify_program_change(0, CHANNEL, 6);
    // switch 1 should decrease it again
    verify_program_change(1, CHANNEL, 5);
    state_change_register_all(false);

    // configure another switch for PROGRAM_CHANGE_DEC
    configure_pc_switch(2, CHANNEL, false);

    state_change_register_all(true);
    // switch 0 should decrease the value by 1
    verify_program_change(0, CHANNEL, 4);
    // switch 1 should decrease it again
    verify_program_change(1, CHANNEL, 3);
    // switch 2 should decrease it again
    verify_program_change(2, CHANNEL, 2);
    state_change_register_all(false);

    // reset all received messages first
    _listener.event_log.clear();

    // only two program change messages should be sent
    // program change value is 0 after the second switch decreases it
    // once the value is 0 no further messages should be sent in dec mode

    state_change_register_all(true);
    // switch 0 should decrease the value by 1
    verify_program_change(0, CHANNEL, 1);
    // switch 1 should decrease it again
    verify_program_change(1, CHANNEL, 0);

    // verify that only two program change messages have been received
    uint8_t pc_counter = 0;

    for (size_t i = 0; i < _listener.event_log.size(); i++)
    {
        if (_listener.event_log.at(i).message == midi::MessageType::ProgramChange)
        {
            pc_counter++;
        }
    }

    ASSERT_EQ(2, pc_counter);

    state_change_register_all(false);

    // revert all switches to default
    _database_admin.factory_reset();

    configure_pc_switch(0, CHANNEL, true);

    state_change_register_all(true);
    // switch 0 should increase the last value by 1
    verify_program_change(0, CHANNEL, 1);
    state_change_register_all(false);
}

TEST_F(DigitalSwitchesTest, ProgramChangeWithOffset)
{
    static constexpr size_t REQUIRED_SWITCH_COUNT_FOR_PROGRAM_CHANGE_OFFSET = 3;

    if (DigitalSwitchesTest::digital_switch_count() < REQUIRED_SWITCH_COUNT_FOR_PROGRAM_CHANGE_OFFSET)
    {
        return;
    }

    static constexpr size_t SWITCH_INDEX_OFFSET_DEC = 0;
    static constexpr size_t SWITCH_INDEX_OFFSET_INC = 1;
    static constexpr size_t SWITCH_INDEX_PROGRAM    = 2;
    static constexpr size_t PROGRAM_CHANGE_CHANNEL  = 1;
    static constexpr size_t OFFSET_INC_DEC_AMOUNT   = 3;

    auto verify_program_change = [&](size_t index, uint8_t channel, uint8_t program)
    {
        ASSERT_EQ(midi::MessageType::ProgramChange, _listener.event_log.at(index).message);

        // program change value should always be set to 0
        ASSERT_EQ(0, _listener.event_log.at(index).value);

        // verify channel
        ASSERT_EQ(channel, _listener.event_log.at(index).channel);

        ASSERT_EQ(program, _listener.event_log.at(index).index);
    };

    auto toggle_program_change_inc_switch = [&]()
    {
        state_change_register_single(SWITCH_INDEX_OFFSET_INC, true);
        ASSERT_TRUE(MidiProgram.offset() <= 127);
        state_change_register_single(SWITCH_INDEX_OFFSET_INC, false);
    };

    auto toggle_program_change_dec_switch = [&]()
    {
        state_change_register_single(SWITCH_INDEX_OFFSET_DEC, true);
        ASSERT_TRUE(MidiProgram.offset() <= 127);
        state_change_register_single(SWITCH_INDEX_OFFSET_DEC, false);
    };

    auto toggle_program_change_switch = [&]()
    {
        state_change_register_single(SWITCH_INDEX_PROGRAM, true);
        auto                   events          = _listener.snapshot();
        std::vector<TestEvent> program_changes = {};

        for (const auto& event : events)
        {
            if (event.message == midi::MessageType::ProgramChange)
            {
                program_changes.push_back(event);
            }
        }

        ASSERT_EQ(1, program_changes.size());
        ASSERT_EQ(midi::MessageType::ProgramChange, program_changes.at(0).message);
        ASSERT_EQ(0, program_changes.at(0).value);
        ASSERT_EQ(PROGRAM_CHANGE_CHANNEL, program_changes.at(0).channel);
        ASSERT_EQ(SWITCH_INDEX_PROGRAM + MidiProgram.offset(), program_changes.at(0).index);
        state_change_register_single(SWITCH_INDEX_PROGRAM, false);
    };

    // set known state
    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, SWITCH_INDEX_OFFSET_DEC, switches::MessageType::ProgramChangeOffsetDec));
    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Value, SWITCH_INDEX_OFFSET_DEC, OFFSET_INC_DEC_AMOUNT));
    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, SWITCH_INDEX_OFFSET_INC, switches::MessageType::ProgramChangeOffsetInc));
    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Value, SWITCH_INDEX_OFFSET_INC, OFFSET_INC_DEC_AMOUNT));
    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, SWITCH_INDEX_PROGRAM, switches::MessageType::ProgramChange));

    _digital._builderSwitches._instance.reset(SWITCH_INDEX_OFFSET_DEC);
    _digital._builderSwitches._instance.reset(SWITCH_INDEX_OFFSET_INC);
    _digital._builderSwitches._instance.reset(SWITCH_INDEX_PROGRAM);

    toggle_program_change_switch();
    toggle_program_change_inc_switch();
    toggle_program_change_switch();

    // toggle the incrementing switch few more times
    for (size_t i = 0; i < 10; i++)
    {
        toggle_program_change_inc_switch();
    }

    toggle_program_change_switch();

    // verify that incrementing past the value of 127 isn't possible
    while (MidiProgram.offset() != 127)
    {
        toggle_program_change_inc_switch();
    }

    // try again - nothing should change
    toggle_program_change_inc_switch();
    ASSERT_EQ(127, MidiProgram.offset());

    // start decrementing offset
    for (size_t i = 0; i < 50; i++)
    {
        toggle_program_change_dec_switch();
    }

    toggle_program_change_switch();

    MidiProgram.set_offset(0);
    ASSERT_EQ(0, MidiProgram.offset());

    toggle_program_change_switch();

    // decrement again - verify that nothing changes
    toggle_program_change_dec_switch();
    ASSERT_EQ(0, MidiProgram.offset());
}

TEST_F(DigitalSwitchesTest, ControlChange)
{
    if (!DigitalSwitchesTest::digital_switch_count())
    {
        return;
    }

    auto control_change_test = [&](uint8_t control_value)
    {
        // set known state
        for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type, i, switches::Type::Momentary));
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, i, switches::MessageType::ControlChange));
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Value, i, control_value));

            _digital._builderSwitches._instance.reset(i);
        }

        auto verify_message = [&](uint8_t midi_value)
        {
            // verify all received messages are control change
            for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
            {
                ASSERT_EQ(midi::MessageType::ControlChange, _listener.event_log.at(i).message);
                ASSERT_EQ(midi_value, _listener.event_log.at(i).value);
                ASSERT_EQ(1, _listener.event_log.at(i).channel);
                ASSERT_EQ(i, _listener.event_log.at(i).index);
            }
        };

        // simulate switch press
        state_change_register_all(true);
        ASSERT_EQ(switches::Collection::size(switches::GroupDigitalInputs), _listener.event_log.size());

        verify_message(control_value);

        // no messages should be sent on release
        state_change_register_all(false);
        ASSERT_EQ(0, _listener.event_log.size());

        // change to latching type
        // behaviour should be the same

        for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type, i, switches::Type::Latching));
        }

        state_change_register_all(true);
        ASSERT_EQ(switches::Collection::size(switches::GroupDigitalInputs), _listener.event_log.size());

        verify_message(control_value);

        state_change_register_all(false);
        ASSERT_EQ(0, _listener.event_log.size());

        // change type to control change with 0 on reset, momentary mode
        // this means CC value 0 should be sent on release

        for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type, i, switches::Type::Momentary));
        }

        for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, i, switches::MessageType::ControlChangeReset));
        }

        state_change_register_all(true);
        ASSERT_EQ(switches::Collection::size(switches::GroupDigitalInputs), _listener.event_log.size());

        verify_message(control_value);

        state_change_register_all(false);
        ASSERT_EQ(switches::Collection::size(switches::GroupDigitalInputs), _listener.event_log.size());

        verify_message(0);

        // same test again, but in latching mode
        // now, on press, messages should be sent
        // on release, nothing should happen
        // on second press reset should be sent (CC with value 0)

        for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type, i, switches::Type::Latching));
        }

        state_change_register_all(true);
        ASSERT_EQ(switches::Collection::size(switches::GroupDigitalInputs), _listener.event_log.size());

        verify_message(control_value);

        state_change_register_all(false);
        ASSERT_EQ(0, _listener.event_log.size());

        state_change_register_all(true);
        ASSERT_EQ(switches::Collection::size(switches::GroupDigitalInputs), _listener.event_log.size());

        verify_message(0);
    };

    // verify representative control values across the valid range
    for (auto value : std::array<uint8_t, 3>{ 1, 64, 127 })
    {
        control_change_test(value);
    }
}

TEST_F(DigitalSwitchesTest, OscFollowsOnlyLogicalSwitchState)
{
    if (!DigitalSwitchesTest::digital_switch_count())
    {
        return;
    }

    static constexpr size_t SWITCH_INDEX = 0;

    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType,
                                                           SWITCH_INDEX,
                                                           switches::MessageType::None));
    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type,
                                                           SWITCH_INDEX,
                                                           switches::Type::Momentary));
    _digital._builderSwitches._instance.reset(SWITCH_INDEX);

    state_change_register_single(SWITCH_INDEX, true);
    auto osc_events = _listener.osc_snapshot();
    ASSERT_EQ(1U, osc_events.size());
    EXPECT_EQ(SWITCH_INDEX, osc_events.at(0).component_index);
    EXPECT_EQ(1U, osc_events.at(0).value);
    EXPECT_EQ(0U, _listener.event_log.size());

    state_change_register_single(SWITCH_INDEX, false);
    osc_events = _listener.osc_snapshot();
    ASSERT_EQ(1U, osc_events.size());
    EXPECT_EQ(SWITCH_INDEX, osc_events.at(0).component_index);
    EXPECT_EQ(0U, osc_events.at(0).value);
    EXPECT_EQ(0U, _listener.event_log.size());

    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type,
                                                           SWITCH_INDEX,
                                                           switches::Type::Latching));
    _digital._builderSwitches._instance.reset(SWITCH_INDEX);

    state_change_register_single(SWITCH_INDEX, true);
    osc_events = _listener.osc_snapshot();
    ASSERT_EQ(1U, osc_events.size());
    EXPECT_EQ(SWITCH_INDEX, osc_events.at(0).component_index);
    EXPECT_EQ(1U, osc_events.at(0).value);

    state_change_register_single(SWITCH_INDEX, false);
    osc_events = _listener.osc_snapshot();
    ASSERT_EQ(0U, osc_events.size());

    state_change_register_single(SWITCH_INDEX, true);
    osc_events = _listener.osc_snapshot();
    ASSERT_EQ(1U, osc_events.size());
    EXPECT_EQ(SWITCH_INDEX, osc_events.at(0).component_index);
    EXPECT_EQ(0U, osc_events.at(0).value);
}

TEST_F(DigitalSwitchesTest, NoMessages)
{
    if (!switches::Collection::size(switches::GroupDigitalInputs))
    {
        return;
    }

    // configure all switches to MessageType::None so that messages aren't sent

    for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
    {
        ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, i, switches::MessageType::None));

        _digital._builderSwitches._instance.reset(i);
    }

    state_change_register_all(true);
    ASSERT_EQ(0, _listener.event_log.size());

    state_change_register_all(false);
    ASSERT_EQ(0, _listener.event_log.size());

    for (size_t i = 0; i < switches::Collection::size(switches::GroupDigitalInputs); i++)
    {
        ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::Type, i, switches::Type::Latching));
    }

    state_change_register_all(true);
    ASSERT_EQ(0, _listener.event_log.size());

    state_change_register_all(false);
    ASSERT_EQ(0, _listener.event_log.size());

    state_change_register_all(true);
    ASSERT_EQ(0, _listener.event_log.size());

    state_change_register_all(false);
    ASSERT_EQ(0, _listener.event_log.size());
}

TEST_F(DigitalSwitchesTest, PresetChange)
{
    if (_database_admin.supported_presets() <= 1)
    {
        return;
    }

    if (!DigitalSwitchesTest::digital_switch_count())
    {
        return;
    }

    // configure one switch to change preset
    static constexpr size_t SWITCH_INDEX = 0;

    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MidiId, SWITCH_INDEX, 1));
    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, SWITCH_INDEX, switches::MessageType::PresetChange));
    _digital._builderSwitches._instance.reset(SWITCH_INDEX);

    // simulate switch press
    state_change_register_single(SWITCH_INDEX, true);

    ASSERT_EQ(1, _listener.event_log.size());
    ASSERT_EQ(signaling::SystemEvent::PresetChangeDirectReq, _listener.event_log.at(0).system_event);

    // verify that no new events are generated on switch release
    state_change_register_single(SWITCH_INDEX, false);
    ASSERT_EQ(0, _listener.event_log.size());
}

TEST_F(DigitalSwitchesTest, MMCStartStop)
{
    if (!DigitalSwitchesTest::digital_switch_count())
    {
        return;
    }

    // configure one switch to MMC_PLAY_STOP message type
    static constexpr size_t SWITCH_INDEX = 0;

    ASSERT_TRUE(_digital._builderSwitches._database.update(database::Config::Section::Switch::MessageType, SWITCH_INDEX, switches::MessageType::MmcPlayStop));
    _digital._builderSwitches._instance.reset(SWITCH_INDEX);

    // simulate switch press
    state_change_register_single(SWITCH_INDEX, true);

    ASSERT_EQ(1, _listener.event_log.size());
    ASSERT_EQ(midi::MessageType::MmcPlay, _listener.event_log.at(0).message);

    // verify that no new events are generated on switch release
    state_change_register_single(SWITCH_INDEX, false);
    ASSERT_EQ(0, _listener.event_log.size());

    // simulate a new switch press
    state_change_register_single(SWITCH_INDEX, true);

    // the message should be MMC_STOP now
    ASSERT_EQ(1, _listener.event_log.size());
    ASSERT_EQ(midi::MessageType::MmcStop, _listener.event_log.at(0).message);
}
#else
TEST(DigitalSwitchesTest, SkippedWhenPresetDoesNotSupportSwitches)
{
    SUCCEED();
}
#endif
