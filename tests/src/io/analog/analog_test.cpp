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

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ADC

#include "tests/common.h"
#include "application/io/analog/builder.h"
#include "application/io/digital/buttons/builder.h"
#include "application/io/digital/buttons/buttons.h"
#include "application/util/configurable/configurable.h"
#include "zlibs/utils/misc/mutex.h"

using namespace io;
using namespace protocol;

namespace
{
    class MidiSignalCollector
    {
        public:
        void push(const messaging::MidiSignal& signal)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _signals.push_back(signal);
        }

        void clear()
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _signals.clear();
        }

        size_t size() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _signals.size();
        }

        std::vector<messaging::MidiSignal> snapshot() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _signals;
        }

        private:
        mutable zlibs::utils::misc::Mutex  _mutex;
        std::vector<messaging::MidiSignal> _signals = {};
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

    class AnalogTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_databaseAdmin.init(_handlers));
            ASSERT_TRUE(_databaseAdmin.factoryReset());
            ASSERT_EQ(0, _databaseAdmin.currentPreset());
            ASSERT_TRUE(_analog._instance.init());
            EXPECT_CALL(_buttons._hwa, init())
                .WillOnce(Return(true));
            ASSERT_TRUE(_buttons._instance.init());

            for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
            {
                ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::ENABLE, i, 1));
                ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::CHANNEL, i, 1));
            }

            messaging::subscribe<messaging::MidiSignal>(
                [this](const messaging::MidiSignal& signal)
                {
                    if (signal.source == messaging::MidiSource::Analog)
                    {
                        _analogMessages.push(signal);
                    }

                    if (signal.source == messaging::MidiSource::AnalogButton)
                    {
                        _analogButtonMessages.push(signal);
                    }

                    if (signal.source == messaging::MidiSource::Button)
                    {
                        _buttonMessages.push(signal);
                    }
                });

            // The signaling dispatcher is asynchronous and shared across test
            // cases, so allow any stale queued traffic to drain before each
            // test starts asserting on fresh messages.
            k_msleep(20);
            clearMessages();
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            messaging::SignalRegistry<messaging::MidiSignal>::instance().clear();
            messaging::SignalRegistry<messaging::SystemSignal>::instance().clear();
            messaging::SignalRegistry<messaging::UmpSignal>::instance().clear();
            messaging::SignalRegistry<messaging::MidiTrafficSignal>::instance().clear();
            clearMessages();
        }

        void stateChangeRegister(uint16_t value)
        {
            EXPECT_CALL(_analog._hwa, value(_))
                .WillRepeatedly(Return(std::optional<uint16_t>(value)));

            _analog._instance.updateAll();
        }

        void clearMessages()
        {
            _analogMessages.clear();
            _analogButtonMessages.clear();
            _buttonMessages.clear();
        }

        void waitForSignalQuiescence()
        {
            size_t stableIterations = 0;
            size_t lastSize         = static_cast<size_t>(-1);

            while (stableIterations < 10)
            {
                const size_t currentSize = _analogMessages.size() + _analogButtonMessages.size() + _buttonMessages.size();

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
            }
        }

        NoOpHandlers        _handlers;
        database::Builder   _builderDatabase;
        database::Admin&    _databaseAdmin = _builderDatabase.instance();
        analog::Builder     _analog        = analog::Builder(_databaseAdmin);
        buttons::Builder    _buttons       = buttons::Builder(_databaseAdmin);
        MidiSignalCollector _analogMessages;
        MidiSignalCollector _analogButtonMessages;
        MidiSignalCollector _buttonMessages;
    };
}    // namespace

TEST_F(AnalogTest, CC)
{
    // feed all the values from minimum to maximum
    // expect the following:
    // first value is 0
    // last value is 127

    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    waitForSignalQuiescence();
    auto analogMessages = _analogMessages.snapshot();

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128), analogMessages.size());

    // all received messages should be control change
    for (size_t i = 0; i < analogMessages.size(); i++)
    {
        EXPECT_EQ(midi::messageType_t::CONTROL_CHANGE, analogMessages.at(i).message);
    }

    uint8_t expectedMIDIvalue = 0;

    for (size_t i = 0; i < analogMessages.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, analogMessages.at(index).value);
        }

        expectedMIDIvalue++;
    }

    // now go backward

    _analogMessages.clear();

    for (int i = 126; i >= 0; i--)
    {
        stateChangeRegister(i);
    }

    waitForSignalQuiescence();
    analogMessages = _analogMessages.snapshot();

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 127), analogMessages.size());

    expectedMIDIvalue = 126;

    for (size_t i = 0; i < analogMessages.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, analogMessages.at(index).value);
        }

        expectedMIDIvalue--;
    }

    _analogMessages.clear();

    // try to feed value larger than 127
    // no response should be received
    stateChangeRegister(10000);
    waitForSignalQuiescence();

    EXPECT_EQ(0, _analogMessages.size());
}

