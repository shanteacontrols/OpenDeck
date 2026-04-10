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
#include "application/io/digital/buttons/builder.h"
#include "application/io/digital/drivers/buffer.h"
#include "application/util/configurable/configurable.h"
#include "zlibs/utils/misc/mutex.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BUTTONS

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
                .componentIndex = 0,
                .channel        = 0,
                .index          = 0,
                .value          = signal.value,
                .message        = midi::messageType_t::INVALID,
                .systemMessage  = signal.systemMessage,
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

        std::vector<TestEvent> snapshot() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _event;
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

    class ButtonsTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_databaseAdmin.init(_handlers));
            ASSERT_TRUE(_databaseAdmin.factoryReset());
            ASSERT_EQ(0, _databaseAdmin.currentPreset());

            for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
            {
                ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::TYPE, i, buttons::type_t::MOMENTARY));
                ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MESSAGE_TYPE, i, buttons::messageType_t::NOTE));
                ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::VALUE, i, 127));

                _buttons._instance.reset(i);
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
            messaging::SignalRegistry<messaging::MidiSignal>::instance().clear();
            messaging::SignalRegistry<messaging::SystemSignal>::instance().clear();
            messaging::SignalRegistry<messaging::UmpSignal>::instance().clear();
            messaging::SignalRegistry<messaging::MidiTrafficSignal>::instance().clear();
            _listener.clear();
        }

        void stateChangeRegisterAll(bool state)
        {
            _listener.clear();

            EXPECT_CALL(_buttons._hwa, state(_))
                .WillRepeatedly(Return(std::optional<bool>(state)));

            _buttons._instance.updateAll();
            waitForSignalQuiescence();
        }

        void stateChangeRegisterSingle(size_t index, bool state)
        {
            _listener.clear();

            EXPECT_CALL(_buttons._hwa, state(_))
                .WillRepeatedly(Return(std::optional<bool>(state)));

            _buttons._instance.updateSingle(index);
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

        NoOpHandlers      _handlers;
        Listener          _listener;
        database::Builder _builderDatabase;
        database::Admin&  _databaseAdmin = _builderDatabase.instance();
        buttons::Builder  _buttons       = buttons::Builder(_databaseAdmin);
    };
}    // namespace

TEST(ButtonsBufferTest, PreservesOrderAndDrains)
{
    io::digital::drivers::Buffer<uint8_t> buffer = {};

    EXPECT_TRUE(buffer.empty());
    EXPECT_FALSE(buffer.pop().has_value());

    buffer.push(1);
    buffer.push(2);
    buffer.push(3);

    ASSERT_FALSE(buffer.empty());

    auto value = buffer.pop();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(1, value.value());

    value = buffer.pop();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(2, value.value());

    value = buffer.pop();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(3, value.value());

    EXPECT_FALSE(buffer.pop().has_value());
    EXPECT_TRUE(buffer.empty());
}

TEST(ButtonsBufferTest, OverwritesOldestUnreadSampleWhenFull)
{
    io::digital::drivers::Buffer<uint8_t> buffer         = {};
    constexpr uint8_t                     OVERFLOW_COUNT = 2;

    for (uint8_t i = 0; i < (io::digital::drivers::Buffer<uint8_t>::CAPACITY + OVERFLOW_COUNT); i++)
    {
        buffer.push(i);
    }

    for (uint8_t expected = OVERFLOW_COUNT; expected < (io::digital::drivers::Buffer<uint8_t>::CAPACITY + OVERFLOW_COUNT); expected++)
    {
        auto value = buffer.pop();
        ASSERT_TRUE(value.has_value());
        EXPECT_EQ(expected, value.value());
    }

    EXPECT_FALSE(buffer.pop().has_value());
}

