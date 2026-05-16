/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/database.h"
#include "tests/helpers/misc.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ADC

#include "firmware/src/io/analog/builder/builder.h"
#include "firmware/src/io/analog/drivers/scan_driver_base.h"
#include "firmware/src/io/analog/instance/impl/remap.h"
#include "firmware/src/io/digital/switches/builder/builder.h"
#include "firmware/src/io/digital/switches/instance/impl/switches.h"
#include "firmware/src/util/configurable/configurable.h"

#include "zlibs/utils/misc/ring_buffer.h"
#include "zlibs/utils/misc/mutex.h"
#include "zlibs/utils/misc/numeric.h"

#include <deque>
#include <optional>

using namespace opendeck::io;
using namespace opendeck::protocol;

namespace
{
    const std::vector<uint16_t> midi_7bit_test_values = {
        0,
        1,
        2,
        63,
        64,
        65,
        126,
        127,
    };

    const std::vector<uint16_t> scaling_test_values = {
        0,
        1,
        2,
        10,
        11,
        12,
        63,
        64,
        65,
        99,
        100,
        101,
        126,
        127,
    };

    const std::vector<uint16_t> pitch_bend_position_values = {
        0,
        1,
        2,
        4,
        8,
        32,
        64,
        128,
        255,
        256,
        384,
        510,
        511,
    };

    analog::Frame make_frame(uint16_t value)
    {
        analog::Frame frame = {};
        frame.fill(value);
        return frame;
    }

    class FakeScanDriver : public analog::drivers::ScanDriverBase<FakeScanDriver>
    {
        public:
        static constexpr size_t INPUT_COUNT = 3;

        bool init_driver()
        {
            return true;
        }

        size_t physical_input_count() const
        {
            return INPUT_COUNT;
        }

        void select_input(size_t index)
        {
            _activeIndex = index;
            selected_inputs.push_back(index);
        }

        std::optional<uint16_t> read_sample()
        {
            if (samples[_activeIndex].empty())
            {
                return {};
            }

            const auto sample = samples[_activeIndex].front();
            samples[_activeIndex].pop_front();
            return sample;
        }

        std::array<std::deque<uint16_t>, INPUT_COUNT> samples         = {};
        std::vector<size_t>                           selected_inputs = {};

        private:
        size_t _activeIndex = 0;
    };

    class MidiIoSignalCollector
    {
        public:
        void push(const signaling::MidiIoSignal& signal)
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _signals.push_back(signal);
        }

