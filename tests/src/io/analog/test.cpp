#include "framework/Framework.h"
#include "stubs/Analog.h"
#include "stubs/Listener.h"
#include "io/buttons/Buttons.h"

#ifdef ANALOG_SUPPORTED

using namespace IO;

namespace
{
    class AnalogTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_analog._database.init());
            ASSERT_TRUE(_analog._database.factoryReset());
            ASSERT_EQ(0, _analog._database.getPreset());
            ASSERT_TRUE(_analog._instance.init());

            for (int i = 0; i < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); i++)
            {
                ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::ENABLE, i, 1));
                ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::CHANNEL, i, 1));
            }

            MIDIDispatcher.listen(Messaging::eventType_t::ANALOG,
                                  [this](const Messaging::event_t& dispatchMessage)
                                  {
                                      _listener.messageListener(dispatchMessage);
                                  });
        }

        void TearDown() override
        {
            MIDIDispatcher.clear();
            _listener._event.clear();
        }

        void stateChangeRegister(uint16_t value)
        {
            EXPECT_CALL(_analog._hwa, value(_, _))
                .WillRepeatedly(DoAll(SetArgReferee<1>(value),
                                      Return(true)));

            _analog._instance.updateAll();
            _analog.updateLastFilterValue(value);
        }

        Listener   _listener;
        TestAnalog _analog;
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

    EXPECT_EQ((Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS) * 128), _listener._event.size());

    // all received messages should be control change
    for (int i = 0; i < _listener._event.size(); i++)
    {
        EXPECT_EQ(MIDI::messageType_t::CONTROL_CHANGE, _listener._event.at(i).message);
    }

    uint8_t expectedMIDIvalue = 0;

    for (int i = 0; i < _listener._event.size(); i += Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS))
    {
        for (int j = 0; j < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); j++)
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

    EXPECT_EQ((Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS) * 127), _listener._event.size());

    expectedMIDIvalue = 126;

    for (int i = 0; i < _listener._event.size(); i += Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS))
    {
        for (int j = 0; j < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); j++)
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

TEST_F(AnalogTest, PitchBendTest)
{
    // set known state
    for (int i = 0; i < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); i++)
    {
        // configure all analog components as potentiometers with Pitch Bend MIDI message
        ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::TYPE, i, Analog::type_t::PITCH_BEND));
    }

    for (int i = 0; i <= 16383; i++)
    {
        stateChangeRegister(i);
    }

    EXPECT_EQ((Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS) * 16384), _listener._event.size());

    // //all received messages should be pitch bend
    for (int i = 0; i < _listener._event.size(); i++)
    {
        EXPECT_EQ(MIDI::messageType_t::PITCH_BEND, _listener._event.at(i).message);
    }

    uint16_t expectedMIDIvalue = 0;

    for (int i = 0; i < _listener._event.size(); i += Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS))
    {
        for (int j = 0; j < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); j++)
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
    EXPECT_EQ((Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS) * 16383), _listener._event.size());

    expectedMIDIvalue = 16382;

    for (int i = 0; i < _listener._event.size(); i += Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS))
    {
        for (int j = 0; j < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); j++)
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
    for (int i = 0; i < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::INVERT, i, 1));
    }

    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    EXPECT_EQ((Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS) * 128), _listener._event.size());

    // first value should be 127
    // last value should be 0

    uint16_t expectedMIDIvalue = 127;

    for (int i = 0; i < _listener._event.size(); i += Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS))
    {
        for (int j = 0; j < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, _listener._event.at(index).value);
        }

        expectedMIDIvalue--;
    }

    _listener._event.clear();

    // funky setup: set lower limit to 127, upper to 0 while inversion is enabled
    // result should be the same as when default setup is used (no inversion / 0 as lower limit, 127 as upper limit)

    for (int i = 0; i < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::INVERT, i, 1));
        ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::LOWER_LIMIT, i, 127));
        ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::UPPER_LIMIT, i, 0));

        _analog._instance.reset(i);
    }

    // feed all the values again
    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    EXPECT_EQ((Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS) * 128), _listener._event.size());

    expectedMIDIvalue = 0;

    for (int i = 0; i < _listener._event.size(); i += Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS))
    {
        for (int j = 0; j < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, _listener._event.at(index).value);
        }

        expectedMIDIvalue++;
    }

    _listener._event.clear();

    // now disable inversion
    _listener._event.clear();

    for (int i = 0; i < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::INVERT, i, 0));
        _analog._instance.reset(i);
    }

    // feed all the values again
    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    EXPECT_EQ((Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS) * 128), _listener._event.size());

    expectedMIDIvalue = 127;

    for (int i = 0; i < _listener._event.size(); i += Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS))
    {
        for (int j = 0; j < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); j++)
        {
            size_t index = i + j;
            EXPECT_EQ(expectedMIDIvalue, _listener._event.at(index).value);
        }

        expectedMIDIvalue--;
    }
}