TEST_F(ButtonsTest, Note)
{
    if (!buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS))
    {
        return;
    }

    auto test = [&](uint8_t channel, uint8_t velocity)
    {
        // set known state
        for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::TYPE, i, buttons::type_t::MOMENTARY));
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MESSAGE_TYPE, i, buttons::messageType_t::NOTE));
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::VALUE, i, velocity));
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::CHANNEL, i, channel));

            _buttons._instance.reset(i);
        }

        auto verifyValue = [&](bool state)
        {
            // verify all received messages
            for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
            {
                if (state)
                {
                    ASSERT_EQ(midi::messageType_t::NOTE_ON, _listener._event.at(i).message);
                }
                else
                {
                    ASSERT_EQ(midi::messageType_t::NOTE_OFF, _listener._event.at(i).message);
                }

                ASSERT_EQ(state ? velocity : 0, _listener._event.at(i).value);
                ASSERT_EQ(channel, _listener._event.at(i).channel);

                // also verify MIDI ID
                // it should be equal to button ID by default
                ASSERT_EQ(i, _listener._event.at(i).index);
            }
        };

        // simulate button press
        stateChangeRegisterAll(true);
        ASSERT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyValue(true);

        // simulate button release
        stateChangeRegisterAll(false);
        ASSERT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyValue(false);

        // try with the latching mode
        for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::TYPE, i, buttons::type_t::LATCHING));
        }

        stateChangeRegisterAll(true);
        ASSERT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyValue(true);

        // nothing should happen on release
        stateChangeRegisterAll(false);
        ASSERT_EQ(0, _listener._event.size());

        // press again, new messages should arrive
        stateChangeRegisterAll(true);
        verifyValue(false);
    };

    const std::array<uint8_t, 3> CHANNELS   = { 1, 8, 16 };
    const std::array<uint8_t, 3> VELOCITIES = { 1, 64, 127 };

    for (auto channel : CHANNELS)
    {
        for (auto velocity : VELOCITIES)
        {
            test(channel, velocity);
        }
    }
}

