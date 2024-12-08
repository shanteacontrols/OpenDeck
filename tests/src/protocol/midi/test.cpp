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

#ifndef PROJECT_TARGET_USB_OVER_SERIAL_HOST

#include "tests/common.h"
#include "application/protocol/midi/builder_test.h"

using namespace io;

namespace
{
    class MIDITest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_midi._databaseAdmin.init());

            EXPECT_CALL(_midi._hwaSerial, setLoopback(false))
                .WillOnce(Return(true));

            ASSERT_TRUE(_midi._instance.init());
        }

        void TearDown() override
        {
        }

        protocol::midi::BuilderTest _midi;
    };
}    // namespace

TEST_F(MIDITest, OmniChannel)
{
    // simulate button event
    messaging::Event event = {};
    event.componentIndex   = 0;
    event.channel          = 1;
    event.index            = 0;
    event.value            = 127;
    event.message          = midi::messageType_t::NOTE_ON;

    MidiDispatcher.notify(messaging::eventType_t::BUTTON, event);

    // only 1 message should be written out
    ASSERT_EQ(1, _midi._hwaUsb._writeParser.totalWrittenChannelMessages());
    ASSERT_EQ(midi::messageType_t::NOTE_ON, _midi._hwaUsb._writeParser.writtenMessages().at(0).type);

    // now set the channel to omni and verify that 16 messages are sent
    _midi._hwaUsb.clear();
    event.channel = midi::OMNI_CHANNEL;

    MidiDispatcher.notify(messaging::eventType_t::BUTTON, event);

    ASSERT_EQ(16, _midi._hwaUsb._writeParser.totalWrittenChannelMessages());

    // verify that the messages are identical apart from the channel
    for (size_t i = 0; i < 16; i++)
    {
        ASSERT_EQ(i + 1, _midi._hwaUsb._writeParser.writtenMessages().at(i).channel);
        ASSERT_EQ(0, _midi._hwaUsb._writeParser.writtenMessages().at(i).data1);
        ASSERT_EQ(127, _midi._hwaUsb._writeParser.writtenMessages().at(i).data2);
    }
}

#endif