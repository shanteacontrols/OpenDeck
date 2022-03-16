#include "framework/Framework.h"
#include "stubs/Encoders.h"
#include "stubs/Listener.h"

#ifdef ENCODERS_SUPPORTED

using namespace IO;

namespace
{
    class EncodersTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_encoders._database.init());
            ASSERT_TRUE(_encoders._database.factoryReset());
            ASSERT_EQ(0, _encoders._database.getPreset());

            // set known state
            for (int i = 0; i < Encoders::Collection::size(); i++)
            {
                ASSERT_TRUE(_encoders._database.update(Database::Config::Section::encoder_t::enable, i, 1));
                ASSERT_TRUE(_encoders._database.update(Database::Config::Section::encoder_t::invert, i, 0));
                ASSERT_TRUE(_encoders._database.update(Database::Config::Section::encoder_t::mode, i, Encoders::type_t::controlChange7Fh01h));
                ASSERT_TRUE(_encoders._database.update(Database::Config::Section::encoder_t::pulsesPerStep, i, 1));
            }

            MIDIDispatcher.listen(Messaging::eventType_t::encoder,
                                  [this](const Messaging::event_t& dispatchMessage) {
                                      _listener.messageListener(dispatchMessage);
                                  });
        }

        void TearDown() override
        {
            MIDIDispatcher.clear();
            _listener._event.clear();
        }

        void stateChangeRegister(uint8_t state)
        {
            _listener._event.clear();

            EXPECT_CALL(_encoders._hwa, state(_, _, _))
                .WillRepeatedly(DoAll(SetArgReferee<1>(1),
                                      SetArgReferee<2>(state),
                                      Return(true)));

            _encoders._instance.updateAll();
        }

        Listener     _listener;
        TestEncoders _encoders;
    };
}    // namespace

TEST_F(EncodersTest, StateDecoding)
{
    auto verifyValue = [&](MIDI::messageType_t message, uint16_t value) {
        for (int i = 0; i < Encoders::Collection::size(); i++)
        {
            ASSERT_EQ(message, _listener._event.at(i).message);
            ASSERT_EQ(value, _listener._event.at(i).midiValue);
        }
    };

    // test expected permutations for ccw and cw directions

    // clockwise: 00, 10, 11, 01

    stateChangeRegister(0b00);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b10);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

    stateChangeRegister(0b11);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

    stateChangeRegister(0b01);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

    stateChangeRegister(0b10);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b11);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

    stateChangeRegister(0b01);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

    stateChangeRegister(0b00);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

    stateChangeRegister(0b11);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b01);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

    stateChangeRegister(0b00);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

    stateChangeRegister(0b10);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

    stateChangeRegister(0b01);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b00);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

    stateChangeRegister(0b10);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

    stateChangeRegister(0b11);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

    // counter-clockwise: 00, 01, 11, 10

    stateChangeRegister(0b00);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b01);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);

    stateChangeRegister(0b11);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);

    stateChangeRegister(0b10);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);

    stateChangeRegister(0b01);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b11);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);

    stateChangeRegister(0b10);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);

    stateChangeRegister(0b00);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);

    stateChangeRegister(0b11);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b10);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);

    stateChangeRegister(0b00);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);

    stateChangeRegister(0b01);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);

    stateChangeRegister(0b10);
    ASSERT_EQ(0, _listener._event.size());

    stateChangeRegister(0b00);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);

    stateChangeRegister(0b01);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);

    stateChangeRegister(0b11);
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);

    // this time configure 4 pulses per step
    for (int i = 0; i < Encoders::Collection::size(); i++)
    {
        ASSERT_TRUE(_encoders._database.update(Database::Config::Section::encoder_t::pulsesPerStep, i, 4));
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
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

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
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);

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
    ASSERT_EQ(Encoders::Collection::size(), _listener._event.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
}

#endif