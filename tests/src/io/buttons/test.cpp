#include "framework/Framework.h"
#include "stubs/Buttons.h"
#include "stubs/Listener.h"

#ifdef BUTTONS_SUPPORTED

using namespace IO;

namespace
{
    class ButtonsTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_buttons._database.init());
            ASSERT_TRUE(_buttons._database.factoryReset());
            ASSERT_EQ(0, _buttons._database.getPreset());
            ASSERT_TRUE(_buttons._instance.init());

            for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
            {
                ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::type, i, Buttons::type_t::momentary));
                ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::midiMessage, i, Buttons::messageType_t::note));
                ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::velocity, i, 127));

                _buttons._instance.reset(i);
            }

            MIDIDispatcher.listen(Messaging::eventSource_t::buttons,
                                  Messaging::listenType_t::nonFwd,
                                  [this](const Messaging::event_t& dispatchMessage) {
                                      _listener.messageListener(dispatchMessage);
                                  });
        }

        void TearDown() override
        {
            MIDIDispatcher.clear();
            _listener._event.clear();
        }

        void stateChangeRegister(bool state)
        {
            _listener._event.clear();

            EXPECT_CALL(_buttons._hwa, state(_, _, _))
                .WillRepeatedly(DoAll(SetArgReferee<1>(1),
                                      SetArgReferee<2>(state),
                                      Return(true)));

            _buttons._instance.updateAll();
        }

        Listener    _listener;
        TestButtons _buttons;
    };
}    // namespace

TEST_F(ButtonsTest, Note)
{
    if (!Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS))
    {
        return;
    }

    auto test = [&](uint8_t channel, uint8_t velocity) {
        // set known state
        for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::type, i, Buttons::type_t::momentary));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::midiMessage, i, Buttons::messageType_t::note));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::velocity, i, velocity));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::midiChannel, i, channel));

            _buttons._instance.reset(i);
        }

        auto verifyValue = [&](bool state) {
            // verify all received messages
            for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
            {
                if (state)
                {
                    ASSERT_EQ(MIDI::messageType_t::noteOn, _listener._event.at(i).message);
                }
                else
                {
                    ASSERT_EQ(MIDI::messageType_t::noteOff, _listener._event.at(i).message);
                }

                ASSERT_EQ(state ? velocity : 0, _listener._event.at(i).midiValue);
                ASSERT_EQ(channel, _listener._event.at(i).midiChannel);

                // also verify MIDI ID
                // it should be equal to button ID by default
                ASSERT_EQ(i, _listener._event.at(i).midiIndex);
            }
        };

        // simulate button press
        stateChangeRegister(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyValue(true);

        // simulate button release
        stateChangeRegister(false);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyValue(false);

        // try with the latching mode
        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::type, i, Buttons::type_t::latching));
        }

        stateChangeRegister(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyValue(true);

        // nothing should happen on release
        stateChangeRegister(false);
        ASSERT_EQ(0, _listener._event.size());

        // press again, new messages should arrive
        stateChangeRegister(true);
        verifyValue(false);
    };

    // test for all velocity/channel values
    for (int channel = 1; channel <= 16; channel++)
    {
        for (int velocity = 1; velocity <= 127; velocity++)
        {
            test(channel, velocity);
        }
    }
}

