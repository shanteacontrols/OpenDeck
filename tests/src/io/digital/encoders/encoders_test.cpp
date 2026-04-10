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

#include "tests/common.h"
#include "application/io/digital/encoders/builder.h"
#include "application/util/configurable/configurable.h"
#include "zlibs/utils/misc/mutex.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ENCODERS

using namespace io;
using namespace protocol;

namespace
{
    struct TestEvent
    {
        size_t                     componentIndex = 0;
        uint8_t                    channel        = 0;
        uint16_t                   index          = 0;
        uint16_t                   value          = 0;
        midi::messageType_t        message        = midi::messageType_t::INVALID;
        messaging::systemMessage_t systemMessage  = messaging::systemMessage_t::FORCE_IO_REFRESH;
    };

    class Listener
    {
        public:
        void push(const messaging::MidiSignal& signal)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);

            _event.push_back(TestEvent{
                .componentIndex = signal.componentIndex,
                .channel        = signal.channel,
                .index          = signal.index,
                .value          = signal.value,
                .message        = signal.message,
            });
        }

        void push(const messaging::SystemSignal& signal)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);

            _event.push_back(TestEvent{
                .systemMessage = signal.systemMessage,
            });
        }

        void clear()
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _event.clear();
        }

        size_t size() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _event.size();
        }

        TestEvent at(size_t index) const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _event.at(index);
        }

        private:
        mutable zlibs::utils::misc::Mutex _mutex;

        public:
        std::vector<TestEvent> _event = {};
    };

    class NoOpHandlers : public database::Handlers
    {
        public:
        void presetChange(uint8_t) override
        {}

        void factoryResetStart() override
        {}

        void factoryResetDone() override
        {}

        void initialized() override
        {}
    };

    class EncodersTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_databaseAdmin.init(_handlers));
            ASSERT_TRUE(_databaseAdmin.factoryReset());
            ASSERT_EQ(0, _databaseAdmin.currentPreset());

            // set known state
            for (size_t i = 0; i < encoders::Collection::SIZE(); i++)
            {
                ASSERT_TRUE(_encoders._database.update(database::Config::Section::encoder_t::ENABLE, i, 1));
                ASSERT_TRUE(_encoders._database.update(database::Config::Section::encoder_t::INVERT, i, 0));
                ASSERT_TRUE(_encoders._database.update(database::Config::Section::encoder_t::MODE, i, encoders::type_t::CONTROL_CHANGE_7FH01H));
                ASSERT_TRUE(_encoders._database.update(database::Config::Section::encoder_t::PULSES_PER_STEP, i, 1));
            }

            messaging::subscribe<messaging::MidiSignal>(
                [this](const messaging::MidiSignal& signal)
                {
                    if (signal.source == messaging::MidiSource::Encoder)
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
            messaging::SignalRegistry<messaging::MidiSignal>::instance().clear();
            messaging::SignalRegistry<messaging::SystemSignal>::instance().clear();
            messaging::SignalRegistry<messaging::UmpSignal>::instance().clear();
            messaging::SignalRegistry<messaging::MidiTrafficSignal>::instance().clear();
            _listener.clear();
        }

        void stateChangeRegister(uint8_t state)
        {
            _listener.clear();

            EXPECT_CALL(_encoders._hwa, state(_))
                .WillRepeatedly(Return(std::optional<uint8_t>(state)));

            _encoders._instance.updateAll();
            waitForSignalQuiescence();
        }

        void waitForSignalQuiescence()
        {
            size_t stableIterations = 0;
            size_t lastSize         = static_cast<size_t>(-1);
            size_t maxIterations    = 0;

            while ((stableIterations < 2) && (maxIterations < 8))
            {
                const size_t currentSize = _listener.size();

                if (currentSize == lastSize)
                {
                    stableIterations++;
                }
                else
                {
                    stableIterations = 0;
                    lastSize         = currentSize;
                }

                k_msleep(1);
                maxIterations++;
            }
        }

        static constexpr std::array<uint8_t, 4> ENCODER_STATE = {
            0b00,
            0b10,
            0b11,
            0b01
        };

        NoOpHandlers      _handlers;
        Listener          _listener;
        database::Builder _builderDatabase;
        database::Admin&  _databaseAdmin = _builderDatabase.instance();
        encoders::Builder _encoders      = encoders::Builder(_databaseAdmin);
    };
}    // namespace

