/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/database.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BUTTONS

#include "io/digital/builder.h"
#include "util/configurable/configurable.h"
#include "zlibs/utils/misc/ring_buffer.h"
#include "zlibs/utils/misc/mutex.h"

#include <limits>

using namespace io;
using namespace protocol;

namespace
{
    struct TestEvent
    {
        size_t                   component_index = 0;
        uint8_t                  channel         = 0;
        uint16_t                 index           = 0;
        uint16_t                 value           = 0;
        midi::MessageType        message         = midi::MessageType::Invalid;
        messaging::SystemMessage system_message  = {};
    };

    class Listener
    {
        public:
        void push(const messaging::MidiSignal& signal)
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

        void push(const messaging::SystemSignal& signal)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);

            event_log.push_back(TestEvent{
                .component_index = 0,
                .channel         = 0,
                .index           = 0,
                .value           = signal.value,
                .message         = midi::MessageType::Invalid,
                .system_message  = signal.system_message,
            });
        }

        void clear()
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            event_log.clear();
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

        private:
        mutable zlibs::utils::misc::Mutex _mutex;

        public:
        std::vector<TestEvent> event_log = {};
    };

    class DigitalButtonsTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            tests::resume_io();

            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_database_admin.factory_reset());
            ASSERT_EQ(0, _database_admin.current_preset());

            for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
            {
                ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Type, i, buttons::Type::Momentary));
                ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MessageType, i, buttons::MessageType::Note));
                ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Value, i, 127));

                _digital._builderButtons._instance.reset(i);
            }

            messaging::subscribe<messaging::MidiSignal>(
                [this](const messaging::MidiSignal& signal)
                {
                    if (signal.source == messaging::MidiSource::Button)
                    {
                        _listener.push(signal);
                    }
                });

            messaging::subscribe<messaging::SystemSignal>(
                [this](const messaging::SystemSignal& signal)
                {
                    _listener.push(signal);
                });

            k_msleep(20);
            _listener.clear();
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            messaging::clear_registry();
            _listener.clear();
        }

        void state_change_register_all(bool state)
        {
            _listener.clear();

            EXPECT_CALL(_digital._builderButtons._hwa, state(_))
                .WillRepeatedly(Return(std::optional<bool>(state)));

            EXPECT_CALL(_digital._builderEncoders._hwa, state(_))
                .WillRepeatedly(Return(std::optional<uint8_t>(0b00)));

            _digital._instance.process_state_changes();
            wait_for_signals();
        }

        void state_change_register_single(size_t index, bool state)
        {
            ASSERT_LT(index, buttons::Collection::size(buttons::GroupDigitalInputs));

            _listener.clear();

            // Only the target button transitions; all others stay released.
            // DigitalTest::process_state_changes calls _buttons.process_state_changes(), so every
            // button HWA is queried - we must avoid activating unrelated buttons.
            EXPECT_CALL(_digital._builderButtons._hwa, state(Ne(index)))
                .WillRepeatedly(Return(std::optional<bool>(false)));
            EXPECT_CALL(_digital._builderButtons._hwa, state(Eq(index)))
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

        static size_t digital_button_count()
        {
            return buttons::Collection::size(buttons::GroupDigitalInputs);
        }

        tests::NoOpDatabaseHandlers _handlers;
        Listener                    _listener;
        database::Builder           _builder_database;
        database::Admin&            _database_admin = _builder_database.instance();
        io::digital::Builder        _digital        = io::digital::Builder(_database_admin);
    };
}    // namespace

TEST(ButtonsBufferTest, PreservesOrderAndDrains)
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

TEST(ButtonsBufferTest, OverwritesOldestUnreadSampleWhenFull)
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

