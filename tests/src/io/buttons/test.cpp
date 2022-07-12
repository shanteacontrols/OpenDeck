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
            ASSERT_TRUE(_buttons._databaseAdmin.init());
            ASSERT_TRUE(_buttons._databaseAdmin.factoryReset());
            ASSERT_EQ(0, _buttons._databaseAdmin.getPreset());
            ASSERT_TRUE(_buttons._databaseAdmin.init());

            for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
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

        void stateChangeRegisterAll(bool state)
        {
            _listener._event.clear();

            EXPECT_CALL(_buttons._hwa, state(_, _, _))
                .WillRepeatedly(DoAll(SetArgReferee<1>(1),
                                      SetArgReferee<2>(state),
                                      Return(true)));

            _buttons._instance.updateAll();
        }

        void stateChangeRegisterSingle(size_t index, bool state)
        {
            _listener._event.clear();

            EXPECT_CALL(_buttons._hwa, state(_, _, _))
                .WillRepeatedly(DoAll(SetArgReferee<1>(1),
                                      SetArgReferee<2>(state),
                                      Return(true)));

            _buttons._instance.updateSingle(index);
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
        stateChangeRegisterAll(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyValue(true);

        // simulate button release
        stateChangeRegisterAll(false);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyValue(false);

        // try with the latching mode
        for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::LATCHING));
        }

        stateChangeRegisterAll(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyValue(true);

        // nothing should happen on release
        stateChangeRegisterAll(false);
        ASSERT_EQ(0, _listener._event.size());

        // press again, new messages should arrive
        stateChangeRegisterAll(true);
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

    auto testProgramChange = [&](uint8_t channel)
    {
        // set known state
        for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::MOMENTARY));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, i, Buttons::messageType_t::PROGRAM_CHANGE));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::CHANNEL, i, channel));

            _buttons._instance.reset(i);
        }

        auto verifyMessage = [&]()
        {
            // verify all received messages are program change
            for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
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
        stateChangeRegisterAll(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyMessage();

        // program change shouldn't be sent on release
        stateChangeRegisterAll(false);
        ASSERT_EQ(0, _listener._event.size());

        // repeat the entire test again, but with buttons configured as latching types
        // behaviour should be the same
        for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::LATCHING));
        }

        stateChangeRegisterAll(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());
        verifyMessage();

        stateChangeRegisterAll(false);
        ASSERT_EQ(0, _listener._event.size());
    };

    // test for all channels
    for (int i = 1; i <= 16; i++)
    {
        testProgramChange(i);
    }

    // test PROGRAM_CHANGE_INC/PROGRAM_CHANGE_DEC
    _buttons._databaseAdmin.factoryReset();
    stateChangeRegisterAll(false);

    auto configurePCbutton = [&](size_t index, uint8_t channel, bool increase)
    {
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, index, Buttons::type_t::MOMENTARY));
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, index, increase ? Buttons::messageType_t::PROGRAM_CHANGE_INC : Buttons::messageType_t::PROGRAM_CHANGE_DEC));
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::CHANNEL, index, channel));

        for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
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
    _buttons._databaseAdmin.factoryReset();

    if (Buttons::Collection::size(IO::Buttons::GROUP_DIGITAL_INPUTS) < 4)
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
    _buttons._databaseAdmin.factoryReset();

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
        if (_listener._event.at(i).message == MIDI::messageType_t::PROGRAM_CHANGE)
        {
            pcCounter++;
        }
    }

    ASSERT_EQ(2, pcCounter);

    stateChangeRegisterAll(false);

    // revert all buttons to default
    _buttons._databaseAdmin.factoryReset();

    configurePCbutton(0, CHANNEL, true);

    stateChangeRegisterAll(true);
    // button 0 should increase the last value by 1
    verifyProgramChange(0, CHANNEL, 1);
    stateChangeRegisterAll(false);
}