        void clear()
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            _signals.clear();
        }

        size_t size() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _signals.size();
        }

        std::vector<signaling::MidiIoSignal> snapshot() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _signals;
        }

        private:
        mutable zlibs::utils::misc::Mutex    _mutex;
        std::vector<signaling::MidiIoSignal> _signals = {};
    };

    class AnalogTest : public ::testing::Test
    {
        protected:
        void reset_analog(size_t index)
        {
            auto type = _analog._database.read(database::Config::Section::Analog::Type, index);

            ASSERT_EQ(static_cast<uint8_t>(sys::Config::Status::Ack),
                      ConfigHandler.set(sys::Config::Block::Analog,
                                        static_cast<uint8_t>(sys::Config::Section::Analog::Type),
                                        index,
                                        type));
        }

        void SetUp() override
        {
            tests::resume_io();

            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_database_admin.factory_reset());
            ASSERT_EQ(0, _database_admin.current_preset());
            ASSERT_TRUE(_analog._instance.init());
            EXPECT_CALL(_switches._hwa, init())
                .WillOnce(Return(true));
            ASSERT_TRUE(_switches._instance.init());

            for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
            {
                ASSERT_EQ(static_cast<uint8_t>(sys::Config::Status::Ack),
                          ConfigHandler.set(sys::Config::Block::Analog,
                                            static_cast<uint8_t>(sys::Config::Section::Analog::Enable),
                                            i,
                                            1));
                ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::Channel, i, 1));
            }

            signaling::subscribe<signaling::MidiIoSignal>(
                [this](const signaling::MidiIoSignal& signal)
                {
                    if (signal.source == signaling::IoEventSource::Analog)
                    {
                        _analog_messages.push(signal);
                    }

                    if (signal.source == signaling::IoEventSource::AnalogSwitch)
                    {
                        _analogSwitch_messages.push(signal);
                    }

                    if (signal.source == signaling::IoEventSource::Switch)
                    {
                        _switch_messages.push(signal);
                    }
                });

            // The signaling dispatcher is asynchronous and shared across test
            // cases, so allow any stale queued traffic to drain before each
            // test starts asserting on fresh messages.
            k_msleep(20);
            clear_messages();
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            signaling::clear_registry();
            clear_messages();
        }

        void state_change_register(uint16_t value)
        {
            state_change_register(std::vector<uint16_t>{ value });
        }

        uint16_t position_for_midi_value(uint16_t value, uint16_t max_value) const
        {
            return static_cast<uint16_t>(zlibs::utils::misc::map_range(static_cast<uint32_t>(value),
                                                                       static_cast<uint32_t>(0),
                                                                       static_cast<uint32_t>(max_value),
                                                                       static_cast<uint32_t>(0),
                                                                       static_cast<uint32_t>(analog::Filter::POSITION_MAX_VALUE)));
        }

        void state_change_register_midi_7bit(uint16_t value)
        {
            state_change_register(position_for_midi_value(value, midi::MAX_VALUE_7BIT));
        }

        void state_change_register_midi_14bit(uint16_t value)
        {
            state_change_register(position_for_midi_value(value, midi::MAX_VALUE_14BIT));
        }

        uint16_t midi_14bit_value_for_position(uint16_t value) const
        {
            return static_cast<uint16_t>(zlibs::utils::misc::map_range(static_cast<uint32_t>(value),
                                                                       static_cast<uint32_t>(0),
                                                                       static_cast<uint32_t>(analog::Filter::POSITION_MAX_VALUE),
                                                                       static_cast<uint32_t>(0),
                                                                       static_cast<uint32_t>(midi::MAX_VALUE_14BIT)));
        }

        void state_change_register(const std::vector<uint16_t>& values)
        {
            for (auto value : values)
            {
                _analog._hwa.push_frame(make_frame(value));
            }
        }

        void clear_messages()
        {
            _analog_messages.clear();
            _analogSwitch_messages.clear();
            _switch_messages.clear();
        }

        void wait_for_signals()
        {
            size_t stable_iterations = 0;
            size_t last_size         = static_cast<size_t>(-1);

            while (stable_iterations < 10)
            {
                const size_t current_size = _analog_messages.size() + _analogSwitch_messages.size() + _switch_messages.size();

                if (current_size == last_size)
                {
                    stable_iterations++;
                }
                else
                {
                    stable_iterations = 0;
                    last_size         = current_size;
                }

                k_msleep(1);
            }
        }

        tests::NoOpDatabaseHandlers _handlers;
        database::Builder           _builder_database;
        database::Admin&            _database_admin = _builder_database.instance();
        analog::Builder             _analog         = analog::Builder(_database_admin);
        switches::Builder           _switches       = switches::Builder(_database_admin);
        MidiIoSignalCollector       _analog_messages;
        MidiIoSignalCollector       _analogSwitch_messages;
        MidiIoSignalCollector       _switch_messages;
    };
}    // namespace

TEST(AnalogDriverBufferTest, KeepsNewestEightFrames)
{
    static constexpr size_t                                              BUFFER_CAPACITY = 8;
    zlibs::utils::misc::RingBuffer<BUFFER_CAPACITY, true, analog::Frame> buffer;

    EXPECT_FALSE(buffer.remove().has_value());

    for (uint16_t i = 0; i < 9; i++)
    {
        EXPECT_TRUE(buffer.insert(make_frame(i)));
    }

    for (uint16_t i = 1; i < 9; i++)
    {
        auto frame = buffer.remove();

        ASSERT_TRUE(frame.has_value());
        EXPECT_EQ(i, frame->at(0));
    }

    EXPECT_FALSE(buffer.remove().has_value());
}

TEST(AnalogDriverScanTest, PublishesFullFrames)
{
    if (FakeScanDriver::INPUT_COUNT < 3)
    {
        return;
    }

    FakeScanDriver   driver;
    analog::ScanMask mask = {};

    for (size_t i = 0; i < FakeScanDriver::INPUT_COUNT; i++)
    {
        mask[i] = true;
    }

    driver.samples[0] = { 10, 10, 30, 30 };
    driver.samples[1] = { 20, 20, 40, 40 };
    driver.samples[2] = { 50, 50, 60, 60 };

    ASSERT_TRUE(driver.init());
    driver.set_scan_mask(mask);

    auto frame = driver.read();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(10, frame->at(0));
    EXPECT_EQ(20, frame->at(1));
    EXPECT_EQ(50, frame->at(2));
    EXPECT_EQ((std::vector<size_t>{ 0, 1, 2 }), driver.selected_inputs);

    driver.selected_inputs.clear();

    frame = driver.read();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(30, frame->at(0));
    EXPECT_EQ(40, frame->at(1));
    EXPECT_EQ(60, frame->at(2));
    EXPECT_EQ((std::vector<size_t>{ 0, 1, 2 }), driver.selected_inputs);
}

