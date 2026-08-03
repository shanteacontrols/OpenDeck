/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/database.h"
#include "firmware/src/protocol/midi/builder/builder.h"
#include "firmware/src/util/configurable/configurable.h"

using namespace opendeck;
using namespace opendeck::firmware;

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

        uint8_t set_midi_setting(protocol::midi::Setting setting, uint16_t value)
        {
            return ConfigHandler.set(sys::Config::Block::Global,
                                     static_cast<uint8_t>(sys::Config::Section::Global::MidiSettings),
                                     static_cast<size_t>(setting),
                                     value);
        }

        void expect_ble_enable()
        {
            EXPECT_CALL(_midi._hwaBle, init()).WillOnce(testing::Return(true));
            EXPECT_CALL(_midi._hwaBle, deinit()).WillRepeatedly(testing::Return(true));
        }

        void expect_din_enable()
        {
            EXPECT_CALL(_midi._hwaSerial, init()).WillOnce(testing::Return(true));
            EXPECT_CALL(_midi._hwaSerial, deinit()).WillRepeatedly(testing::Return(true));
            EXPECT_CALL(_midi._hwaSerial, set_loopback(testing::_)).WillRepeatedly(testing::Return(true));
        }

        tests::NoOpDatabaseHandlers _handlers;
        database::Builder           _builder_database;
        database::Admin&            _database_admin = _builder_database.instance();
        protocol::midi::Builder     _midi           = protocol::midi::Builder(_database_admin);
    };
}    // namespace