TEST_F(ButtonsTest, ProgramChange)
{
    if (!Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS))
    {
        return;
    }

    auto tesprogramChange = [&](uint8_t channel) {
        // set known state
        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::type, i, Buttons::type_t::momentary));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::midiMessage, i, Buttons::messageType_t::programChange));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::midiChannel, i, channel));

            _buttons._instance.reset(i);
        }

        auto verifyMessage = [&]() {
            // verify all received messages are program change
            for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
            {
                ASSERT_EQ(MIDI::messageType_t::programChange, _listener._event.at(i).message);

                // program change value should always be set to 0
                ASSERT_EQ(0, _listener._event.at(i).midiValue);

                // verify channel
                ASSERT_EQ(channel, _listener._event.at(i).midiChannel);

                // also verify MIDI ID/program
                // it should be equal to button ID by default
                ASSERT_EQ(i, _listener._event.at(i).midiIndex);
            }
        };

        // simulate button press
        stateChangeRegister(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyMessage();

        // program change shouldn't be sent on release
        stateChangeRegister(false);
        ASSERT_EQ(0, _listener._event.size());

        // repeat the entire test again, but with buttons configured as latching types
        // behaviour should be the same
        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::type, i, Buttons::type_t::latching));
        }

        stateChangeRegister(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyMessage();

        stateChangeRegister(false);
        ASSERT_EQ(0, _listener._event.size());
    };

    // test for all channels
    for (int i = 1; i <= 16; i++)
    {
        tesprogramChange(i);
    }

    // test programChangeInc/programChangeDec
    _buttons._database.factoryReset();
    stateChangeRegister(false);

    auto configurePCbutton = [&](size_t index, uint8_t channel, bool increase) {
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::type, index, Buttons::type_t::momentary));
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::midiMessage, index, increase ? Buttons::messageType_t::programChangeInc : Buttons::messageType_t::programChangeDec));
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::midiChannel, index, channel));

        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            _buttons._instance.reset(i);
        }
    };

    auto verifyProgramChange = [&](size_t index, uint8_t channel, uint8_t program) {
        ASSERT_EQ(MIDI::messageType_t::programChange, _listener._event.at(index).message);

        // program change value should always be set to 0
        ASSERT_EQ(0, _listener._event.at(index).midiValue);

        // verify channel
        ASSERT_EQ(channel, _listener._event.at(index).midiChannel);

        ASSERT_EQ(program, _listener._event.at(index).midiIndex);
    };

    static constexpr uint8_t CHANNEL = 1;

    configurePCbutton(0, CHANNEL, true);

    // verify that the received program change was 1 for button 0
    stateChangeRegister(true);
    verifyProgramChange(0, CHANNEL, 1);
    stateChangeRegister(false);

    // after this, verify that the received program change was 2 for button 0
    stateChangeRegister(true);
    verifyProgramChange(0, CHANNEL, 2);
    stateChangeRegister(false);

    // after this, verify that the received program change was 3 for button 0
    stateChangeRegister(true);
    verifyProgramChange(0, CHANNEL, 3);
    stateChangeRegister(false);

    // now, revert all buttons back to default
    _buttons._database.factoryReset();

    if (Buttons::Collection::size() < 2)
    {
        return;
    }

    // configure some other button to programChangeInc
    configurePCbutton(4, CHANNEL, true);

    // verify that the program change is continuing to increase
    stateChangeRegister(true);
    verifyProgramChange(4, CHANNEL, 4);
    stateChangeRegister(false);

    stateChangeRegister(true);
    verifyProgramChange(4, CHANNEL, 5);
    stateChangeRegister(false);

    // now configure two buttons to send program change/inc
    configurePCbutton(0, CHANNEL, true);

    stateChangeRegister(true);
    // program change should be increased by 1, first by button 0
    verifyProgramChange(0, CHANNEL, 6);
    // then by button 4
    verifyProgramChange(4, CHANNEL, 7);
    stateChangeRegister(false);

    // configure another button to programChangeInc, but on other channel
    configurePCbutton(1, 4, true);

    stateChangeRegister(true);
    // program change should be increased by 1, first by button 0
    verifyProgramChange(0, CHANNEL, 8);
    // then by button 4
    verifyProgramChange(4, CHANNEL, 9);
    // program change should be sent on channel 4 by button 1
    verifyProgramChange(1, 4, 1);
    stateChangeRegister(false);

    // revert to default again
    _buttons._database.factoryReset();

    // now configure button 0 for programChangeDec
    configurePCbutton(0, CHANNEL, false);

    stateChangeRegister(true);
    // program change should decrease by 1
    verifyProgramChange(0, CHANNEL, 8);
    stateChangeRegister(false);

    stateChangeRegister(true);
    // program change should decrease by 1 again
    verifyProgramChange(0, CHANNEL, 7);
    stateChangeRegister(false);

    // configure another button for programChangeDec
    configurePCbutton(1, CHANNEL, false);

    stateChangeRegister(true);
    // button 0 should decrease the value by 1
    verifyProgramChange(0, CHANNEL, 6);
    // button 1 should decrease it again
    verifyProgramChange(1, CHANNEL, 5);
    stateChangeRegister(false);

    // configure another button for programChangeDec
    configurePCbutton(2, CHANNEL, false);

    stateChangeRegister(true);
    // button 0 should decrease the value by 1
    verifyProgramChange(0, CHANNEL, 4);
    // button 1 should decrease it again
    verifyProgramChange(1, CHANNEL, 3);
    // button 2 should decrease it again
    verifyProgramChange(2, CHANNEL, 2);
    stateChangeRegister(false);

    // reset all received messages first
    _listener._event.clear();

    // only two program change messages should be sent
    // program change value is 0 after the second button decreases it
    // once the value is 0 no further messages should be sent in dec mode

    stateChangeRegister(true);
    // button 0 should decrease the value by 1
    verifyProgramChange(0, CHANNEL, 1);
    // button 1 should decrease it again
    verifyProgramChange(1, CHANNEL, 0);

    // verify that only two program change messages have been received
    uint8_t pcCounter = 0;

    for (int i = 0; i < _listener._event.size(); i++)
    {
        if (_listener._event.at(i).message == MIDI::messageType_t::programChange)
        {
            pcCounter++;
        }
    }

    ASSERT_EQ(2, pcCounter);

    stateChangeRegister(false);

    // revert all buttons to default
    _buttons._database.factoryReset();

    configurePCbutton(0, CHANNEL, true);

    stateChangeRegister(true);
    // button 0 should increase the last value by 1
    verifyProgramChange(0, CHANNEL, 1);
    stateChangeRegister(false);
}

