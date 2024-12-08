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

#ifdef PROJECT_TARGET_SUPPORT_ADC

#include "tests/common.h"
#include "tests/helpers/listener.h"
#include "application/io/analog/builder_test.h"
#include "application/io/buttons/buttons.h"
#include "application/util/configurable/configurable.h"

using namespace io;

namespace
{
    class AnalogTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_analog._databaseAdmin.init());
            ASSERT_TRUE(_analog._databaseAdmin.factoryReset());
            ASSERT_EQ(0, _analog._databaseAdmin.getPreset());
            ASSERT_TRUE(_analog._databaseAdmin.init());
            ASSERT_TRUE(_analog._instance.init());

            for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
            {
                ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::ENABLE, i, 1));
                ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::CHANNEL, i, 1));
            }

            MidiDispatcher.listen(messaging::eventType_t::ANALOG,
                                  [this](const messaging::Event& dispatchMessage)
                                  {
                                      _listener.messageListener(dispatchMessage);
                                  });
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            MidiDispatcher.clear();
            _listener._event.clear();
        }

        void stateChangeRegister(uint16_t value)
        {
            EXPECT_CALL(_analog._hwa, value(_, _))
                .WillRepeatedly(DoAll(SetArgReferee<1>(value),
                                      Return(true)));

            _analog._instance.updateAll();
        }

        test::Listener      _listener;
        analog::BuilderTest _analog;
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

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128), _listener._event.size());

    // all received messages should be control change
    for (size_t i = 0; i < _listener._event.size(); i++)
    {
        EXPECT_EQ(midi::messageType_t::CONTROL_CHANGE, _listener._event.at(i).message);
    }

    uint8_t expectedMIDIvalue = 0;

    for (size_t i = 0; i < _listener._event.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, _listener._event.at(index).value);
        }

        expectedMIDIvalue++;
    }

    // now go backward

    _listener._event.clear();

    for (int i = 126; i >= 0; i--)
    {
        stateChangeRegister(i);
    }

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 127), _listener._event.size());

    expectedMIDIvalue = 126;

    for (size_t i = 0; i < _listener._event.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, _listener._event.at(index).value);
        }

        expectedMIDIvalue--;
    }

    _listener._event.clear();

    // try to feed value larger than 127
    // no response should be received
    stateChangeRegister(10000);

    EXPECT_EQ(0, _listener._event.size());
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

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128), _listener._event.size());

    // all received messages should be NRPN
    for (size_t i = 0; i < _listener._event.size(); i++)
    {
        EXPECT_EQ(midi::messageType_t::NRPN_7BIT, _listener._event.at(i).message);
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

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128), _listener._event.size());

    // all received messages should be NRPN
    for (size_t i = 0; i < _listener._event.size(); i++)
    {
        EXPECT_EQ(midi::messageType_t::NRPN_14BIT, _listener._event.at(i).message);
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

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 16384), _listener._event.size());

    // //all received messages should be pitch bend
    for (size_t i = 0; i < _listener._event.size(); i++)
    {
        EXPECT_EQ(midi::messageType_t::PITCH_BEND, _listener._event.at(i).message);
    }

    uint16_t expectedMIDIvalue = 0;

    for (size_t i = 0; i < _listener._event.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, _listener._event.at(index).value);
        }

        expectedMIDIvalue++;
    }

    // now go backward
    _listener._event.clear();

    for (int i = 16382; i >= 0; i--)
    {
        stateChangeRegister(i);
    }

    // one message less
    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 16383), _listener._event.size());

    expectedMIDIvalue = 16382;

    for (size_t i = 0; i < _listener._event.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, _listener._event.at(index).value);
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

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128), _listener._event.size());

    // first value should be 127
    // last value should be 0

    uint16_t expectedMIDIvalue = 127;

    for (size_t i = 0; i < _listener._event.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, _listener._event.at(index).value);
        }

        expectedMIDIvalue--;
    }

    _listener._event.clear();

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

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128), _listener._event.size());

    expectedMIDIvalue = 0;

    for (size_t i = 0; i < _listener._event.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, _listener._event.at(index).value);
        }

        expectedMIDIvalue++;
    }

    _listener._event.clear();

    // now disable inversion
    _listener._event.clear();

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

    EXPECT_EQ((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128), _listener._event.size());

    expectedMIDIvalue = 127;

    for (size_t i = 0; i < _listener._event.size(); i += analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS))
    {
        for (size_t j = 0; j < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, _listener._event.at(index).value);
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

    // since the values are scaled, verify that all the messages aren't received
    LOG(INFO) << "Received " << _listener._event.size() << " messages";
    ASSERT_TRUE((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128) > _listener._event.size());

    // first value should be 0
    // last value should match the configured scaled value (SCALED_UPPER)
    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        EXPECT_EQ(0, _listener._event.at(i).value);
        EXPECT_EQ(SCALED_UPPER, _listener._event.at(_listener._event.size() - analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) + i).value);
    }

    // now scale minimum value as well
    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::LOWER_LIMIT, i, SCALED_LOWER));
    }

    _listener._event.clear();

    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    LOG(INFO) << "Received " << _listener._event.size() << " messages";
    ASSERT_TRUE((analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) * 128) > _listener._event.size());

    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_listener._event.at(i).value >= SCALED_LOWER);
        ASSERT_TRUE(_listener._event.at(_listener._event.size() - analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) + i).value <= SCALED_UPPER);
    }

    // now enable inversion
    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::analog_t::INVERT, i, 1));
        _analog._instance.reset(i);
    }

    _listener._event.clear();

    for (int i = 1; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    for (size_t i = 0; i < analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_listener._event.at(i).value >= SCALED_UPPER);
        ASSERT_TRUE(_listener._event.at(_listener._event.size() - analog::Collection::SIZE(analog::GROUP_ANALOG_INPUTS) + i).value <= SCALED_LOWER);
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
    ASSERT_TRUE(_analog._databaseAdmin.update(database::Config::Section::button_t::TYPE, buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, buttons::type_t::MOMENTARY));
    ASSERT_TRUE(_analog._databaseAdmin.update(database::Config::Section::button_t::CHANNEL, buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, BUTTON_MIDI_CHANNEL));
    ASSERT_TRUE(_analog._databaseAdmin.update(database::Config::Section::button_t::MESSAGE_TYPE, buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, buttons::messageType_t::CONTROL_CHANGE_RESET));
    ASSERT_TRUE(_analog._databaseAdmin.update(database::Config::Section::button_t::VALUE, buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, BUTTON_VELOCITY));

    std::vector<messaging::Event> dispatchMessageAnalogFwd;

    MidiDispatcher.listen(messaging::eventType_t::ANALOG_BUTTON,
                          [&](const messaging::Event& dispatchMessage)
                          {
                              dispatchMessageAnalogFwd.push_back(dispatchMessage);
                          });

    stateChangeRegister(0xFFFF);

    // analog class should just forward the message with the original component index
    // the rest of the message doesn't matter
    ASSERT_EQ(1, dispatchMessageAnalogFwd.size());
    EXPECT_EQ(BUTTON_INDEX, dispatchMessageAnalogFwd.at(0).componentIndex);

    stateChangeRegister(0);

    // since the button was configured in CONTROL_CHANGE_RESET reset mode,
    // another message should be sent from buttons
    ASSERT_EQ(2, dispatchMessageAnalogFwd.size());
    EXPECT_EQ(BUTTON_INDEX, dispatchMessageAnalogFwd.at(1).componentIndex);

    // repeat the update - analog class sends forwarding message again since it's not in charge of filtering it
    _analog._instance.updateAll();

    EXPECT_EQ(3, dispatchMessageAnalogFwd.size());

    // similar test with the button message type being normal CC
    dispatchMessageAnalogFwd.clear();

    ASSERT_TRUE(_analog._databaseAdmin.update(database::Config::Section::button_t::MESSAGE_TYPE, buttons::Collection::SIZE(buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, buttons::messageType_t::CONTROL_CHANGE));

    stateChangeRegister(0xFFFF);

    ASSERT_EQ(1, dispatchMessageAnalogFwd.size());
    EXPECT_EQ(BUTTON_INDEX, dispatchMessageAnalogFwd.at(0).componentIndex);

    stateChangeRegister(0);

    EXPECT_EQ(2, dispatchMessageAnalogFwd.size());
}

#endif