TEST_F(DigitalButtonsTest, Note)
{
    if (!DigitalButtonsTest::digital_button_count())
    {
        return;
    }

    auto test = [&](uint8_t channel, uint8_t velocity)
    {
        // set known state
        for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Type, i, buttons::Type::Momentary));
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MessageType, i, buttons::MessageType::Note));
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Value, i, velocity));
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Channel, i, channel));

            _digital._builderButtons._instance.reset(i);
        }

        auto verify_value = [&](bool state)
        {
            // verify all received messages
            for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
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
                // it should be equal to button ID by default
                ASSERT_EQ(i, _listener.event_log.at(i).index);
            }
        };

        // simulate button press
        state_change_register_all(true);
        ASSERT_EQ(buttons::Collection::size(buttons::GroupDigitalInputs), _listener.event_log.size());
        verify_value(true);

        // simulate button release
        state_change_register_all(false);
        ASSERT_EQ(buttons::Collection::size(buttons::GroupDigitalInputs), _listener.event_log.size());
        verify_value(false);

        // try with the latching mode
        for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Type, i, buttons::Type::Latching));
        }

        state_change_register_all(true);
        ASSERT_EQ(buttons::Collection::size(buttons::GroupDigitalInputs), _listener.event_log.size());
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

TEST_F(DigitalButtonsTest, ProgramChange)
{
    if (!DigitalButtonsTest::digital_button_count())
    {
        return;
    }

    auto test_program_change = [&](uint8_t channel)
    {
        // set known state
        for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Type, i, buttons::Type::Momentary));
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MessageType, i, buttons::MessageType::ProgramChange));
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Channel, i, channel));

            _digital._builderButtons._instance.reset(i);
        }

        auto verify_message = [&]()
        {
            // verify all received messages are program change
            for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
            {
                ASSERT_EQ(midi::MessageType::ProgramChange, _listener.event_log.at(i).message);

                // program change value should always be set to 0
                ASSERT_EQ(0, _listener.event_log.at(i).value);

                // verify channel
                ASSERT_EQ(channel, _listener.event_log.at(i).channel);

                // also verify MIDI ID/program
                // it should be equal to button ID by default
                ASSERT_EQ(i, _listener.event_log.at(i).index);
            }
        };

        // simulate button press
        state_change_register_all(true);
        ASSERT_EQ(buttons::Collection::size(buttons::GroupDigitalInputs), _listener.event_log.size());
        verify_message();

        // program change shouldn't be sent on release
        state_change_register_all(false);
        ASSERT_EQ(0, _listener.event_log.size());

        // repeat the entire test again, but with buttons configured as latching types
        // behaviour should be the same
        for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Type, i, buttons::Type::Latching));
        }

        state_change_register_all(true);
        ASSERT_EQ(buttons::Collection::size(buttons::GroupDigitalInputs), _listener.event_log.size());
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

    auto configure_pc_button = [&](size_t index, uint8_t channel, bool increase)
    {
        ASSERT_LT(index, DigitalButtonsTest::digital_button_count());
        ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Type, index, buttons::Type::Momentary));
        ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MessageType, index, increase ? buttons::MessageType::ProgramChangeInc : buttons::MessageType::ProgramChangeDec));
        ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Channel, index, channel));

        for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
        {
            _digital._builderButtons._instance.reset(i);
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

    configure_pc_button(0, CHANNEL, true);

    // verify that the received program change was 1 for button 0
    state_change_register_all(true);
    verify_program_change(0, CHANNEL, 1);
    state_change_register_all(false);

    // after this, verify that the received program change was 2 for button 0
    state_change_register_all(true);
    verify_program_change(0, CHANNEL, 2);
    state_change_register_all(false);

    // after this, verify that the received program change was 3 for button 0
    state_change_register_all(true);
    verify_program_change(0, CHANNEL, 3);
    state_change_register_all(false);

    // now, revert all buttons back to default
    _database_admin.factory_reset();

    if (DigitalButtonsTest::digital_button_count() < 2)
    {
        return;
    }

    const size_t second_program_change_inc_button = DigitalButtonsTest::digital_button_count() - 1;

    // configure some other button to PROGRAM_CHANGE_INC
    configure_pc_button(second_program_change_inc_button, CHANNEL, true);

    // verify that the program change is continuing to increase
    state_change_register_all(true);
    verify_program_change(second_program_change_inc_button, CHANNEL, 4);
    state_change_register_all(false);

    state_change_register_all(true);
    verify_program_change(second_program_change_inc_button, CHANNEL, 5);
    state_change_register_all(false);

    // now configure two buttons to send program change/inc
    configure_pc_button(0, CHANNEL, true);

    state_change_register_all(true);
    // program change should be increased by 1, first by button 0
    verify_program_change(0, CHANNEL, 6);
    // then by the second configured increment button
    verify_program_change(second_program_change_inc_button, CHANNEL, 7);
    state_change_register_all(false);

    // configure another button to PROGRAM_CHANGE_INC, but on other channel
    configure_pc_button(1, 4, true);

    state_change_register_all(true);
    // program change should be increased by 1, first by button 0
    verify_program_change(0, CHANNEL, 8);
    // then by the second configured increment button
    verify_program_change(second_program_change_inc_button, CHANNEL, 9);
    // program change should be sent on channel 4 by button 1
    verify_program_change(1, 4, 1);
    state_change_register_all(false);

    // revert to default again
    _database_admin.factory_reset();

    // now configure button 0 for PROGRAM_CHANGE_DEC
    configure_pc_button(0, CHANNEL, false);

    state_change_register_all(true);
    // program change should decrease by 1
    verify_program_change(0, CHANNEL, 8);
    state_change_register_all(false);

    state_change_register_all(true);
    // program change should decrease by 1 again
    verify_program_change(0, CHANNEL, 7);
    state_change_register_all(false);

    // configure another button for PROGRAM_CHANGE_DEC
    configure_pc_button(1, CHANNEL, false);

    state_change_register_all(true);
    // button 0 should decrease the value by 1
    verify_program_change(0, CHANNEL, 6);
    // button 1 should decrease it again
    verify_program_change(1, CHANNEL, 5);
    state_change_register_all(false);

    // configure another button for PROGRAM_CHANGE_DEC
    configure_pc_button(2, CHANNEL, false);

    state_change_register_all(true);
    // button 0 should decrease the value by 1
    verify_program_change(0, CHANNEL, 4);
    // button 1 should decrease it again
    verify_program_change(1, CHANNEL, 3);
    // button 2 should decrease it again
    verify_program_change(2, CHANNEL, 2);
    state_change_register_all(false);

    // reset all received messages first
    _listener.event_log.clear();

    // only two program change messages should be sent
    // program change value is 0 after the second button decreases it
    // once the value is 0 no further messages should be sent in dec mode

    state_change_register_all(true);
    // button 0 should decrease the value by 1
    verify_program_change(0, CHANNEL, 1);
    // button 1 should decrease it again
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

    // revert all buttons to default
    _database_admin.factory_reset();

    configure_pc_button(0, CHANNEL, true);

    state_change_register_all(true);
    // button 0 should increase the last value by 1
    verify_program_change(0, CHANNEL, 1);
    state_change_register_all(false);
}

