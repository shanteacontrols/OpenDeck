/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/database.h"
#include "tests/helpers/misc.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS

#include "io/digital/builder.h"
#include "util/configurable/configurable.h"
#include "zlibs/utils/misc/mutex.h"

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
                .system_event = signal.system_event,
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

        private:
        mutable zlibs::utils::misc::Mutex _mutex;

        public:
        std::vector<TestEvent> event_log = {};
    };

    class DigitalEncodersTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            tests::resume_io();

            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_database_admin.factory_reset());
            ASSERT_EQ(0, _database_admin.current_preset());

            // set known state
            for (size_t i = 0; i < encoders::Collection::size(); i++)
            {
                ASSERT_TRUE(_digital._builderEncoders._database.update(database::Config::Section::Encoder::Enable, i, 1));
                ASSERT_TRUE(_digital._builderEncoders._database.update(database::Config::Section::Encoder::Invert, i, 0));
                ASSERT_TRUE(_digital._builderEncoders._database.update(database::Config::Section::Encoder::Mode, i, encoders::Type::ControlChange7fh01h));
            }

            signaling::subscribe<signaling::MidiIoSignal>(
                [this](const signaling::MidiIoSignal& signal)
                {
                    if (signal.source == signaling::IoEventSource::Encoder)
                    {
                        _listener.push(signal);
                    }
                });

            signaling::subscribe<signaling::SystemSignal>(
                [this](const signaling::SystemSignal& signal)
                {
                    _listener.push(signal);
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

        void state_change_register(uint8_t state)
        {
            _listener.clear();

            EXPECT_CALL(_digital._builderEncoders._hwa, state(_))
                .WillRepeatedly(Return(std::optional<uint8_t>(state)));

            // keep switches at released state to avoid spurious switch signals
            EXPECT_CALL(_digital._builderSwitches._hwa, state(_))
                .WillRepeatedly(Return(std::optional<bool>(false)));

            _digital._instance.process_state_changes();
            wait_for_signals();
        }

        void wait_for_signals()
        {
            size_t stable_iterations = 0;
            size_t last_size         = static_cast<size_t>(-1);
            size_t max_iterations    = 0;

            while ((stable_iterations < 2) && (max_iterations < 8))
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

        static constexpr std::array<uint8_t, 4> ENCODER_STATE = {
            0b00,
            0b10,
            0b11,
            0b01
        };

        tests::NoOpDatabaseHandlers _handlers;
        Listener                    _listener;
        database::Builder           _builder_database;
        database::Admin&            _database_admin = _builder_database.instance();
        io::digital::Builder        _digital        = io::digital::Builder(_database_admin);
    };
}    // namespace

TEST_F(DigitalEncodersTest, StateDecoding)
{
    if (!encoders::Collection::size())
    {
        return;
    }

    auto verify_value = [&](midi::MessageType message, uint16_t value)
    {
        for (size_t i = 0; i < encoders::Collection::size(); i++)
        {
            ASSERT_EQ(message, _listener.event_log.at(i).message);
            ASSERT_EQ(value, _listener.event_log.at(i).value);
        }
    };

    auto setup = [&](encoders::Type type)
    {
        for (size_t i = 0; i < encoders::Collection::size(); i++)
        {
            ASSERT_TRUE(_digital._builderEncoders._database.update(database::Config::Section::Encoder::Mode, i, type));
            _digital._builderEncoders._instance.reset(i);
            state_change_register(0b00);
        }
    };

    auto rotate = [this](bool clockwise)
    {
        size_t state_index = 0;

        auto next_value = [&]()
        {
            if (clockwise)
            {
                state_index++;
            }
            else
            {
                state_index--;
            }

            state_index %= ENCODER_STATE.size();
            return ENCODER_STATE.at(state_index);
        };

        for (size_t pulse = 0; pulse < encoders::Filter::PULSES_PER_STEP; pulse++)
        {
            state_change_register(next_value());
        }
    };

    setup(encoders::Type::ControlChange7fh01h);
    rotate(true);
    ASSERT_EQ(encoders::Collection::size(), _listener.event_log.size());
    verify_value(midi::MessageType::ControlChange, 1);

    rotate(true);
    ASSERT_EQ(encoders::Collection::size(), _listener.event_log.size());
    verify_value(midi::MessageType::ControlChange, 1);

    rotate(false);
    ASSERT_EQ(encoders::Collection::size(), _listener.event_log.size());
    verify_value(midi::MessageType::ControlChange, 127);
}

TEST_F(DigitalEncodersTest, Messages)
{
    if (!encoders::Collection::size())
    {
        return;
    }

    auto setup = [&](encoders::Type type)
    {
        for (size_t i = 0; i < encoders::Collection::size(); i++)
        {
            ASSERT_TRUE(_digital._builderEncoders._database.update(database::Config::Section::Encoder::Mode, i, type));
            _digital._builderEncoders._instance.reset(i);

            // simulate initial reading
            state_change_register(0b00);
        }
    };

    auto verify_value = [&](midi::MessageType message, uint16_t value)
    {
        for (size_t i = 0; i < encoders::Collection::size(); i++)
        {
            ASSERT_EQ(message, _listener.event_log.at(i).message);
            ASSERT_EQ(value, _listener.event_log.at(i).value);
        }
    };

    auto rotate = [this](bool clockwise)
    {
        // skip the initial state
        size_t state_index = 0;

        auto next_value = [&]()
        {
            if (clockwise)
            {
                state_index++;
            }
            else
            {
                state_index--;
            }

            state_index %= ENCODER_STATE.size();
            return ENCODER_STATE.at(state_index);
        };

        for (size_t pulse = 0; pulse < encoders::Filter::PULSES_PER_STEP; pulse++)
        {
            state_change_register(next_value());
        }
    };

    setup(encoders::Type::ControlChange7fh01h);
    rotate(true);
    ASSERT_EQ(encoders::Collection::size(), _listener.event_log.size());
    verify_value(midi::MessageType::ControlChange, 1);
    rotate(false);
    ASSERT_EQ(encoders::Collection::size(), _listener.event_log.size());
    verify_value(midi::MessageType::ControlChange, 127);

    setup(encoders::Type::ControlChange3fh41h);
    rotate(true);
    ASSERT_EQ(encoders::Collection::size(), _listener.event_log.size());
    verify_value(midi::MessageType::ControlChange, 65);
    rotate(false);
    ASSERT_EQ(encoders::Collection::size(), _listener.event_log.size());
    verify_value(midi::MessageType::ControlChange, 63);

    setup(encoders::Type::ControlChange41h01h);
    rotate(true);
    ASSERT_EQ(encoders::Collection::size(), _listener.event_log.size());
    verify_value(midi::MessageType::ControlChange, 1);
    rotate(false);
    ASSERT_EQ(encoders::Collection::size(), _listener.event_log.size());
    verify_value(midi::MessageType::ControlChange, 65);

    setup(encoders::Type::SingleNoteVariableVal);
    rotate(true);
    ASSERT_EQ(encoders::Collection::size(), _listener.event_log.size());
    verify_value(midi::MessageType::NoteOn, 1);
    rotate(false);
    ASSERT_EQ(encoders::Collection::size(), _listener.event_log.size());
    verify_value(midi::MessageType::NoteOn, 0);
}

#else
TEST(DigitalEncodersTest, SkippedWhenPresetDoesNotSupportEncoders)
{
    SUCCEED();
}
#endif