TEST_F(ButtonsTest, ProgramChange)
{
    if (!buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS))
    {
        return;
    }

    auto testProgramChange = [&](uint8_t channel)
    {
        // set known state
        for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::TYPE, i, buttons::type_t::MOMENTARY));
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MESSAGE_TYPE, i, buttons::messageType_t::PROGRAM_CHANGE));
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::CHANNEL, i, channel));

            _buttons._instance.reset(i);
        }

        auto verifyMessage = [&]()
        {
            // verify all received messages are program change
            for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
            {
                ASSERT_EQ(midi::messageType_t::PROGRAM_CHANGE, _listener._event.at(i).message);

                // program change value should always be set to 0
                ASSERT_EQ(0, _listener._event.at(i).value);

                // verify channel
                ASSERT_EQ(channel, _listener._event.at(i).channel);

                // also verify MIDI ID/program
                // it should be equal to button ID by default
                ASSERT_EQ(i, _listener._event.at(i).index);
            }
        };

        // simulate button press
        stateChangeRegisterAll(true);
        ASSERT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyMessage();

        // program change shouldn't be sent on release
        stateChangeRegisterAll(false);
        ASSERT_EQ(0, _listener._event.size());

        // repeat the entire test again, but with buttons configured as latching types
        // behaviour should be the same
        for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::TYPE, i, buttons::type_t::LATCHING));
        }

        stateChangeRegisterAll(true);
        ASSERT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyMessage();

        stateChangeRegisterAll(false);
        ASSERT_EQ(0, _listener._event.size());
    };

    for (auto channel : std::array<uint8_t, 3>{ 1, 8, 16 })
    {
        testProgramChange(channel);
    }

    // test PROGRAM_CHANGE_INC/PROGRAM_CHANGE_DEC
    _databaseAdmin.factoryReset();
    stateChangeRegisterAll(false);

    auto configurePCbutton = [&](size_t index, uint8_t channel, bool increase)
    {
        ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::TYPE, index, buttons::type_t::MOMENTARY));
        ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MESSAGE_TYPE, index, increase ? buttons::messageType_t::PROGRAM_CHANGE_INC : buttons::messageType_t::PROGRAM_CHANGE_DEC));
        ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::CHANNEL, index, channel));

        for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            _buttons._instance.reset(i);
        }
    };

    auto verifyProgramChange = [&](size_t index, uint8_t channel, uint8_t program)
    {
        ASSERT_EQ(midi::messageType_t::PROGRAM_CHANGE, _listener._event.at(index).message);

        // program change value should always be set to 0
        ASSERT_EQ(0, _listener._event.at(index).value);

        // verify channel
        ASSERT_EQ(channel, _listener._event.at(index).channel);

        ASSERT_EQ(program, _listener._event.at(index).index);
    };

    static constexpr uint8_t CHANNEL = 1;

    configurePCbutton(0, CHANNEL, true);

    // verify that the received program change was 1 for button 0
    stateChangeRegisterAll(true);
    verifyProgramChange(0, CHANNEL, 1);
    stateChangeRegisterAll(false);

    // after this, verify that the received program change was 2 for button 0
    stateChangeRegisterAll(true);
    verifyProgramChange(0, CHANNEL, 2);
    stateChangeRegisterAll(false);

    // after this, verify that the received program change was 3 for button 0
    stateChangeRegisterAll(true);
    verifyProgramChange(0, CHANNEL, 3);
    stateChangeRegisterAll(false);

    // now, revert all buttons back to default
    _databaseAdmin.factoryReset();

    if (buttons::Collection::SIZE(io::buttons::GROUP_DIGITAL_INPUTS) < 4)
    {
        return;
    }

    // configure some other button to PROGRAM_CHANGE_INC
    configurePCbutton(4, CHANNEL, true);

    // verify that the program change is continuing to increase
    stateChangeRegisterAll(true);
    verifyProgramChange(4, CHANNEL, 4);
    stateChangeRegisterAll(false);

    stateChangeRegisterAll(true);
    verifyProgramChange(4, CHANNEL, 5);
    stateChangeRegisterAll(false);

    // now configure two buttons to send program change/inc
    configurePCbutton(0, CHANNEL, true);

    stateChangeRegisterAll(true);
    // program change should be increased by 1, first by button 0
    verifyProgramChange(0, CHANNEL, 6);
    // then by button 4
    verifyProgramChange(4, CHANNEL, 7);
    stateChangeRegisterAll(false);

    // configure another button to PROGRAM_CHANGE_INC, but on other channel
    configurePCbutton(1, 4, true);

    stateChangeRegisterAll(true);
    // program change should be increased by 1, first by button 0
    verifyProgramChange(0, CHANNEL, 8);
    // then by button 4
    verifyProgramChange(4, CHANNEL, 9);
    // program change should be sent on channel 4 by button 1
    verifyProgramChange(1, 4, 1);
    stateChangeRegisterAll(false);

    // revert to default again
    _databaseAdmin.factoryReset();

    // now configure button 0 for PROGRAM_CHANGE_DEC
    configurePCbutton(0, CHANNEL, false);

    stateChangeRegisterAll(true);
    // program change should decrease by 1
    verifyProgramChange(0, CHANNEL, 8);
    stateChangeRegisterAll(false);

    stateChangeRegisterAll(true);
    // program change should decrease by 1 again
    verifyProgramChange(0, CHANNEL, 7);
    stateChangeRegisterAll(false);

    // configure another button for PROGRAM_CHANGE_DEC
    configurePCbutton(1, CHANNEL, false);

    stateChangeRegisterAll(true);
    // button 0 should decrease the value by 1
    verifyProgramChange(0, CHANNEL, 6);
    // button 1 should decrease it again
    verifyProgramChange(1, CHANNEL, 5);
    stateChangeRegisterAll(false);

    // configure another button for PROGRAM_CHANGE_DEC
    configurePCbutton(2, CHANNEL, false);

    stateChangeRegisterAll(true);
    // button 0 should decrease the value by 1
    verifyProgramChange(0, CHANNEL, 4);
    // button 1 should decrease it again
    verifyProgramChange(1, CHANNEL, 3);
    // button 2 should decrease it again
    verifyProgramChange(2, CHANNEL, 2);
    stateChangeRegisterAll(false);

    // reset all received messages first
    _listener._event.clear();

    // only two program change messages should be sent
    // program change value is 0 after the second button decreases it
    // once the value is 0 no further messages should be sent in dec mode

    stateChangeRegisterAll(true);
    // button 0 should decrease the value by 1
    verifyProgramChange(0, CHANNEL, 1);
    // button 1 should decrease it again
    verifyProgramChange(1, CHANNEL, 0);

    // verify that only two program change messages have been received
    uint8_t pcCounter = 0;

    for (size_t i = 0; i < _listener._event.size(); i++)
    {
        if (_listener._event.at(i).message == midi::messageType_t::PROGRAM_CHANGE)
        {
            pcCounter++;
        }
    }

    ASSERT_EQ(2, pcCounter);

    stateChangeRegisterAll(false);

    // revert all buttons to default
    _databaseAdmin.factoryReset();

    configurePCbutton(0, CHANNEL, true);

    stateChangeRegisterAll(true);
    // button 0 should increase the last value by 1
    verifyProgramChange(0, CHANNEL, 1);
    stateChangeRegisterAll(false);
}