TEST(AnalogDriverScanTest, SkipsMaskedChannels)
{
    if (FakeScanDriver::INPUT_COUNT < 3)
    {
        return;
    }

    FakeScanDriver          driver;
    analog::ScanMask        mask         = {};
    static constexpr size_t FIRST_INPUT  = 0;
    static constexpr size_t SECOND_INPUT = 1;
    static constexpr size_t LAST_INPUT   = FakeScanDriver::INPUT_COUNT - 1;

    mask[FIRST_INPUT] = true;
    mask[LAST_INPUT]  = true;

    driver.samples[FIRST_INPUT] = { 10, 10 };
    driver.samples[LAST_INPUT]  = { 50, 50 };

    ASSERT_TRUE(driver.init());
    driver.set_scan_mask(mask);

    auto frame = driver.read();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(10, frame->at(FIRST_INPUT));
    EXPECT_EQ(0, frame->at(SECOND_INPUT));
    EXPECT_EQ(50, frame->at(LAST_INPUT));
    EXPECT_EQ((std::vector<size_t>{ FIRST_INPUT, LAST_INPUT }), driver.selected_inputs);
}

TEST_F(AnalogTest, CC)
{
    // feed all the values from minimum to maximum
    // expect the following:
    // first value is 0
    // last value is 127

    for (auto value : midi_7bit_test_values)
    {
        state_change_register_midi_7bit(value);
    }

    wait_for_signals();
    auto analog_messages = _analog_messages.snapshot();

    EXPECT_EQ((analog::Collection::size(analog::GroupAnalogInputs) * midi_7bit_test_values.size()), analog_messages.size());

    // all received messages should be control change
    for (size_t i = 0; i < analog_messages.size(); i++)
    {
        EXPECT_EQ(midi::MessageType::ControlChange, analog_messages.at(i).message);
    }

    for (size_t i = 0; i < midi_7bit_test_values.size(); i++)
    {
        for (size_t j = 0; j < analog::Collection::size(analog::GroupAnalogInputs); j++)
        {
            const size_t index = (i * analog::Collection::size(analog::GroupAnalogInputs)) + j;
            EXPECT_EQ(midi_7bit_test_values.at(i), analog_messages.at(index).value);
        }
    }

    // now go backward

    _analog_messages.clear();

    const std::vector<uint16_t> reverse_values(midi_7bit_test_values.rbegin() + 1, midi_7bit_test_values.rend());

    for (auto value : reverse_values)
    {
        state_change_register_midi_7bit(value);
    }

    wait_for_signals();
    analog_messages = _analog_messages.snapshot();

    EXPECT_EQ((analog::Collection::size(analog::GroupAnalogInputs) * reverse_values.size()), analog_messages.size());

    for (size_t i = 0; i < reverse_values.size(); i++)
    {
        for (size_t j = 0; j < analog::Collection::size(analog::GroupAnalogInputs); j++)
        {
            const size_t index = (i * analog::Collection::size(analog::GroupAnalogInputs)) + j;
            EXPECT_EQ(reverse_values.at(i), analog_messages.at(index).value);
        }
    }

    _analog_messages.clear();

    // try to feed value larger than 127
    // no response should be received
    state_change_register(10000);
    wait_for_signals();

    EXPECT_EQ(0, _analog_messages.size());
}

TEST_F(AnalogTest, NRPN7bit)
{
    // set known state
    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        // configure all analog components as potentiometers with 7-bit NRPN MIDI message
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::Type, i, analog::Type::Nrpn7Bit));
    }

    // feed all the values from minimum to maximum

    for (auto value : midi_7bit_test_values)
    {
        state_change_register_midi_7bit(value);
    }

    wait_for_signals();
    auto analog_messages = _analog_messages.snapshot();

    EXPECT_EQ((analog::Collection::size(analog::GroupAnalogInputs) * midi_7bit_test_values.size()), analog_messages.size());

    // all received messages should be NRPN
    for (size_t i = 0; i < analog_messages.size(); i++)
    {
        EXPECT_EQ(midi::MessageType::Nrpn7Bit, analog_messages.at(i).message);
    }
}