TEST_F(AnalogTest, Scaling)
{
    if (!Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS))
    {
        return;
    }

    static constexpr uint32_t SCALED_LOWER = 11;
    static constexpr uint32_t SCALED_UPPER = 100;

    // set known state
    for (int i = 0; i < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::LOWER_LIMIT, i, 0));
        ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::UPPER_LIMIT, i, SCALED_UPPER));
    }

    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    // since the values are scaled, verify that all the messages aren't received
    LOG(INFO) << "Received " << _listener._event.size() << " messages";
    ASSERT_TRUE((Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS) * 128) > _listener._event.size());

    // first value should be 0
    // last value should match the configured scaled value (SCALED_UPPER)
    for (int i = 0; i < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); i++)
    {
        EXPECT_EQ(0, _listener._event.at(i).value);
        EXPECT_EQ(SCALED_UPPER, _listener._event.at(_listener._event.size() - Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS) + i).value);
    }

    // now scale minimum value as well
    for (int i = 0; i < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::LOWER_LIMIT, i, SCALED_LOWER));
    }

    _listener._event.clear();

    for (int i = 0; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    LOG(INFO) << "Received " << _listener._event.size() << " messages";
    ASSERT_TRUE((Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS) * 128) > _listener._event.size());

    for (int i = 0; i < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_listener._event.at(i).value >= SCALED_LOWER);
        ASSERT_TRUE(_listener._event.at(_listener._event.size() - Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS) + i).value <= SCALED_UPPER);
    }

    // now enable inversion
    for (int i = 0; i < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::INVERT, i, 1));
        _analog._instance.reset(i);
    }

    _listener._event.clear();

    for (int i = 1; i <= 127; i++)
    {
        stateChangeRegister(i);
    }

    for (int i = 0; i < Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS); i++)
    {
        ASSERT_TRUE(_listener._event.at(i).value >= SCALED_UPPER);
        ASSERT_TRUE(_listener._event.at(_listener._event.size() - Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS) + i).value <= SCALED_LOWER);
    }
}