TEST_F(ButtonsTest, ProgramChangeWithOffset)
{
    if (buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) < 3)
    {
        return;
    }

    static constexpr size_t BUTTON_INDEX_OFFSET_DEC = 0;
    static constexpr size_t BUTTON_INDEX_OFFSET_INC = 1;
    static constexpr size_t BUTTON_INDEX_PROGRAM    = 2;
    static constexpr size_t PROGRAM_CHANGE_CHANNEL  = 1;
    static constexpr size_t OFFSET_INC_DEC_AMOUNT   = 3;

    auto verifyProgramChange = [&](size_t index, uint8_t channel, uint8_t program)
    {
        ASSERT_EQ(midi::messageType_t::PROGRAM_CHANGE, _listener._event.at(index).message);

        // program change value should always be set to 0
        ASSERT_EQ(0, _listener._event.at(index).value);

        // verify channel
        ASSERT_EQ(channel, _listener._event.at(index).channel);

        ASSERT_EQ(program, _listener._event.at(index).index);
    };

    auto toggleProgramChangeIncButton = [&]()
    {
        stateChangeRegisterSingle(BUTTON_INDEX_OFFSET_INC, true);
        ASSERT_TRUE(MidiProgram.offset() >= 0);
        ASSERT_TRUE(MidiProgram.offset() <= 127);
        stateChangeRegisterSingle(BUTTON_INDEX_OFFSET_INC, false);
    };

    auto toggleProgramChangeDecButton = [&]()
    {
        stateChangeRegisterSingle(BUTTON_INDEX_OFFSET_DEC, true);
        ASSERT_TRUE(MidiProgram.offset() >= 0);
        ASSERT_TRUE(MidiProgram.offset() <= 127);
        stateChangeRegisterSingle(BUTTON_INDEX_OFFSET_DEC, false);
    };

    auto toggleProgramChangeButton = [&]()
    {
        stateChangeRegisterSingle(BUTTON_INDEX_PROGRAM, true);
        auto                   events         = _listener.snapshot();
        std::vector<TestEvent> programChanges = {};

        for (const auto& event : events)
        {
            if (event.message == midi::messageType_t::PROGRAM_CHANGE)
            {
                programChanges.push_back(event);
            }
        }

        ASSERT_EQ(1, programChanges.size());
        ASSERT_EQ(midi::messageType_t::PROGRAM_CHANGE, programChanges.at(0).message);
        ASSERT_EQ(0, programChanges.at(0).value);
        ASSERT_EQ(PROGRAM_CHANGE_CHANNEL, programChanges.at(0).channel);
        ASSERT_EQ(BUTTON_INDEX_PROGRAM + MidiProgram.offset(), programChanges.at(0).index);
        stateChangeRegisterSingle(BUTTON_INDEX_PROGRAM, false);
    };

    // set known state
    ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MESSAGE_TYPE, BUTTON_INDEX_OFFSET_DEC, buttons::messageType_t::PROGRAM_CHANGE_OFFSET_DEC));
    ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::VALUE, BUTTON_INDEX_OFFSET_DEC, OFFSET_INC_DEC_AMOUNT));
    ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MESSAGE_TYPE, BUTTON_INDEX_OFFSET_INC, buttons::messageType_t::PROGRAM_CHANGE_OFFSET_INC));
    ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::VALUE, BUTTON_INDEX_OFFSET_INC, OFFSET_INC_DEC_AMOUNT));
    ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MESSAGE_TYPE, BUTTON_INDEX_PROGRAM, buttons::messageType_t::PROGRAM_CHANGE));

    _buttons._instance.reset(BUTTON_INDEX_OFFSET_DEC);
    _buttons._instance.reset(BUTTON_INDEX_OFFSET_INC);
    _buttons._instance.reset(BUTTON_INDEX_PROGRAM);

    toggleProgramChangeButton();
    toggleProgramChangeIncButton();
    toggleProgramChangeButton();

    // toggle the incrementing button few more times
    for (size_t i = 0; i < 10; i++)
    {
        toggleProgramChangeIncButton();
    }

    toggleProgramChangeButton();

    // verify that incrementing past the value of 127 isn't possible
    while (MidiProgram.offset() != 127)
    {
        toggleProgramChangeIncButton();
    }

    // try again - nothing should change
    toggleProgramChangeIncButton();
    ASSERT_EQ(127, MidiProgram.offset());

    // start decrementing offset
    for (size_t i = 0; i < 50; i++)
    {
        toggleProgramChangeDecButton();
    }

    toggleProgramChangeButton();

    MidiProgram.setOffset(0);
    ASSERT_EQ(0, MidiProgram.offset());

    toggleProgramChangeButton();

    // decrement again - verify that nothing changes
    toggleProgramChangeDecButton();
    ASSERT_EQ(0, MidiProgram.offset());
}