TEST_F(ButtonsTest, ControlChange)
{
    if (!Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS))
    {
        return;
    }

    auto controlChangeTest = [&](uint8_t controlValue) {
        // set known state
        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::type, i, Buttons::type_t::momentary));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::midiMessage, i, Buttons::messageType_t::controlChange));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::velocity, i, controlValue));

            _buttons._instance.reset(i);
        }

        auto verifyMessage = [&](uint8_t midiValue) {
            // verify all received messages are control change
            for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
            {
                ASSERT_EQ(MIDI::messageType_t::controlChange, _listener._event.at(i).message);
                ASSERT_EQ(midiValue, _listener._event.at(i).midiValue);
                ASSERT_EQ(1, _listener._event.at(i).midiChannel);
                ASSERT_EQ(i, _listener._event.at(i).midiIndex);
            }
        };

        // simulate button press
        stateChangeRegister(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(controlValue);

        // no messages should be sent on release
        stateChangeRegister(false);
        ASSERT_EQ(0, _listener._event.size());

        // change to latching type
        // behaviour should be the same

        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::type, i, Buttons::type_t::latching));
        }

        stateChangeRegister(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(controlValue);

        stateChangeRegister(false);
        ASSERT_EQ(0, _listener._event.size());

        // change type to control change with 0 on reset, momentary mode
        // this means CC value 0 should be sent on release

        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::type, i, Buttons::type_t::momentary));
        }

        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::midiMessage, i, Buttons::messageType_t::controlChangeReset));
        }

        stateChangeRegister(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(controlValue);

        stateChangeRegister(false);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(0);

        // same test again, but in latching mode
        // now, on press, messages should be sent
        // on release, nothing should happen
        // on second press reset should be sent (CC with value 0)

        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::type, i, Buttons::type_t::latching));
        }

        stateChangeRegister(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(controlValue);

        stateChangeRegister(false);
        ASSERT_EQ(0, _listener._event.size());

        stateChangeRegister(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(0);
    };

    // verify with all control values
    // value 0 is normally blocked to configure for users (via sysex)
    for (int i = 1; i < 128; i++)
    {
        controlChangeTest(i);
    }
}

TEST_F(ButtonsTest, NoMessages)
{
    if (!Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS))
    {
        return;
    }

    // configure all buttons to messageType_t::none so that messages aren't sent

    for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
    {
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::midiMessage, i, Buttons::messageType_t::none));

        _buttons._instance.reset(i);
    }

    stateChangeRegister(true);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(false);
    ASSERT_EQ(0, _listener._event.size());

    for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
    {
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::type, i, Buttons::type_t::latching));
    }

    stateChangeRegister(true);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(false);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(true);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(false);
    ASSERT_EQ(0, _listener._event.size());
}

TEST_F(ButtonsTest, PresetChange)
{
    if (_buttons._database.getSupportedPresets() <= 1)
    {
        return;
    }

    if (!Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS))
    {
        return;
    }

    class DBhandlers : public Database::Instance::Handlers
    {
        public:
        DBhandlers() = default;

        void presetChange(uint8_t preset) override
        {
            _preset = preset;
        }

        void factoryResetStart() override
        {
        }

        void factoryResetDone() override
        {
        }

        void initialized() override
        {
        }

        uint8_t _preset = 0;
    } _dbHandlers;

    _buttons._database.registerHandlers(_dbHandlers);

    // configure one button to change preset
    static constexpr size_t BUTTON_INDEX = 0;

    ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::midiID, BUTTON_INDEX, 1));
    ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::midiMessage, BUTTON_INDEX, Buttons::messageType_t::presetOpenDeck));
    _buttons._instance.reset(BUTTON_INDEX);

    // simulate button press
    stateChangeRegister(true);
    ASSERT_EQ(1, _dbHandlers._preset);
    _dbHandlers._preset = 0;

    // verify that preset is unchanged after the button is released
    stateChangeRegister(false);
    ASSERT_EQ(0, _dbHandlers._preset);
}

#endif