TEST_F(DigitalButtonsTest, ProgramChangeWithOffset)
{
    static constexpr size_t REQUIRED_BUTTON_COUNT_FOR_PROGRAM_CHANGE_OFFSET = 3;

    if (DigitalButtonsTest::digital_button_count() < REQUIRED_BUTTON_COUNT_FOR_PROGRAM_CHANGE_OFFSET)
    {
        return;
    }

    static constexpr size_t BUTTON_INDEX_OFFSET_DEC = 0;
    static constexpr size_t BUTTON_INDEX_OFFSET_INC = 1;
    static constexpr size_t BUTTON_INDEX_PROGRAM    = 2;
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

    auto toggle_program_change_inc_button = [&]()
    {
        state_change_register_single(BUTTON_INDEX_OFFSET_INC, true);
        ASSERT_TRUE(MidiProgram.offset() <= 127);
        state_change_register_single(BUTTON_INDEX_OFFSET_INC, false);
    };

    auto toggle_program_change_dec_button = [&]()
    {
        state_change_register_single(BUTTON_INDEX_OFFSET_DEC, true);
        ASSERT_TRUE(MidiProgram.offset() <= 127);
        state_change_register_single(BUTTON_INDEX_OFFSET_DEC, false);
    };

    auto toggle_program_change_button = [&]()
    {
        state_change_register_single(BUTTON_INDEX_PROGRAM, true);
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
        ASSERT_EQ(BUTTON_INDEX_PROGRAM + MidiProgram.offset(), program_changes.at(0).index);
        state_change_register_single(BUTTON_INDEX_PROGRAM, false);
    };

    // set known state
    ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MessageType, BUTTON_INDEX_OFFSET_DEC, buttons::MessageType::ProgramChangeOffsetDec));
    ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Value, BUTTON_INDEX_OFFSET_DEC, OFFSET_INC_DEC_AMOUNT));
    ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MessageType, BUTTON_INDEX_OFFSET_INC, buttons::MessageType::ProgramChangeOffsetInc));
    ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Value, BUTTON_INDEX_OFFSET_INC, OFFSET_INC_DEC_AMOUNT));
    ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MessageType, BUTTON_INDEX_PROGRAM, buttons::MessageType::ProgramChange));

    _digital._builderButtons._instance.reset(BUTTON_INDEX_OFFSET_DEC);
    _digital._builderButtons._instance.reset(BUTTON_INDEX_OFFSET_INC);
    _digital._builderButtons._instance.reset(BUTTON_INDEX_PROGRAM);

    toggle_program_change_button();
    toggle_program_change_inc_button();
    toggle_program_change_button();

    // toggle the incrementing button few more times
    for (size_t i = 0; i < 10; i++)
    {
        toggle_program_change_inc_button();
    }

    toggle_program_change_button();

    // verify that incrementing past the value of 127 isn't possible
    while (MidiProgram.offset() != 127)
    {
        toggle_program_change_inc_button();
    }

    // try again - nothing should change
    toggle_program_change_inc_button();
    ASSERT_EQ(127, MidiProgram.offset());

    // start decrementing offset
    for (size_t i = 0; i < 50; i++)
    {
        toggle_program_change_dec_button();
    }

    toggle_program_change_button();

    MidiProgram.set_offset(0);
    ASSERT_EQ(0, MidiProgram.offset());

    toggle_program_change_button();

    // decrement again - verify that nothing changes
    toggle_program_change_dec_button();
    ASSERT_EQ(0, MidiProgram.offset());
}

