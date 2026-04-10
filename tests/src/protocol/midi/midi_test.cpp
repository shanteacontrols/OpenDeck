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
#include "application/protocol/midi/builder.h"
#include "application/util/configurable/configurable.h"

using namespace io;
using namespace protocol;

namespace
{
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

    class MIDITest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_databaseAdmin.init(_handlers));

            EXPECT_CALL(_midi._hwaSerial, setLoopback(false))
                .WillOnce(Return(true));

            ASSERT_TRUE(_midi._instance.init());
        }

        void TearDown() override
        {
            util::Configurable::instance().clear();
            messaging::SignalRegistry<messaging::MidiSignal>::instance().clear();
            messaging::SignalRegistry<messaging::SystemSignal>::instance().clear();
            messaging::SignalRegistry<messaging::UmpSignal>::instance().clear();
            messaging::SignalRegistry<messaging::MidiTrafficSignal>::instance().clear();
        }

        void waitForSignalDispatch()
        {
            k_msleep(5);
        }

        NoOpHandlers            _handlers;
        database::Builder       _builderDatabase;
        database::Admin&        _databaseAdmin = _builderDatabase.instance();
        protocol::midi::Builder _midi          = protocol::midi::Builder(_databaseAdmin);
    };
}    // namespace

TEST_F(MIDITest, OmniChannel)
{
    messaging::publish(messaging::MidiSignal{
        .source         = messaging::MidiSource::Button,
        .componentIndex = 0,
        .channel        = 1,
        .index          = 0,
        .value          = 127,
        .message        = midi::messageType_t::NOTE_ON,
    });
    waitForSignalDispatch();

    // only 1 message should be written out
    ASSERT_EQ(1, _midi._hwaUsb._writeParser.totalWrittenChannelMessages());
    ASSERT_EQ(midi::messageType_t::NOTE_ON, _midi._hwaUsb._writeParser.writtenMessages().at(0).type);

    // now set the channel to omni and verify that 16 messages are sent
    _midi._hwaUsb.clear();
    messaging::publish(messaging::MidiSignal{
        .source         = messaging::MidiSource::Button,
        .componentIndex = 0,
        .channel        = midi::OMNI_CHANNEL,
        .index          = 0,
        .value          = 127,
        .message        = midi::messageType_t::NOTE_ON,
    });
    waitForSignalDispatch();

    ASSERT_EQ(16, _midi._hwaUsb._writeParser.totalWrittenChannelMessages());

    // verify that the messages are identical apart from the channel
    for (size_t i = 0; i < 16; i++)
    {
        ASSERT_EQ(i + 1, _midi._hwaUsb._writeParser.writtenMessages().at(i).channel);
        ASSERT_EQ(0, _midi._hwaUsb._writeParser.writtenMessages().at(i).data1);
        ASSERT_EQ(127, _midi._hwaUsb._writeParser.writtenMessages().at(i).data2);
    }
}