TEST_F(MIDITest, OmniChannel)
{
    signaling::publish(signaling::MidiIoSignal{
        .source          = signaling::IoEventSource::Switch,
        .component_index = 0,
        .channel         = 1,
        .index           = 0,
        .value           = 127,
        .message         = protocol::midi::MessageType::NoteOn,
    });
    wait_for_signal_dispatch();

    // only 1 message should be written out
    ASSERT_EQ(1, _midi._hwaUsb._writeParser.total_written_channel_messages());
    ASSERT_EQ(protocol::midi::MessageType::NoteOn, _midi._hwaUsb._writeParser.written_messages().at(0).type);

    // now set the channel to omni and verify that 16 messages are sent
    _midi._hwaUsb.clear();
    signaling::publish(signaling::MidiIoSignal{
        .source          = signaling::IoEventSource::Switch,
        .component_index = 0,
        .channel         = protocol::midi::OMNI_CHANNEL,
        .index           = 0,
        .value           = 127,
        .message         = protocol::midi::MessageType::NoteOn,
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

TEST_F(MIDITest, EncoderContinuous14BitControlChangeWritesCoarseAndFineControllers)
{
    signaling::publish(signaling::MidiIoSignal{
        .source          = signaling::IoEventSource::Encoder,
        .component_index = 0,
        .channel         = 1,
        .index           = 10,
        .value           = 0x1234,
        .message         = protocol::midi::MessageType::ControlChange14Bit,
    });
    wait_for_signal_dispatch();

    const auto& messages = _midi._hwaUsb._writeParser.written_messages();

    ASSERT_EQ(2, _midi._hwaUsb._writeParser.total_written_channel_messages());
    ASSERT_EQ(2, messages.size());

    EXPECT_EQ(protocol::midi::MessageType::ControlChange, messages.at(0).type);
    EXPECT_EQ(1, messages.at(0).channel);
    EXPECT_EQ(10, messages.at(0).data1);
    EXPECT_EQ(0x24, messages.at(0).data2);

    EXPECT_EQ(protocol::midi::MessageType::ControlChange, messages.at(1).type);
    EXPECT_EQ(1, messages.at(1).channel);
    EXPECT_EQ(42, messages.at(1).data1);
    EXPECT_EQ(0x34, messages.at(1).data2);
}

TEST_F(MIDITest, Analog7BitNrpnWritesParameterAndDataEntryControllers)
{
    signaling::publish(signaling::MidiIoSignal{
        .source          = signaling::IoEventSource::Analog,
        .component_index = 0,
        .channel         = 1,
        .index           = 0x1234,
        .value           = 0x56,
        .message         = protocol::midi::MessageType::Nrpn7Bit,
    });
    wait_for_signal_dispatch();

    const auto& messages = _midi._hwaUsb._writeParser.written_messages();

    ASSERT_EQ(3, _midi._hwaUsb._writeParser.total_written_channel_messages());
    ASSERT_EQ(3, messages.size());

    EXPECT_EQ(protocol::midi::MessageType::ControlChange, messages.at(0).type);
    EXPECT_EQ(1, messages.at(0).channel);
    EXPECT_EQ(99, messages.at(0).data1);
    EXPECT_EQ(0x24, messages.at(0).data2);

    EXPECT_EQ(protocol::midi::MessageType::ControlChange, messages.at(1).type);
    EXPECT_EQ(1, messages.at(1).channel);
    EXPECT_EQ(98, messages.at(1).data1);
    EXPECT_EQ(0x34, messages.at(1).data2);

    EXPECT_EQ(protocol::midi::MessageType::ControlChange, messages.at(2).type);
    EXPECT_EQ(1, messages.at(2).channel);
    EXPECT_EQ(6, messages.at(2).data1);
    EXPECT_EQ(0x56, messages.at(2).data2);
}

TEST_F(MIDITest, Encoder14BitNrpnWritesParameterAndDataEntryControllers)
{
    signaling::publish(signaling::MidiIoSignal{
        .source          = signaling::IoEventSource::Encoder,
        .component_index = 0,
        .channel         = 1,
        .index           = 0x1234,
        .value           = 0x2345,
        .message         = protocol::midi::MessageType::Nrpn14Bit,
    });
    wait_for_signal_dispatch();

    const auto& messages = _midi._hwaUsb._writeParser.written_messages();

    ASSERT_EQ(4, _midi._hwaUsb._writeParser.total_written_channel_messages());
    ASSERT_EQ(4, messages.size());

    EXPECT_EQ(protocol::midi::MessageType::ControlChange, messages.at(0).type);
    EXPECT_EQ(1, messages.at(0).channel);
    EXPECT_EQ(99, messages.at(0).data1);
    EXPECT_EQ(0x24, messages.at(0).data2);

    EXPECT_EQ(protocol::midi::MessageType::ControlChange, messages.at(1).type);
    EXPECT_EQ(1, messages.at(1).channel);
    EXPECT_EQ(98, messages.at(1).data1);
    EXPECT_EQ(0x34, messages.at(1).data2);

    EXPECT_EQ(protocol::midi::MessageType::ControlChange, messages.at(2).type);
    EXPECT_EQ(1, messages.at(2).channel);
    EXPECT_EQ(6, messages.at(2).data1);
    EXPECT_EQ(0x46, messages.at(2).data2);

    EXPECT_EQ(protocol::midi::MessageType::ControlChange, messages.at(3).type);
    EXPECT_EQ(1, messages.at(3).channel);
    EXPECT_EQ(38, messages.at(3).data1);
    EXPECT_EQ(0x45, messages.at(3).data2);
}

TEST_F(MIDITest, EncoderPitchBendWrites14BitValue)
{
    signaling::publish(signaling::MidiIoSignal{
        .source          = signaling::IoEventSource::Encoder,
        .component_index = 0,
        .channel         = 1,
        .index           = 0,
        .value           = 0x1234,
        .message         = protocol::midi::MessageType::PitchBend,
    });
    wait_for_signal_dispatch();

    const auto& messages = _midi._hwaUsb._writeParser.written_messages();

    ASSERT_EQ(1, _midi._hwaUsb._writeParser.total_written_channel_messages());
    ASSERT_EQ(1, messages.size());

    EXPECT_EQ(protocol::midi::MessageType::PitchBend, messages.at(0).type);
    EXPECT_EQ(1, messages.at(0).channel);
    EXPECT_EQ(0x1234, messages.at(0).data2);
}

TEST_F(MIDITest, TestBackendSupportDefaultsToEnabled)
{
    ASSERT_TRUE(_midi._hwaUsb.supported());
    ASSERT_TRUE(_midi._hwaSerial.supported());
    ASSERT_TRUE(_midi._hwaBle.supported());
}

TEST_F(MIDITest, BleEnabledIsAppliedWhenBleIsSupported)
{
    expect_ble_enable();

    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(protocol::midi::Setting::BleEnabled, 1));
    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(protocol::midi::Setting::BleEnabled, 0));
}

TEST_F(MIDITest, BleEnabledIsRejectedWhenBleIsNotSupported)
{
    _midi._hwaBle._supported = false;

    ASSERT_EQ(sys::Config::Status::ErrorNotSupported, set_midi_setting(protocol::midi::Setting::BleEnabled, 1));
}

TEST_F(MIDITest, BleTxIsSkippedUntilReady)
{
    expect_ble_enable();

    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(protocol::midi::Setting::BleEnabled, 1));

    _midi._hwaBle.clear();
    _midi._hwaBle._ready = false;

    signaling::publish(signaling::MidiIoSignal{
        .source          = signaling::IoEventSource::Switch,
        .component_index = 0,
        .channel         = 1,
        .index           = 0,
        .value           = 127,
        .message         = protocol::midi::MessageType::NoteOn,
    });
    wait_for_signal_dispatch();

    ASSERT_TRUE(_midi._hwaBle._writePackets.empty());
}