TEST_F(DigitalButtonsTest, ControlChange)
{
    if (!DigitalButtonsTest::digital_button_count())
    {
        return;
    }

    auto control_change_test = [&](uint8_t control_value)
    {
        // set known state
        for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Type, i, buttons::Type::Momentary));
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MessageType, i, buttons::MessageType::ControlChange));
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Value, i, control_value));

            _digital._builderButtons._instance.reset(i);
        }

        auto verify_message = [&](uint8_t midi_value)
        {
            // verify all received messages are control change
            for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
            {
                ASSERT_EQ(midi::MessageType::ControlChange, _listener.event_log.at(i).message);
                ASSERT_EQ(midi_value, _listener.event_log.at(i).value);
                ASSERT_EQ(1, _listener.event_log.at(i).channel);
                ASSERT_EQ(i, _listener.event_log.at(i).index);
            }
        };

        // simulate button press
        state_change_register_all(true);
        ASSERT_EQ(buttons::Collection::size(buttons::GroupDigitalInputs), _listener.event_log.size());

        verify_message(control_value);

        // no messages should be sent on release
        state_change_register_all(false);
        ASSERT_EQ(0, _listener.event_log.size());

        // change to latching type
        // behaviour should be the same

        for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Type, i, buttons::Type::Latching));
        }

        state_change_register_all(true);
        ASSERT_EQ(buttons::Collection::size(buttons::GroupDigitalInputs), _listener.event_log.size());

        verify_message(control_value);

        state_change_register_all(false);
        ASSERT_EQ(0, _listener.event_log.size());

        // change type to control change with 0 on reset, momentary mode
        // this means CC value 0 should be sent on release

        for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Type, i, buttons::Type::Momentary));
        }

        for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MessageType, i, buttons::MessageType::ControlChangeReset));
        }

        state_change_register_all(true);
        ASSERT_EQ(buttons::Collection::size(buttons::GroupDigitalInputs), _listener.event_log.size());

        verify_message(control_value);

        state_change_register_all(false);
        ASSERT_EQ(buttons::Collection::size(buttons::GroupDigitalInputs), _listener.event_log.size());

        verify_message(0);

        // same test again, but in latching mode
        // now, on press, messages should be sent
        // on release, nothing should happen
        // on second press reset should be sent (CC with value 0)

        for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
        {
            ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Type, i, buttons::Type::Latching));
        }

        state_change_register_all(true);
        ASSERT_EQ(buttons::Collection::size(buttons::GroupDigitalInputs), _listener.event_log.size());

        verify_message(control_value);

        state_change_register_all(false);
        ASSERT_EQ(0, _listener.event_log.size());

        state_change_register_all(true);
        ASSERT_EQ(buttons::Collection::size(buttons::GroupDigitalInputs), _listener.event_log.size());

        verify_message(0);
    };

    // verify representative control values across the valid range
    for (auto value : std::array<uint8_t, 3>{ 1, 64, 127 })
    {
        control_change_test(value);
    }
}