TEST_F(ButtonsTest, ControlChange)
{
    if (!buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS))
    {
        return;
    }

    auto controlChangeTest = [&](uint8_t controlValue)
    {
        // set known state
        for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::TYPE, i, buttons::type_t::MOMENTARY));
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MESSAGE_TYPE, i, buttons::messageType_t::CONTROL_CHANGE));
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::VALUE, i, controlValue));

            _buttons._instance.reset(i);
        }

        auto verifyMessage = [&](uint8_t midiValue)
        {
            // verify all received messages are control change
            for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
            {
                ASSERT_EQ(midi::messageType_t::CONTROL_CHANGE, _listener._event.at(i).message);
                ASSERT_EQ(midiValue, _listener._event.at(i).value);
                ASSERT_EQ(1, _listener._event.at(i).channel);
                ASSERT_EQ(i, _listener._event.at(i).index);
            }
        };

        // simulate button press
        stateChangeRegisterAll(true);
        ASSERT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(controlValue);

        // no messages should be sent on release
        stateChangeRegisterAll(false);
        ASSERT_EQ(0, _listener._event.size());

        // change to latching type
        // behaviour should be the same

        for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::TYPE, i, buttons::type_t::LATCHING));
        }

        stateChangeRegisterAll(true);
        ASSERT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(controlValue);

        stateChangeRegisterAll(false);
        ASSERT_EQ(0, _listener._event.size());

        // change type to control change with 0 on reset, momentary mode
        // this means CC value 0 should be sent on release

        for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::TYPE, i, buttons::type_t::MOMENTARY));
        }

        for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MESSAGE_TYPE, i, buttons::messageType_t::CONTROL_CHANGE_RESET));
        }

        stateChangeRegisterAll(true);
        ASSERT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(controlValue);

        stateChangeRegisterAll(false);
        ASSERT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(0);

        // same test again, but in latching mode
        // now, on press, messages should be sent
        // on release, nothing should happen
        // on second press reset should be sent (CC with value 0)

        for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::TYPE, i, buttons::type_t::LATCHING));
        }

        stateChangeRegisterAll(true);
        ASSERT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(controlValue);

        stateChangeRegisterAll(false);
        ASSERT_EQ(0, _listener._event.size());

        stateChangeRegisterAll(true);
        ASSERT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(0);
    };

    // verify representative control values across the valid range
    for (auto value : std::array<uint8_t, 3>{ 1, 64, 127 })
    {
        controlChangeTest(value);
    }
}