TEST_F(AnalogTest, NRPN7bit)
{
    // set known state
    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        // configure all analog components as potentiometers with 7-bit NRPN MIDI message
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::TYPE, i, analog::type_t::NRPN_7BIT));
    }

    // feed all the values from minimum to maximum

    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    waitForSignalQuiescence();
    auto analogMessages = _analogMessages.snapshot();

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128), analogMessages.size());

    // all received messages should be NRPN
    for (size_t i = 0; i < analogMessages.size(); i++)
    {
        EXPECT_EQ(midi::messageType_t::NRPN_7BIT, analogMessages.at(i).message);
    }
}

TEST_F(AnalogTest, NRPN14bit)
{
    // set known state
    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        // configure all analog components as potentiometers with 14-bit NRPN MIDI message
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::TYPE, i, analog::type_t::NRPN_14BIT));
    }

    // feed all the values from minimum to maximum

    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    waitForSignalQuiescence();
    auto analogMessages = _analogMessages.snapshot();

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128), analogMessages.size());

    // all received messages should be NRPN
    for (size_t i = 0; i < analogMessages.size(); i++)
    {
        EXPECT_EQ(midi::messageType_t::NRPN_14BIT, analogMessages.at(i).message);
    }
}

TEST_F(AnalogTest, PitchBendTest)
{
    // set known state
    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        // configure all analog components as potentiometers with Pitch Bend MIDI message
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::TYPE, i, analog::type_t::PITCH_BEND));
    }

    for (int i = 0; i <= 16383; i++)
    {
        stateChangeRegister(i);
    }

    waitForSignalQuiescence();
    auto analogMessages = _analogMessages.snapshot();

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 16384), analogMessages.size());

    // //all received messages should be pitch bend
    for (size_t i = 0; i < analogMessages.size(); i++)
    {
        EXPECT_EQ(midi::messageType_t::PITCH_BEND, analogMessages.at(i).message);
    }

    uint16_t expectedMIDIvalue = 0;

    for (size_t i = 0; i < analogMessages.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, analogMessages.at(index).value);
        }

        expectedMIDIvalue++;
    }

    // now go backward
    _analogMessages.clear();

    for (int i = 16382; i >= 0; i--)
    {
        stateChangeRegister(i);
    }

    // one message less
    waitForSignalQuiescence();
    analogMessages = _analogMessages.snapshot();

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 16383), analogMessages.size());

    expectedMIDIvalue = 16382;

    for (size_t i = 0; i < analogMessages.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, analogMessages.at(index).value);
        }

        expectedMIDIvalue--;
    }
}

TEST_F(AnalogTest, Inversion)
{
    // enable inversion
    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::INVERT, i, 1));
    }

    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    waitForSignalQuiescence();
    auto analogMessages = _analogMessages.snapshot();

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128), analogMessages.size());

    // first value should be 127
    // last value should be 0

    uint16_t expectedMIDIvalue = 127;

    for (size_t i = 0; i < analogMessages.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, analogMessages.at(index).value);
        }

        expectedMIDIvalue--;
    }

    _analogMessages.clear();

    // funky setup: set lower limit to 127, upper to 0 while inversion is enabled
    // result should be the same as when default setup is used (no inversion / 0 as lower limit, 127 as upper limit)

    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::INVERT, i, 1));
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::LOWER_LIMIT, i, 127));
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::UPPER_LIMIT, i, 0));

        _analog._instance.reset(i);
    }

    // feed all the values again
    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    waitForSignalQuiescence();
    analogMessages = _analogMessages.snapshot();

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128), analogMessages.size());

    expectedMIDIvalue = 0;

    for (size_t i = 0; i < analogMessages.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, analogMessages.at(index).value);
        }

        expectedMIDIvalue++;
    }

    _analogMessages.clear();

    // now disable inversion
    _analogMessages.clear();

    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::INVERT, i, 0));
        _analog._instance.reset(i);
    }

    // feed all the values again
    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    waitForSignalQuiescence();
    analogMessages = _analogMessages.snapshot();

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128), analogMessages.size());

    expectedMIDIvalue = 127;

    for (size_t i = 0; i < analogMessages.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, analogMessages.at(index).value);
        }

        expectedMIDIvalue--;
    }
}