TEST_F(DigitalButtonsTest, NoMessages)
{
    if (!buttons::Collection::size(buttons::GroupDigitalInputs))
    {
        return;
    }

    // configure all buttons to MessageType::None so that messages aren't sent

    for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
    {
        ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MessageType, i, buttons::MessageType::None));

        _digital._builderButtons._instance.reset(i);
    }

    state_change_register_all(true);
    ASSERT_EQ(0, _listener.event_log.size());

    state_change_register_all(false);
    ASSERT_EQ(0, _listener.event_log.size());

    for (size_t i = 0; i < buttons::Collection::size(buttons::GroupDigitalInputs); i++)
    {
        ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::Type, i, buttons::Type::Latching));
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

TEST_F(DigitalButtonsTest, PresetChange)
{
    if (_database_admin.supported_presets() <= 1)
    {
        return;
    }

    if (!DigitalButtonsTest::digital_button_count())
    {
        return;
    }

    // configure one button to change preset
    static constexpr size_t BUTTON_INDEX = 0;

    ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MidiId, BUTTON_INDEX, 1));
    ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MessageType, BUTTON_INDEX, buttons::MessageType::PresetChange));
    _digital._builderButtons._instance.reset(BUTTON_INDEX);

    // simulate button press
    state_change_register_single(BUTTON_INDEX, true);

    ASSERT_EQ(1, _listener.event_log.size());
    ASSERT_EQ(messaging::SystemMessage::PresetChangeDirectReq, _listener.event_log.at(0).system_message);

    // verify that no new events are generated on button release
    state_change_register_single(BUTTON_INDEX, false);
    ASSERT_EQ(0, _listener.event_log.size());
}

TEST_F(DigitalButtonsTest, MMCStartStop)
{
    if (!DigitalButtonsTest::digital_button_count())
    {
        return;
    }

    // configure one button to MMC_PLAY_STOP message type
    static constexpr size_t BUTTON_INDEX = 0;

    ASSERT_TRUE(_digital._builderButtons._database.update(database::Config::Section::Button::MessageType, BUTTON_INDEX, buttons::MessageType::MmcPlayStop));
    _digital._builderButtons._instance.reset(BUTTON_INDEX);

    // simulate button press
    state_change_register_single(BUTTON_INDEX, true);

    ASSERT_EQ(1, _listener.event_log.size());
    ASSERT_EQ(midi::MessageType::MmcPlay, _listener.event_log.at(0).message);

    // verify that no new events are generated on button release
    state_change_register_single(BUTTON_INDEX, false);
    ASSERT_EQ(0, _listener.event_log.size());

    // simulate a new button press
    state_change_register_single(BUTTON_INDEX, true);

    // the message should be MMC_STOP now
    ASSERT_EQ(1, _listener.event_log.size());
    ASSERT_EQ(midi::MessageType::MmcStop, _listener.event_log.at(0).message);
}
#else
TEST(DigitalButtonsTest, SkippedWhenPresetDoesNotSupportButtons)
{
    SUCCEED();
}
#endif