TEST_F(AnalogTest, NRPN14bit)
{
    // set known state
    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        // configure all analog components as potentiometers with 14-bit NRPN MIDI message
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::Type, i, analog::Type::Nrpn14Bit));
    }

    // feed all the values from minimum to maximum

    for (auto value : midi_7bit_test_values)
    {
        state_change_register_midi_7bit(value);
    }

    wait_for_signals();
    auto analog_messages = _analog_messages.snapshot();

    EXPECT_EQ((analog::Collection::size(analog::GroupAnalogInputs) * midi_7bit_test_values.size()), analog_messages.size());

    // all received messages should be NRPN
    for (size_t i = 0; i < analog_messages.size(); i++)
    {
        EXPECT_EQ(midi::MessageType::Nrpn14Bit, analog_messages.at(i).message);
    }
}

TEST_F(AnalogTest, PitchBendTest)
{
    const auto validate_messages =
        [&](const std::vector<signaling::MidiIoSignal>& analog_messages,
            const std::vector<uint16_t>&                expected_values)
    {
        const auto input_count = analog::Collection::size(analog::GroupAnalogInputs);

        EXPECT_EQ((input_count * expected_values.size()), analog_messages.size());

        for (size_t i = 0; i < analog_messages.size(); i++)
        {
            EXPECT_EQ(midi::MessageType::PitchBend, analog_messages.at(i).message);
        }

        for (size_t i = 0; i < expected_values.size(); i++)
        {
            for (size_t j = 0; j < input_count; j++)
            {
                const size_t index = (i * input_count) + j;
                EXPECT_EQ(expected_values.at(i), analog_messages.at(index).value);
            }
        }
    };

    // set known state
    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        // configure all analog components as potentiometers with Pitch Bend MIDI message
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::Type, i, analog::Type::PitchBend));
    }

    std::vector<uint16_t> expected_values;

    for (auto value : pitch_bend_position_values)
    {
        state_change_register(value);
        expected_values.push_back(midi_14bit_value_for_position(value));
    }

    wait_for_signals();
    auto analog_messages = _analog_messages.snapshot();
    validate_messages(analog_messages, expected_values);

    // now go backward
    _analog_messages.clear();

    std::vector<uint16_t> reverse_positions(pitch_bend_position_values.rbegin() + 1, pitch_bend_position_values.rend());
    std::vector<uint16_t> reverse_values;

    for (auto value : reverse_positions)
    {
        state_change_register(value);
        reverse_values.push_back(midi_14bit_value_for_position(value));
    }

    wait_for_signals();
    analog_messages = _analog_messages.snapshot();
    validate_messages(analog_messages, reverse_values);
}