TEST_F(ButtonsTest, NoMessages)
{
    if (!buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS))
    {
        return;
    }

    // configure all buttons to messageType_t::NONE so that messages aren't sent

    for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
    {
        ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MESSAGE_TYPE, i, buttons::messageType_t::NONE));

        _buttons._instance.reset(i);
    }

    stateChangeRegisterAll(true);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegisterAll(false);
    ASSERT_EQ(0, _listener._event.size());

    for (size_t i = 0; i < buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS); i++)
    {
        ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::TYPE, i, buttons::type_t::LATCHING));
    }

    stateChangeRegisterAll(true);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegisterAll(false);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegisterAll(true);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegisterAll(false);
    ASSERT_EQ(0, _listener._event.size());
}

TEST_F(ButtonsTest, PresetChange)
{
    if (_databaseAdmin.getSupportedPresets() <= 1)
    {
        return;
    }

    if (!buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS))
    {
        return;
    }

    // configure one button to change preset
    static constexpr size_t BUTTON_INDEX = 0;

    ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MIDI_ID, BUTTON_INDEX, 1));
    ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MESSAGE_TYPE, BUTTON_INDEX, buttons::messageType_t::PRESET_CHANGE));
    _buttons._instance.reset(BUTTON_INDEX);

    // simulate button press
    stateChangeRegisterSingle(BUTTON_INDEX, true);

    ASSERT_EQ(1, _listener._event.size());
    ASSERT_EQ(messaging::systemMessage_t::PRESET_CHANGE_DIRECT_REQ, _listener._event.at(0).systemMessage);

    // verify that no new events are generated on button release
    stateChangeRegisterSingle(BUTTON_INDEX, false);
    ASSERT_EQ(0, _listener._event.size());
}

TEST_F(ButtonsTest, MMCStartStop)
{
    if (!buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS))
    {
        return;
    }

    // configure one button to MMC_PLAY_STOP message type
    static constexpr size_t BUTTON_INDEX = 0;

    ASSERT_TRUE(_buttons._database.update(database::Config::Section::button_t::MESSAGE_TYPE, BUTTON_INDEX, buttons::messageType_t::MMC_PLAY_STOP));
    _buttons._instance.reset(BUTTON_INDEX);

    // simulate button press
    stateChangeRegisterSingle(BUTTON_INDEX, true);

    ASSERT_EQ(1, _listener._event.size());
    ASSERT_EQ(midi::messageType_t::MMC_PLAY, _listener._event.at(0).message);

    // verify that no new events are generated on button release
    stateChangeRegisterSingle(BUTTON_INDEX, false);
    ASSERT_EQ(0, _listener._event.size());

    // simulate a new button press
    stateChangeRegisterSingle(BUTTON_INDEX, true);

    // the message should be MMC_STOP now
    ASSERT_EQ(1, _listener._event.size());
    ASSERT_EQ(midi::messageType_t::MMC_STOP, _listener._event.at(0).message);
}

#endif