TEST_F(AnalogTest, Scaling)
{
    if (!analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        return;
    }

    static constexpr uint32_t SCALED_LOWER = 11;
    static constexpr uint32_t SCALED_UPPER = 100;

    // set known state
    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::LOWER_LIMIT, i, 0));
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::UPPER_LIMIT, i, SCALED_UPPER));
    }

    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    waitForSignalQuiescence();
    auto analogMessages = _analogMessages.snapshot();

    // since the values are scaled, verify that all the messages aren't received
    std::cout << "Received " << analogMessages.size() << " messages" << std::endl;
    ASSERT_TRUE((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128) > analogMessages.size());

    // first value should be 0
    // last value should match the configured scaled value (SCALED_UPPER)
    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        EXPECT_EQ(0, analogMessages.at(i).value);
        EXPECT_EQ(SCALED_UPPER, analogMessages.at(analogMessages.size() - analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) + i).value);
    }

    // now scale minimum value as well
    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::LOWER_LIMIT, i, SCALED_LOWER));
    }

    _analogMessages.clear();

    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    waitForSignalQuiescence();
    analogMessages = _analogMessages.snapshot();
    std::cout << "Received " << analogMessages.size() << " messages" << std::endl;
    ASSERT_TRUE((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128) > analogMessages.size());

    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(analogMessages.at(i).value >= SCALED_LOWER);
        ASSERT_TRUE(analogMessages.at(analogMessages.size() - analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) + i).value <= SCALED_UPPER);
    }

    // now enable inversion
    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::INVERT, i, 1));
        _analog._instance.reset(i);
    }

    _analogMessages.clear();

    for (int i = 1; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    waitForSignalQuiescence();
    analogMessages = _analogMessages.snapshot();

    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(analogMessages.at(i).value >= SCALED_UPPER);
        ASSERT_TRUE(analogMessages.at(analogMessages.size() - analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) + i).value <= SCALED_LOWER);
    }
}

TEST_F(AnalogTest, ButtonForwarding)
{
    if (!analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        return;
    }

    // configure one analog component to be button type
    static constexpr size_t  BUTTON_INDEX        = 1;
    static constexpr uint8_t BUTTON_MIDI_CHANNEL = 2;
    static constexpr uint8_t BUTTON_VELOCITY     = 100;

    ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::TYPE, BUTTON_INDEX, analog::type_t::BUTTON) == true);

    // configure button with the same index (+offset) to certain parameters
    ASSERT_TRUE(_databaseAdmin.update(database::Config::Section::button_t::TYPE, buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, buttons::type_t::MOMENTARY));
    ASSERT_TRUE(_databaseAdmin.update(database::Config::Section::button_t::CHANNEL, buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, BUTTON_MIDI_CHANNEL));
    ASSERT_TRUE(_databaseAdmin.update(database::Config::Section::button_t::MESSAGE_TYPE, buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, buttons::messageType_t::CONTROL_CHANGE_RESET));
    ASSERT_TRUE(_databaseAdmin.update(database::Config::Section::button_t::VALUE, buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, BUTTON_VELOCITY));

    stateChangeRegister(0xFFFF);
    waitForSignalQuiescence();
    auto analogButtonMessages = _analogButtonMessages.snapshot();

    ASSERT_EQ(1, analogButtonMessages.size());
    EXPECT_EQ(BUTTON_INDEX, analogButtonMessages.at(0).componentIndex);

    stateChangeRegister(0);
    waitForSignalQuiescence();
    analogButtonMessages = _analogButtonMessages.snapshot();
    auto buttonMessages  = _buttonMessages.snapshot();

    // In CONTROL_CHANGE_RESET mode the button path emits once on press and
    // once on release.
    ASSERT_EQ(2, analogButtonMessages.size());
    EXPECT_EQ(BUTTON_INDEX, analogButtonMessages.at(1).componentIndex);
    ASSERT_EQ(2, buttonMessages.size());
    EXPECT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX,
              buttonMessages.at(0).componentIndex);
    EXPECT_EQ(buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX,
              buttonMessages.at(1).componentIndex);

    // repeat the update - analog class sends forwarding message again since it's not in charge of filtering it
    _analog._instance.updateAll();
    waitForSignalQuiescence();
    analogButtonMessages = _analogButtonMessages.snapshot();

    EXPECT_EQ(3, analogButtonMessages.size());

    // similar test with the button message type being normal CC
    _analogButtonMessages.clear();
    _buttonMessages.clear();

    ASSERT_TRUE(_databaseAdmin.update(database::Config::Section::button_t::MESSAGE_TYPE, buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, buttons::messageType_t::CONTROL_CHANGE));

    stateChangeRegister(0xFFFF);
    waitForSignalQuiescence();
    analogButtonMessages = _analogButtonMessages.snapshot();
    buttonMessages       = _buttonMessages.snapshot();

    ASSERT_EQ(1, analogButtonMessages.size());
    EXPECT_EQ(BUTTON_INDEX, analogButtonMessages.at(0).componentIndex);
    ASSERT_EQ(1, buttonMessages.size());

    stateChangeRegister(0);
    waitForSignalQuiescence();
    analogButtonMessages = _analogButtonMessages.snapshot();
    buttonMessages       = _buttonMessages.snapshot();

    EXPECT_EQ(2, analogButtonMessages.size());
    EXPECT_EQ(1, buttonMessages.size());
}

#endif