TEST_F(AnalogTest, Inversion)
{
    // enable inversion
    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::Invert, i, 1));
    }

    for (auto value : midi_7bit_test_values)
    {
        state_change_register_midi_7bit(value);
    }

    wait_for_signals();
    auto analog_messages = _analog_messages.snapshot();

    EXPECT_EQ((analog::Collection::size(analog::GroupAnalogInputs) * midi_7bit_test_values.size()), analog_messages.size());

    // first value should be 127
    // last value should be 0

    for (size_t i = 0; i < midi_7bit_test_values.size(); i++)
    {
        for (size_t j = 0; j < analog::Collection::size(analog::GroupAnalogInputs); j++)
        {
            const size_t index = (i * analog::Collection::size(analog::GroupAnalogInputs)) + j;
            EXPECT_EQ(127 - midi_7bit_test_values.at(i), analog_messages.at(index).value);
        }
    }

    _analog_messages.clear();

    // funky setup: set lower limit to 127, upper to 0 while inversion is enabled
    // result should be the same as when default setup is used (no inversion / 0 as lower limit, 127 as upper limit)

    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::Invert, i, 1));
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::LowerLimit, i, 127));
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::UpperLimit, i, 0));

        reset_analog(i);
    }

    // feed all the values again
    for (auto value : midi_7bit_test_values)
    {
        state_change_register_midi_7bit(value);
    }

    wait_for_signals();
    analog_messages = _analog_messages.snapshot();

    EXPECT_EQ((analog::Collection::size(analog::GroupAnalogInputs) * midi_7bit_test_values.size()), analog_messages.size());

    for (size_t i = 0; i < midi_7bit_test_values.size(); i++)
    {
        for (size_t j = 0; j < analog::Collection::size(analog::GroupAnalogInputs); j++)
        {
            const size_t index = (i * analog::Collection::size(analog::GroupAnalogInputs)) + j;
            EXPECT_EQ(midi_7bit_test_values.at(i), analog_messages.at(index).value);
        }
    }

    _analog_messages.clear();

    // now disable inversion
    _analog_messages.clear();

    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::Invert, i, 0));
        reset_analog(i);
    }

    // feed all the values again
    for (auto value : midi_7bit_test_values)
    {
        state_change_register_midi_7bit(value);
    }

    wait_for_signals();
    analog_messages = _analog_messages.snapshot();

    EXPECT_EQ((analog::Collection::size(analog::GroupAnalogInputs) * midi_7bit_test_values.size()), analog_messages.size());

    for (size_t i = 0; i < midi_7bit_test_values.size(); i++)
    {
        for (size_t j = 0; j < analog::Collection::size(analog::GroupAnalogInputs); j++)
        {
            const size_t index = (i * analog::Collection::size(analog::GroupAnalogInputs)) + j;
            EXPECT_EQ(127 - midi_7bit_test_values.at(i), analog_messages.at(index).value);
        }
    }
}

TEST_F(AnalogTest, Scaling)
{
    if (!analog::Collection::size(analog::GroupAnalogInputs))
    {
        return;
    }

    static constexpr uint32_t SCALED_LOWER = 11;
    static constexpr uint32_t SCALED_UPPER = 100;

    // set known state
    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::LowerLimit, i, 0));
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::UpperLimit, i, SCALED_UPPER));
    }

    for (auto value : scaling_test_values)
    {
        state_change_register_midi_7bit(value);
    }

    wait_for_signals();
    auto analog_messages = _analog_messages.snapshot();

    // since the values are scaled, verify that all the messages aren't received
    std::cout << "Received " << analog_messages.size() << " messages" << std::endl;
    ASSERT_TRUE((analog::Collection::size(analog::GroupAnalogInputs) * scaling_test_values.size()) > analog_messages.size());

    // first value should be 0
    // last value should match the configured scaled value (SCALED_UPPER)
    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        EXPECT_EQ(0, analog_messages.at(i).value);
        EXPECT_EQ(SCALED_UPPER, analog_messages.at(analog_messages.size() - analog::Collection::size(analog::GroupAnalogInputs) + i).value);
    }

    // now scale minimum value as well
    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::LowerLimit, i, SCALED_LOWER));
    }

    _analog_messages.clear();

    for (auto value : scaling_test_values)
    {
        state_change_register_midi_7bit(value);
    }

    wait_for_signals();
    analog_messages = _analog_messages.snapshot();
    std::cout << "Received " << analog_messages.size() << " messages" << std::endl;
    ASSERT_TRUE((analog::Collection::size(analog::GroupAnalogInputs) * scaling_test_values.size()) > analog_messages.size());

    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        ASSERT_TRUE(analog_messages.at(i).value >= SCALED_LOWER);
        ASSERT_TRUE(analog_messages.at(analog_messages.size() - analog::Collection::size(analog::GroupAnalogInputs) + i).value <= SCALED_UPPER);
    }

    // now enable inversion
    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::Invert, i, 1));
        reset_analog(i);
    }

    _analog_messages.clear();

    for (size_t i = 1; i < scaling_test_values.size(); i++)
    {
        state_change_register_midi_7bit(scaling_test_values.at(i));
    }

    wait_for_signals();
    analog_messages = _analog_messages.snapshot();

    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        ASSERT_TRUE(analog_messages.at(i).value >= SCALED_UPPER);
        ASSERT_TRUE(analog_messages.at(analog_messages.size() - analog::Collection::size(analog::GroupAnalogInputs) + i).value <= SCALED_LOWER);
    }
}

