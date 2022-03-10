#ifndef USB_LINK_MCU

#include "framework/Framework.h"
#include "stubs/MIDI.h"

using namespace IO;

namespace
{
    class MIDITest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_midi._database.init());

            EXPECT_CALL(_midi._hwaMIDIDIN, setLoopback(false))
                .WillOnce(Return(true));

            ASSERT_TRUE(_midi._instance.init());
        }

        void TearDown() override
        {
        }

        TestMIDI _midi;
    };
}    // namespace

TEST_F(MIDITest, OmniChannel)
{
    // simulate button event
    Messaging::event_t event;

    event.componentIndex = 0;
    event.midiChannel    = 1;
    event.midiIndex      = 0;
    event.midiValue      = 127;
    event.message        = MIDI::messageType_t::noteOn;

    MIDIDispatcher.notify(Messaging::eventSource_t::buttons,
                          event,
                          Messaging::listenType_t::nonFwd);

    // only 1 message should be written out
    ASSERT_EQ(1, _midi._hwaMIDIUSB._writeParser.totalWrittenChannelMessages());
    ASSERT_EQ(MIDI::messageType_t::noteOn, _midi._hwaMIDIUSB._writeParser.writtenMessages().at(0).type);

    // now set the channel to omni and verify that 16 messages are sent
    _midi._hwaMIDIUSB.clear();
    event.midiChannel = MIDI::MIDI_CHANNEL_OMNI;

    MIDIDispatcher.notify(Messaging::eventSource_t::buttons,
                          event,
                          Messaging::listenType_t::nonFwd);

    ASSERT_EQ(16, _midi._hwaMIDIUSB._writeParser.totalWrittenChannelMessages());

    // verify that the messages are identical apart from the channel
    for (size_t i = 0; i < 16; i++)
    {
        ASSERT_EQ(i + 1, _midi._hwaMIDIUSB._writeParser.writtenMessages().at(i).channel);
        ASSERT_EQ(0, _midi._hwaMIDIUSB._writeParser.writtenMessages().at(i).data1);
        ASSERT_EQ(127, _midi._hwaMIDIUSB._writeParser.writtenMessages().at(i).data2);
    }
}

#endif