TEST_F(EncodersTest, StateDecoding)
{
    if (!encoders::Collection::SIZE())
    {
        return;
    }

    auto verifyValue = [&](midi::messageType_t message, uint16_t value)
    {
        for (size_t i = 0; i < encoders::Collection::SIZE(); i++)
        {
            ASSERT_EQ(message, _listener._event.at(i).message);
            ASSERT_EQ(value, _listener._event.at(i).value);
        }
    };

    // test expected permutations for ccw and cw directions

    // clockwise: 00, 10, 11, 01

    stateChangeRegister(0b00);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b10);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    stateChangeRegister(0b11);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    stateChangeRegister(0b01);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    stateChangeRegister(0b10);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b11);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    stateChangeRegister(0b01);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    stateChangeRegister(0b00);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    stateChangeRegister(0b11);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b01);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    stateChangeRegister(0b00);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    stateChangeRegister(0b10);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    stateChangeRegister(0b01);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b00);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    stateChangeRegister(0b10);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    stateChangeRegister(0b11);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    // counter-clockwise: 00, 01, 11, 10

    stateChangeRegister(0b00);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b01);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    stateChangeRegister(0b11);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    stateChangeRegister(0b10);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    stateChangeRegister(0b01);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b11);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    stateChangeRegister(0b10);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    stateChangeRegister(0b00);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    stateChangeRegister(0b11);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b10);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    stateChangeRegister(0b00);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    stateChangeRegister(0b01);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    stateChangeRegister(0b10);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b00);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    stateChangeRegister(0b01);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    stateChangeRegister(0b11);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    // this time configure 4 pulses per step
    for (size_t i = 0; i < encoders::Collection::SIZE(); i++)
    {
        ASSERT_TRUE(_encoders._database.update(database::Config::Section::encoder_t::PULSES_PER_STEP, i, 4));
        _encoders._instance.reset(i);
    }

    // clockwise: 00, 10, 11, 01

    // initial state doesn't count as pulse, 4 more needed
    stateChangeRegister(0b00);
    ASSERT_EQ(0, _listener._event.size());

    // 1
    stateChangeRegister(0b10);
    ASSERT_EQ(0, _listener._event.size());

    // 2
    stateChangeRegister(0b11);
    ASSERT_EQ(0, _listener._event.size());

    // 3
    stateChangeRegister(0b01);
    ASSERT_EQ(0, _listener._event.size());

    // 4
    // pulse should be registered
    stateChangeRegister(0b00);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    // 1
    stateChangeRegister(0b10);
    ASSERT_EQ(0, _listener._event.size());

    // 2
    stateChangeRegister(0b11);
    ASSERT_EQ(0, _listener._event.size());

    // 3
    stateChangeRegister(0b01);
    ASSERT_EQ(0, _listener._event.size());

    // 4
    stateChangeRegister(0b00);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);

    // now move to opposite direction
    // don't start from 0b00 state again
    // counter-clockwise: 01, 11, 10, 00

    stateChangeRegister(0b01);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b11);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b10);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b00);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);
}

TEST_F(EncodersTest, Messages)
{
    if (!encoders::Collection::SIZE())
    {
        return;
    }

    constexpr size_t PULSES_PER_STEP = 4;

    auto setup = [&](encoders::type_t type)
    {
        for (size_t i = 0; i < encoders::Collection::SIZE(); i++)
        {
            ASSERT_TRUE(_encoders._database.update(database::Config::Section::encoder_t::PULSES_PER_STEP, i, PULSES_PER_STEP));
            ASSERT_TRUE(_encoders._database.update(database::Config::Section::encoder_t::MODE, i, type));
            _encoders._instance.reset(i);

            // simulate initial reading
            stateChangeRegister(0b00);
        }
    };

    auto verifyValue = [&](midi::messageType_t message, uint16_t value)
    {
        for (size_t i = 0; i < encoders::Collection::SIZE(); i++)
        {
            ASSERT_EQ(message, _listener._event.at(i).message);
            ASSERT_EQ(value, _listener._event.at(i).value);
        }
    };

    auto rotate = [this](bool clockwise)
    {
        // skip the initial state
        size_t stateIndex = 0;

        auto nextValue = [&]()
        {
            if (clockwise)
            {
                stateIndex++;
            }
            else
            {
                stateIndex--;
            }

            stateIndex %= ENCODER_STATE.size();
            return ENCODER_STATE.at(stateIndex);
        };

        for (size_t pulse = 0; pulse < PULSES_PER_STEP; pulse++)
        {
            stateChangeRegister(nextValue());
        }
    };

    setup(encoders::type_t::CONTROL_CHANGE_7FH01H);
    rotate(true);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);
    rotate(false);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 127);

    setup(encoders::type_t::CONTROL_CHANGE_3FH41H);
    rotate(true);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 65);
    rotate(false);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 63);

    setup(encoders::type_t::CONTROL_CHANGE_41H01H);
    rotate(true);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 1);
    rotate(false);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::CONTROL_CHANGE, 65);

    setup(encoders::type_t::SINGLE_NOTE_VARIABLE_VAL);
    rotate(true);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::NOTE_ON, 1);
    rotate(false);
    ASSERT_EQ(encoders::Collection::SIZE(), _listener._event.size());
    verifyValue(midi::messageType_t::NOTE_ON, 0);
}

#endif