TEST_F(AnalogTest, SwitchForwarding)
{
    if (!analog::Collection::size(analog::GroupAnalogInputs))
    {
        return;
    }

    // configure one analog component to be switch type
    static constexpr size_t  SWITCH_INDEX        = 1;
    static constexpr uint8_t SWITCH_MIDI_CHANNEL = 2;
    static constexpr uint8_t SWITCH_VELOCITY     = 100;

    ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::Type, SWITCH_INDEX, analog::Type::Switch) == true);

    // configure switch with the same INDEX (+offset) to certain parameters
    ASSERT_TRUE(_database_admin.update(database::Config::Section::Switch::Type, switches::Collection::size(switches::GroupDigitalInputs) + SWITCH_INDEX, switches::Type::Momentary));
    ASSERT_TRUE(_database_admin.update(database::Config::Section::Switch::Channel, switches::Collection::size(switches::GroupDigitalInputs) + SWITCH_INDEX, SWITCH_MIDI_CHANNEL));
    ASSERT_TRUE(_database_admin.update(database::Config::Section::Switch::MessageType, switches::Collection::size(switches::GroupDigitalInputs) + SWITCH_INDEX, switches::MessageType::ControlChangeReset));
    ASSERT_TRUE(_database_admin.update(database::Config::Section::Switch::Value, switches::Collection::size(switches::GroupDigitalInputs) + SWITCH_INDEX, SWITCH_VELOCITY));

    state_change_register(0xFFFF);
    wait_for_signals();
    auto analog_switch_messages = _analogSwitch_messages.snapshot();

    ASSERT_EQ(1, analog_switch_messages.size());
    EXPECT_EQ(SWITCH_INDEX, analog_switch_messages.at(0).component_index);

    state_change_register(0);
    wait_for_signals();
    analog_switch_messages = _analogSwitch_messages.snapshot();
    auto switch_messages   = _switch_messages.snapshot();

    // In CONTROL_CHANGE_RESET mode the switch path emits once on press and
    // once on release.
    ASSERT_EQ(2, analog_switch_messages.size());
    EXPECT_EQ(SWITCH_INDEX, analog_switch_messages.at(1).component_index);
    ASSERT_EQ(2, switch_messages.size());
    EXPECT_EQ(switches::Collection::size(switches::GroupDigitalInputs) + SWITCH_INDEX,
              switch_messages.at(0).component_index);
    EXPECT_EQ(switches::Collection::size(switches::GroupDigitalInputs) + SWITCH_INDEX,
              switch_messages.at(1).component_index);

    // repeat the same input frame - analog class sends forwarding message again since it's not in charge of filtering it
    state_change_register(0);
    wait_for_signals();
    analog_switch_messages = _analogSwitch_messages.snapshot();

    EXPECT_EQ(3, analog_switch_messages.size());

    // similar test with the switch message type being normal CC
    _analogSwitch_messages.clear();
    _switch_messages.clear();

    ASSERT_TRUE(_database_admin.update(database::Config::Section::Switch::MessageType, switches::Collection::size(switches::GroupDigitalInputs) + SWITCH_INDEX, switches::MessageType::ControlChange));

    state_change_register(0xFFFF);
    wait_for_signals();
    analog_switch_messages = _analogSwitch_messages.snapshot();
    switch_messages        = _switch_messages.snapshot();

    ASSERT_EQ(1, analog_switch_messages.size());
    EXPECT_EQ(SWITCH_INDEX, analog_switch_messages.at(0).component_index);
    ASSERT_EQ(1, switch_messages.size());

    state_change_register(0);
    wait_for_signals();
    analog_switch_messages = _analogSwitch_messages.snapshot();
    switch_messages        = _switch_messages.snapshot();

    EXPECT_EQ(2, analog_switch_messages.size());
    EXPECT_EQ(1, switch_messages.size());
}

TEST_F(AnalogTest, FsrPublishesOnlyWhenMappedValueBecomesNonZeroAndOnRelease)
{
    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::Type, i, analog::Type::Fsr));
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::LowerLimit, i, 0));
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::UpperLimit, i, 127));
        reset_analog(i);
    }

    state_change_register(1);
    wait_for_signals();

    auto analog_messages = _analog_messages.snapshot();

    EXPECT_TRUE(analog_messages.empty());

    _analog_messages.clear();

    state_change_register(analog::Filter::POSITION_MAX_VALUE);
    wait_for_signals();

    analog_messages = _analog_messages.snapshot();

    ASSERT_EQ(analog::Collection::size(analog::GroupAnalogInputs), analog_messages.size());

    for (const auto& message : analog_messages)
    {
        EXPECT_EQ(midi::MessageType::NoteOn, message.message);
        EXPECT_EQ(127, message.value);
    }

    _analog_messages.clear();

    state_change_register(0);
    wait_for_signals();

    analog_messages = _analog_messages.snapshot();

    ASSERT_EQ(analog::Collection::size(analog::GroupAnalogInputs), analog_messages.size());

    for (const auto& message : analog_messages)
    {
        EXPECT_EQ(midi::MessageType::NoteOff, message.message);
        EXPECT_EQ(0, message.value);
    }
}

