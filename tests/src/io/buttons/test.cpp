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
                ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::MOMENTARY));
                ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, i, Buttons::messageType_t::NOTE));
                ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::VALUE, i, 127));

                _buttons._instance.reset(i);
            }

            MIDIDispatcher.listen(Messaging::eventType_t::BUTTON,
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

    auto test = [&](uint8_t channel, uint8_t velocity)
    {
        // set known state
        for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::MOMENTARY));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, i, Buttons::messageType_t::NOTE));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::VALUE, i, velocity));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::CHANNEL, i, channel));

            _buttons._instance.reset(i);
        }

        auto verifyValue = [&](bool state)
        {
            // verify all received messages
            for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
            {
                if (state)
                {
                    ASSERT_EQ(MIDI::messageType_t::NOTE_ON, _listener._event.at(i).message);
                }
                else
                {
                    ASSERT_EQ(MIDI::messageType_t::NOTE_OFF, _listener._event.at(i).message);
                }

                ASSERT_EQ(state ? velocity : 0, _listener._event.at(i).value);
                ASSERT_EQ(channel, _listener._event.at(i).channel);

                // also verify MIDI ID
                // it should be equal to button ID by default
                ASSERT_EQ(i, _listener._event.at(i).index);
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
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::LATCHING));
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

    auto tesprogramChange = [&](uint8_t channel)
    {
        // set known state
        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::MOMENTARY));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, i, Buttons::messageType_t::PROGRAM_CHANGE));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::CHANNEL, i, channel));

            _buttons._instance.reset(i);
        }

        auto verifyMessage = [&]()
        {
            // verify all received messages are program change
            for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
            {
                ASSERT_EQ(MIDI::messageType_t::PROGRAM_CHANGE, _listener._event.at(i).message);

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
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::LATCHING));
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

    // test PROGRAM_CHANGE_INC/PROGRAM_CHANGE_DEC
    _buttons._database.factoryReset();
    stateChangeRegister(false);

    auto configurePCbutton = [&](size_t index, uint8_t channel, bool increase)
    {
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, index, Buttons::type_t::MOMENTARY));
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, index, increase ? Buttons::messageType_t::PROGRAM_CHANGE_INC : Buttons::messageType_t::PROGRAM_CHANGE_DEC));
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::CHANNEL, index, channel));

        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            _buttons._instance.reset(i);
        }
    };

    auto verifyProgramChange = [&](size_t index, uint8_t channel, uint8_t program)
    {
        ASSERT_EQ(MIDI::messageType_t::PROGRAM_CHANGE, _listener._event.at(index).message);

        // program change value should always be set to 0
        ASSERT_EQ(0, _listener._event.at(index).value);

        // verify channel
        ASSERT_EQ(channel, _listener._event.at(index).channel);

        ASSERT_EQ(program, _listener._event.at(index).index);
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

    // configure some other button to PROGRAM_CHANGE_INC
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

    // configure another button to PROGRAM_CHANGE_INC, but on other channel
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

    // now configure button 0 for PROGRAM_CHANGE_DEC
    configurePCbutton(0, CHANNEL, false);

    stateChangeRegister(true);
    // program change should decrease by 1
    verifyProgramChange(0, CHANNEL, 8);
    stateChangeRegister(false);

    stateChangeRegister(true);
    // program change should decrease by 1 again
    verifyProgramChange(0, CHANNEL, 7);
    stateChangeRegister(false);

    // configure another button for PROGRAM_CHANGE_DEC
    configurePCbutton(1, CHANNEL, false);

    stateChangeRegister(true);
    // button 0 should decrease the value by 1
    verifyProgramChange(0, CHANNEL, 6);
    // button 1 should decrease it again
    verifyProgramChange(1, CHANNEL, 5);
    stateChangeRegister(false);

    // configure another button for PROGRAM_CHANGE_DEC
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
        if (_listener._event.at(i).message == MIDI::messageType_t::PROGRAM_CHANGE)
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

    auto controlChangeTest = [&](uint8_t controlValue)
    {
        // set known state
        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::MOMENTARY));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, i, Buttons::messageType_t::CONTROL_CHANGE));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::VALUE, i, controlValue));

            _buttons._instance.reset(i);
        }

        auto verifyMessage = [&](uint8_t midiValue)
        {
            // verify all received messages are control change
            for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
            {
                ASSERT_EQ(MIDI::messageType_t::CONTROL_CHANGE, _listener._event.at(i).message);
                ASSERT_EQ(midiValue, _listener._event.at(i).value);
                ASSERT_EQ(1, _listener._event.at(i).channel);
                ASSERT_EQ(i, _listener._event.at(i).index);
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
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::LATCHING));
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
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::MOMENTARY));
        }

        for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, i, Buttons::messageType_t::CONTROL_CHANGE_RESET));
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
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::LATCHING));
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

    // configure all buttons to messageType_t::NONE so that messages aren't sent

    for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
    {
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, i, Buttons::messageType_t::NONE));

        _buttons._instance.reset(i);
    }

    stateChangeRegister(true);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(false);
    ASSERT_EQ(0, _listener._event.size());

    for (int i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
    {
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::LATCHING));
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

    ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MIDI_ID, BUTTON_INDEX, 1));
    ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, BUTTON_INDEX, Buttons::messageType_t::PRESET_OPEN_DECK));
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