TEST_F(ButtonsTest, ProgramChangeWithOffset)
{
    if (Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS) < 3)
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
        LOG(INFO) << "Verifiyng received MIDI Program Change message";
        LOG(INFO) << "Expecting channel " << static_cast<int>(channel) << ", program " << static_cast<int>(program);

        ASSERT_EQ(MIDI::messageType_t::PROGRAM_CHANGE, _listener._event.at(index).message);

        // program change value should always be set to 0
        ASSERT_EQ(0, _listener._event.at(index).value);

        // verify channel
        ASSERT_EQ(channel, _listener._event.at(index).channel);

        ASSERT_EQ(program, _listener._event.at(index).index);
    };

    auto toggleProgramChangeIncButton = [&]()
    {
        LOG(INFO) << "Incrementing MIDI Program Change offset by " << OFFSET_INC_DEC_AMOUNT;
        LOG(INFO) << "Current offset is " << static_cast<int>(MIDIProgram.offset());
        stateChangeRegisterSingle(BUTTON_INDEX_OFFSET_INC, true);
        ASSERT_TRUE(MIDIProgram.offset() >= 0);
        ASSERT_TRUE(MIDIProgram.offset() <= 127);
        stateChangeRegisterSingle(BUTTON_INDEX_OFFSET_INC, false);
        LOG(INFO) << "Offset after change is " << static_cast<int>(MIDIProgram.offset());
    };

    auto toggleProgramChangeDecButton = [&]()
    {
        LOG(INFO) << "Decrementing MIDI Program Change offset by " << OFFSET_INC_DEC_AMOUNT;
        LOG(INFO) << "Current offset is " << static_cast<int>(MIDIProgram.offset());
        stateChangeRegisterSingle(BUTTON_INDEX_OFFSET_DEC, true);
        ASSERT_TRUE(MIDIProgram.offset() >= 0);
        ASSERT_TRUE(MIDIProgram.offset() <= 127);
        stateChangeRegisterSingle(BUTTON_INDEX_OFFSET_DEC, false);
        LOG(INFO) << "Offset after change is " << static_cast<int>(MIDIProgram.offset());
    };

    auto toggleProgramChangeButton = [&]()
    {
        LOG(INFO) << "Toggling program change button";

        stateChangeRegisterSingle(BUTTON_INDEX_PROGRAM, true);
        ASSERT_EQ(1, _listener._event.size());
        verifyProgramChange(0, PROGRAM_CHANGE_CHANNEL, BUTTON_INDEX_PROGRAM + MIDIProgram.offset());
        stateChangeRegisterSingle(BUTTON_INDEX_PROGRAM, false);
    };

    // set known state
    ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, BUTTON_INDEX_OFFSET_DEC, Buttons::messageType_t::PROGRAM_CHANGE_OFFSET_DEC));
    ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::VALUE, BUTTON_INDEX_OFFSET_DEC, OFFSET_INC_DEC_AMOUNT));
    ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, BUTTON_INDEX_OFFSET_INC, Buttons::messageType_t::PROGRAM_CHANGE_OFFSET_INC));
    ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::VALUE, BUTTON_INDEX_OFFSET_INC, OFFSET_INC_DEC_AMOUNT));
    ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, BUTTON_INDEX_PROGRAM, Buttons::messageType_t::PROGRAM_CHANGE));

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
    while (MIDIProgram.offset() != 127)
    {
        toggleProgramChangeIncButton();
    }

    // try again - nothing should change
    toggleProgramChangeIncButton();
    ASSERT_EQ(127, MIDIProgram.offset());

    // start decrementing offset
    for (size_t i = 0; i < 50; i++)
    {
        toggleProgramChangeDecButton();
    }

    toggleProgramChangeButton();

    LOG(INFO) << "Setting the offset to 0 manually";
    MIDIProgram.setOffset(0);
    ASSERT_EQ(0, MIDIProgram.offset());

    toggleProgramChangeButton();

    // decrement again - verify that nothing changes
    toggleProgramChangeDecButton();
    ASSERT_EQ(0, MIDIProgram.offset());
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
        for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::MOMENTARY));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, i, Buttons::messageType_t::CONTROL_CHANGE));
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::VALUE, i, controlValue));

            _buttons._instance.reset(i);
        }

        auto verifyMessage = [&](uint8_t midiValue)
        {
            // verify all received messages are control change
            for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
            {
                ASSERT_EQ(MIDI::messageType_t::CONTROL_CHANGE, _listener._event.at(i).message);
                ASSERT_EQ(midiValue, _listener._event.at(i).value);
                ASSERT_EQ(1, _listener._event.at(i).channel);
                ASSERT_EQ(i, _listener._event.at(i).index);
            }
        };

        // simulate button press
        stateChangeRegisterAll(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(controlValue);

        // no messages should be sent on release
        stateChangeRegisterAll(false);
        ASSERT_EQ(0, _listener._event.size());

        // change to latching type
        // behaviour should be the same

        for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::LATCHING));
        }

        stateChangeRegisterAll(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(controlValue);

        stateChangeRegisterAll(false);
        ASSERT_EQ(0, _listener._event.size());

        // change type to control change with 0 on reset, momentary mode
        // this means CC value 0 should be sent on release

        for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::MOMENTARY));
        }

        for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, i, Buttons::messageType_t::CONTROL_CHANGE_RESET));
        }

        stateChangeRegisterAll(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(controlValue);

        stateChangeRegisterAll(false);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(0);

        // same test again, but in latching mode
        // now, on press, messages should be sent
        // on release, nothing should happen
        // on second press reset should be sent (CC with value 0)

        for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
        {
            ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::LATCHING));
        }

        stateChangeRegisterAll(true);
        ASSERT_EQ(Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS), _listener._event.size());

        verifyMessage(controlValue);

        stateChangeRegisterAll(false);
        ASSERT_EQ(0, _listener._event.size());

        stateChangeRegisterAll(true);
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

    for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
    {
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, i, Buttons::messageType_t::NONE));

        _buttons._instance.reset(i);
    }

    stateChangeRegisterAll(true);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegisterAll(false);
    ASSERT_EQ(0, _listener._event.size());

    for (size_t i = 0; i < Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS); i++)
    {
        ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::TYPE, i, Buttons::type_t::LATCHING));
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
    if (_buttons._databaseAdmin.getSupportedPresets() <= 1)
    {
        return;
    }

    if (!Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS))
    {
        return;
    }

    MIDIDispatcher.listen(Messaging::eventType_t::SYSTEM,
                          [this](const Messaging::event_t& dispatchMessage)
                          {
                              _listener.messageListener(dispatchMessage);
                          });

    // configure one button to change preset
    static constexpr size_t BUTTON_INDEX = 0;

    ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MIDI_ID, BUTTON_INDEX, 1));
    ASSERT_TRUE(_buttons._database.update(Database::Config::Section::button_t::MESSAGE_TYPE, BUTTON_INDEX, Buttons::messageType_t::PRESET_CHANGE));
    _buttons._instance.reset(BUTTON_INDEX);

    // simulate button press
    stateChangeRegisterSingle(BUTTON_INDEX, true);

    ASSERT_EQ(1, _listener._event.size());
    ASSERT_EQ(Messaging::systemMessage_t::PRESET_CHANGE_DIRECT_REQ, _listener._event.at(0).systemMessage);

    // verify that no new events are generated on button release
    stateChangeRegisterSingle(BUTTON_INDEX, false);
    ASSERT_EQ(0, _listener._event.size());
}

#endif