TEST_F(AnalogTest, DrainsMultipleQueuedFramesInSingleUpdate)
{
    state_change_register(std::vector<uint16_t>{
        position_for_midi_value(10, midi::MAX_VALUE_7BIT),
        position_for_midi_value(11, midi::MAX_VALUE_7BIT),
    });
    wait_for_signals();

    auto analog_messages = _analog_messages.snapshot();

    ASSERT_EQ((analog::Collection::size(analog::GroupAnalogInputs) * 2), analog_messages.size());

    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        EXPECT_EQ(10, analog_messages.at(i).value);
        EXPECT_EQ(11, analog_messages.at(i + analog::Collection::size(analog::GroupAnalogInputs)).value);
    }
}

TEST_F(AnalogTest, ForceRefreshUsesLastValueWithoutNewFrames)
{
    state_change_register_midi_7bit(17);
    wait_for_signals();
    clear_messages();

    _analog._hwa.clear_read_count();

    _analog._instance.force_refresh(0, _analog._instance.refreshable_components());
    wait_for_signals();
    EXPECT_EQ(0, _analog._hwa.read_count());

    auto analog_messages = _analog_messages.snapshot();

    ASSERT_EQ(analog::Collection::size(analog::GroupAnalogInputs), analog_messages.size());

    for (const auto& message : analog_messages)
    {
        EXPECT_EQ(17, message.value);
    }
}

TEST_F(AnalogTest, EnableChangesUpdatePhysicalScanMask)
{
    if (!analog::Collection::size(analog::GroupAnalogInputs))
    {
        return;
    }

    analog::ScanMask expected_mask = _analog._hwa.scan_mask();

    ASSERT_EQ(static_cast<uint8_t>(sys::Config::Status::Ack),
              ConfigHandler.set(sys::Config::Block::Analog,
                                static_cast<uint8_t>(sys::Config::Section::Analog::Enable),
                                0,
                                1));

    expected_mask[analog::Remap::physical(0)] = true;
    EXPECT_EQ(expected_mask, _analog._hwa.scan_mask());

    ASSERT_EQ(static_cast<uint8_t>(sys::Config::Status::Ack),
              ConfigHandler.set(sys::Config::Block::Analog,
                                static_cast<uint8_t>(sys::Config::Section::Analog::Enable),
                                0,
                                0));

    expected_mask[analog::Remap::physical(0)] = false;
    EXPECT_EQ(expected_mask, _analog._hwa.scan_mask());
}

TEST_F(AnalogTest, PresetChangeUpdatesPhysicalScanMask)
{
    if (_database_admin.supported_presets() < 2)
    {
        return;
    }

    analog::ScanMask expected_mask = _analog._hwa.scan_mask();

    ASSERT_TRUE(_database_admin.set_preset(1));

    for (size_t i = 0; i < analog::Collection::size(analog::GroupAnalogInputs); i++)
    {
        ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::Enable, i, 1));
    }

    ASSERT_TRUE(_analog._database.update(database::Config::Section::Analog::Enable, 0, 0));

    ASSERT_TRUE(_database_admin.set_preset(0));
    signaling::publish(signaling::SystemSignal{
        .system_event = signaling::SystemEvent::PresetChanged,
        .value        = 0,
    });

    expected_mask[analog::Remap::physical(0)] = true;
    EXPECT_EQ(expected_mask, _analog._hwa.scan_mask());

    ASSERT_TRUE(_database_admin.set_preset(1));
    signaling::publish(signaling::SystemSignal{
        .system_event = signaling::SystemEvent::PresetChanged,
        .value        = 1,
    });

    expected_mask[analog::Remap::physical(0)] = false;
    EXPECT_EQ(expected_mask, _analog._hwa.scan_mask());
}

#else
TEST(AnalogTest, SkippedWhenPresetDoesNotSupportAdc)
{
    SUCCEED();
}
#endif