TEST_F(MIDITest, UsbThruBleRouteIsAppliedWhenUsbAndBleAreSupported)
{
    expect_ble_enable();

    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(protocol::midi::Setting::BleEnabled, 1));
    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(protocol::midi::Setting::UsbThruBle, 1));

    _midi._hwaBle.clear();
    _midi._hwaUsb._readPackets.push_back(zlibs::utils::midi::midi1::note_on(0, 0, 60, 127));
    k_poll_signal_raise(_midi._hwaUsb.data_available_signal(), 0);
    wait_for_signal_dispatch();

    ASSERT_FALSE(_midi._hwaBle._writePackets.empty());
}

TEST_F(MIDITest, UsbThruBleRouteIsRejectedWhenBleIsNotSupported)
{
    _midi._hwaBle._supported = false;

    ASSERT_EQ(sys::Config::Status::ErrorNotSupported, set_midi_setting(protocol::midi::Setting::UsbThruBle, 1));
}

TEST_F(MIDITest, BleThruUsbRouteIsAppliedWhenBleAndUsbAreSupported)
{
    expect_ble_enable();

    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(protocol::midi::Setting::BleEnabled, 1));
    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(protocol::midi::Setting::BleThruUsb, 1));

    _midi._hwaUsb.clear();

    protocol::midi::BlePacket packet = {};
    packet.size                      = 5;
    packet.data                      = { 0x80, 0x80, 0x90, 0x3C, 0x7F };

    _midi._hwaBle._readPackets.push_back(packet);
    k_poll_signal_raise(_midi._hwaBle.data_available_signal(), 0);
    wait_for_signal_dispatch();

    ASSERT_FALSE(_midi._hwaUsb._writePackets.empty());
}

TEST_F(MIDITest, BleThruUsbRouteIsRejectedWhenBleIsNotSupported)
{
    _midi._hwaBle._supported = false;

    ASSERT_EQ(sys::Config::Status::ErrorNotSupported, set_midi_setting(protocol::midi::Setting::BleThruUsb, 1));
}

TEST_F(MIDITest, DinAndBleThruRoutesAreApplied)
{
    expect_din_enable();
    expect_ble_enable();

    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(protocol::midi::Setting::DinEnabled, 1));
    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(protocol::midi::Setting::BleEnabled, 1));
    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(protocol::midi::Setting::DinThruBle, 1));
    ASSERT_EQ(sys::Config::Status::Ack, set_midi_setting(protocol::midi::Setting::BleThruDin, 1));

    _midi._hwaBle.clear();
    _midi._hwaSerial._readPackets = { 0x90, 0x3C, 0x7F };
    k_poll_signal_raise(_midi._hwaSerial.data_available_signal(), 0);
    wait_for_signal_dispatch();
    ASSERT_FALSE(_midi._hwaBle._writePackets.empty());

    _midi._hwaSerial._writePackets.clear();

    protocol::midi::BlePacket packet = {};
    packet.size                      = 5;
    packet.data                      = { 0x80, 0x80, 0x90, 0x3C, 0x7F };

    _midi._hwaBle._readPackets.push_back(packet);
    k_poll_signal_raise(_midi._hwaBle.data_available_signal(), 0);
    wait_for_signal_dispatch();
    ASSERT_FALSE(_midi._hwaSerial._writePackets.empty());
}

TEST_F(MIDITest, DinRoutesAreRejectedWhenDinIsNotSupported)
{
    _midi._hwaSerial._supported = false;

    ASSERT_EQ(sys::Config::Status::ErrorNotSupported, set_midi_setting(protocol::midi::Setting::DinEnabled, 1));
    ASSERT_EQ(sys::Config::Status::ErrorNotSupported, set_midi_setting(protocol::midi::Setting::DinThruUsb, 1));
}
