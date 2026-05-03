/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/database.h"
#include "protocol/midi/builder.h"
#include "util/configurable/configurable.h"

using namespace opendeck::io;
using namespace opendeck::protocol;

namespace
{
    class MIDITest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_midi._instance.init());
        }

        void TearDown() override
        {
            util::Configurable::instance().clear();
            signaling::clear_registry();
        }

        void wait_for_signal_dispatch()
        {
            k_msleep(5);
        }

        uint8_t set_midi_setting(midi::Setting setting, uint16_t value)
        {
            return ConfigHandler.set(sys::Config::Block::Global,
                                     static_cast<uint8_t>(sys::Config::Section::Global::MidiSettings),
                                     static_cast<size_t>(setting),
                                     value);
        }

        tests::NoOpDatabaseHandlers _handlers;
        database::Builder           _builder_database;
        database::Admin&            _database_admin = _builder_database.instance();
        protocol::midi::Builder     _midi           = protocol::midi::Builder(_database_admin);
    };
}    // namespace

TEST_F(MIDITest, OmniChannel)
{
    signaling::publish(signaling::MidiSignal{
        .source          = signaling::MidiSource::Button,
        .component_index = 0,
        .channel         = 1,
        .index           = 0,
        .value           = 127,
        .message         = midi::MessageType::NoteOn,
    });
    wait_for_signal_dispatch();

    // only 1 message should be written out
    ASSERT_EQ(1, _midi._hwaUsb._writeParser.total_written_channel_messages());
    ASSERT_EQ(midi::MessageType::NoteOn, _midi._hwaUsb._writeParser.written_messages().at(0).type);

    // now set the channel to omni and verify that 16 messages are sent
    _midi._hwaUsb.clear();
    signaling::publish(signaling::MidiSignal{
        .source          = signaling::MidiSource::Button,
        .component_index = 0,
        .channel         = midi::OMNI_CHANNEL,
        .index           = 0,
        .value           = 127,
        .message         = midi::MessageType::NoteOn,
    });
    wait_for_signal_dispatch();

    ASSERT_EQ(16, _midi._hwaUsb._writeParser.total_written_channel_messages());

    // verify that the messages are identical apart from the channel
    for (size_t i = 0; i < 16; i++)
    {
        ASSERT_EQ(i + 1, _midi._hwaUsb._writeParser.written_messages().at(i).channel);
        ASSERT_EQ(0, _midi._hwaUsb._writeParser.written_messages().at(i).data1);
        ASSERT_EQ(127, _midi._hwaUsb._writeParser.written_messages().at(i).data2);
    }
}

TEST_F(MIDITest, BleEnabledSupportMatchesBuild)
{
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BLE
    EXPECT_CALL(_midi._hwaBle, init()).WillOnce(testing::Return(true));
    EXPECT_CALL(_midi._hwaBle, deinit()).WillRepeatedly(testing::Return(true));

    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(midi::Setting::BleEnabled, 1));
    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(midi::Setting::BleEnabled, 0));
#else
    ASSERT_EQ(sys::Config::Status::ErrorNotSupported, set_midi_setting(midi::Setting::BleEnabled, 1));
#endif
}

TEST_F(MIDITest, BleTxIsSkippedUntilReady)
{
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BLE
    EXPECT_CALL(_midi._hwaBle, init()).WillOnce(testing::Return(true));
    EXPECT_CALL(_midi._hwaBle, deinit()).WillRepeatedly(testing::Return(true));

    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(midi::Setting::BleEnabled, 1));

    _midi._hwaBle.clear();
    _midi._hwaBle._ready = false;

    signaling::publish(signaling::MidiSignal{
        .source          = signaling::MidiSource::Button,
        .component_index = 0,
        .channel         = 1,
        .index           = 0,
        .value           = 127,
        .message         = midi::MessageType::NoteOn,
    });
    wait_for_signal_dispatch();

    ASSERT_TRUE(_midi._hwaBle._writePackets.empty());
#endif
}

TEST_F(MIDITest, UsbThruBleSupportMatchesBuild)
{
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BLE
    EXPECT_CALL(_midi._hwaBle, init()).WillOnce(testing::Return(true));
    EXPECT_CALL(_midi._hwaBle, deinit()).WillRepeatedly(testing::Return(true));

    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(midi::Setting::BleEnabled, 1));
    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(midi::Setting::UsbThruBle, 1));

    _midi._hwaBle.clear();
    _midi._hwaUsb._readPackets.push_back(zlibs::utils::midi::midi1::note_on(0, 0, 60, 127));
    k_poll_signal_raise(_midi._hwaUsb.data_available_signal(), 0);
    wait_for_signal_dispatch();

    ASSERT_FALSE(_midi._hwaBle._writePackets.empty());
#else
    ASSERT_EQ(sys::Config::Status::ErrorNotSupported, set_midi_setting(midi::Setting::UsbThruBle, 1));
#endif
}

TEST_F(MIDITest, BleThruUsbSupportMatchesBuild)
{
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BLE
    EXPECT_CALL(_midi._hwaBle, init()).WillOnce(testing::Return(true));
    EXPECT_CALL(_midi._hwaBle, deinit()).WillRepeatedly(testing::Return(true));

    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(midi::Setting::BleEnabled, 1));
    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(midi::Setting::BleThruUsb, 1));

    _midi._hwaUsb.clear();

    midi::BlePacket packet = {};
    packet.size            = 5;
    packet.data            = { 0x80, 0x80, 0x90, 0x3C, 0x7F };

    _midi._hwaBle._readPackets.push_back(packet);
    k_poll_signal_raise(_midi._hwaBle.data_available_signal(), 0);
    wait_for_signal_dispatch();

    ASSERT_FALSE(_midi._hwaUsb._writePackets.empty());
#else
    ASSERT_EQ(sys::Config::Status::ErrorNotSupported, set_midi_setting(midi::Setting::BleThruUsb, 1));
#endif
}

#if defined(CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI) && defined(CONFIG_PROJECT_TARGET_SUPPORT_BLE)
TEST_F(MIDITest, DinAndBleThruRoutesAreApplied)
{
    EXPECT_CALL(_midi._hwaSerial, init()).WillOnce(testing::Return(true));
    EXPECT_CALL(_midi._hwaSerial, deinit()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(_midi._hwaSerial, set_loopback(testing::_)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(_midi._hwaBle, init()).WillOnce(testing::Return(true));
    EXPECT_CALL(_midi._hwaBle, deinit()).WillRepeatedly(testing::Return(true));

    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(midi::Setting::DinEnabled, 1));
    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(midi::Setting::BleEnabled, 1));
    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(midi::Setting::DinThruBle, 1));
    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(midi::Setting::BleThruDin, 1));

    _midi._hwaBle.clear();
    _midi._hwaSerial._readPackets = { 0x90, 0x3C, 0x7F };
    k_poll_signal_raise(_midi._hwaSerial.data_available_signal(), 0);
    wait_for_signal_dispatch();
    ASSERT_FALSE(_midi._hwaBle._writePackets.empty());

    _midi._hwaSerial._writePackets.clear();

    midi::BlePacket packet = {};
    packet.size            = 5;
    packet.data            = { 0x80, 0x80, 0x90, 0x3C, 0x7F };

    _midi._hwaBle._readPackets.push_back(packet);
    k_poll_signal_raise(_midi._hwaBle.data_available_signal(), 0);
    wait_for_signal_dispatch();
    ASSERT_FALSE(_midi._hwaSerial._writePackets.empty());
}
#endif