TEST_F(AnalogTest, ButtonForwarding)
{
    if (Analog::Collection::size(Analog::GROUP_ANALOG_INPUTS))
    {
        return;
    }

    // configure one analog component to be button type
    static constexpr size_t  BUTTON_INDEX        = 1;
    static constexpr uint8_t BUTTON_MIDI_CHANNEL = 2;
    static constexpr uint8_t BUTTON_VELOCITY     = 100;

    ASSERT_TRUE(_analog._database.update(Database::Config::Section::analog_t::TYPE, BUTTON_INDEX, Analog::type_t::BUTTON) == true);

    // configure button with the same index (+offset) to certain parameters
    ASSERT_TRUE(_analog._database.update(Database::Config::Section::button_t::TYPE, Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, Buttons::type_t::MOMENTARY));
    ASSERT_TRUE(_analog._database.update(Database::Config::Section::button_t::CHANNEL, Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, BUTTON_MIDI_CHANNEL));
    ASSERT_TRUE(_analog._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, Buttons::messageType_t::CONTROL_CHANGE_RESET));
    ASSERT_TRUE(_analog._database.update(Database::Config::Section::button_t::VALUE, Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, BUTTON_VELOCITY));

    std::vector<Messaging::event_t> dispatchMessageAnalogFwd;
    std::vector<Messaging::event_t> dispatchMessageButtons;

    MIDIDispatcher.listen(Messaging::eventType_t::ANALOG_BUTTON,
                          [&](const Messaging::event_t& dispatchMessage)
                          {
                              dispatchMessageAnalogFwd.push_back(dispatchMessage);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::BUTTON,
                          [&](const Messaging::event_t& dispatchMessage)
                          {
                              dispatchMessageButtons.push_back(dispatchMessage);
                          });

    stateChangeRegister(0xFFFF);

    // analog class should just forward the message with the original component index
    // the rest of the message doesn't matter
    EXPECT_EQ(1, dispatchMessageAnalogFwd.size());
    EXPECT_EQ(BUTTON_INDEX, dispatchMessageAnalogFwd.at(0).componentIndex);

    // button class should receive this message and push new button message with index being Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS) + index
    EXPECT_EQ(1, dispatchMessageButtons.size());
    EXPECT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, dispatchMessageButtons.at(0).componentIndex);

    // verify the rest of the message

    // midi index should be the original one - indexing restarts for each new button group (physical buttons / analog / touchscreen)
    EXPECT_EQ(MIDI::messageType_t::CONTROL_CHANGE, dispatchMessageButtons.at(0).message);
    EXPECT_EQ(BUTTON_INDEX, dispatchMessageButtons.at(0).index);
    EXPECT_EQ(BUTTON_VELOCITY, dispatchMessageButtons.at(0).value);
    EXPECT_EQ(BUTTON_MIDI_CHANNEL, dispatchMessageButtons.at(0).channel);

    stateChangeRegister(0);

    // since the button was configured in CONTROL_CHANGE_RESET reset mode,
    // another message should be sent from buttons
    EXPECT_EQ(2, dispatchMessageAnalogFwd.size());
    EXPECT_EQ(BUTTON_INDEX, dispatchMessageAnalogFwd.at(1).componentIndex);
    EXPECT_EQ(2, dispatchMessageButtons.size());
    EXPECT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, dispatchMessageButtons.at(1).componentIndex);
    EXPECT_EQ(MIDI::messageType_t::CONTROL_CHANGE, dispatchMessageButtons.at(1).message);
    EXPECT_EQ(BUTTON_INDEX, dispatchMessageButtons.at(1).index);
    EXPECT_EQ(0, dispatchMessageButtons.at(1).value);
    EXPECT_EQ(BUTTON_MIDI_CHANNEL, dispatchMessageButtons.at(1).channel);

    // repeat the update - analog class sends forwarding message again since it's not in charge of filtering it
    // buttons class shouldn't send anything new since the state of the button hasn't changed
    _analog._instance.updateAll();

    EXPECT_EQ(3, dispatchMessageAnalogFwd.size());
    EXPECT_EQ(2, dispatchMessageButtons.size());

    // similar test with the button message type being normal CC
    dispatchMessageButtons.clear();
    dispatchMessageAnalogFwd.clear();

    ASSERT_TRUE(_analog._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, Buttons::messageType_t::CONTROL_CHANGE));

    stateChangeRegister(0xFFFF);

    EXPECT_EQ(1, dispatchMessageAnalogFwd.size());
    EXPECT_EQ(BUTTON_INDEX, dispatchMessageAnalogFwd.at(0).componentIndex);
    EXPECT_EQ(1, dispatchMessageButtons.size());
    EXPECT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS) + BUTTON_INDEX, dispatchMessageButtons.at(0).componentIndex);
    EXPECT_EQ(MIDI::messageType_t::CONTROL_CHANGE, dispatchMessageButtons.at(0).message);
    EXPECT_EQ(BUTTON_INDEX, dispatchMessageButtons.at(0).index);
    EXPECT_EQ(BUTTON_VELOCITY, dispatchMessageButtons.at(0).value);
    EXPECT_EQ(BUTTON_MIDI_CHANNEL, dispatchMessageButtons.at(0).channel);

    stateChangeRegister(0);

    // this time buttons shouldn't send anything new since standard CC is a latching message
    EXPECT_EQ(2, dispatchMessageAnalogFwd.size());
    EXPECT_EQ(1, dispatchMessageButtons.size());
}

#endif