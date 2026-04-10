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
#include "application/io/leds/builder.h"
#include "application/util/configurable/configurable.h"
#include "application/global/midi_program.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_LEDS

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

    class LEDsTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_databaseAdmin.init(_handlers));
            ASSERT_TRUE(_databaseAdmin.factoryReset());
            ASSERT_EQ(0, _databaseAdmin.currentPreset());

            // LEDs calls HWA only for digital out group - for the other groups controls is done via dispatcher.
            // Once init() is called, all LEDs should be turned off
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
                .Times(leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS));

            _leds._instance.init();
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            messaging::SignalRegistry<messaging::MidiSignal>::instance().clear();
            messaging::SignalRegistry<messaging::SystemSignal>::instance().clear();
            messaging::SignalRegistry<messaging::TouchscreenScreenSignal>::instance().clear();
            messaging::SignalRegistry<messaging::UmpSignal>::instance().clear();
            messaging::SignalRegistry<messaging::MidiTrafficSignal>::instance().clear();
        }

        void waitForSignalDispatch()
        {
            k_msleep(1);
        }

        static leds::brightness_t expectedBrightness(uint8_t value)
        {
            if (value < 16)
            {
                return leds::brightness_t::OFF;
            }

            return static_cast<leds::brightness_t>((value % 16 % 4) + 1);
        }

        static leds::blinkSpeed_t expectedBlinkSpeed(uint8_t value)
        {
            if (value < 16)
            {
                return leds::blinkSpeed_t::NO_BLINK;
            }

            return static_cast<leds::blinkSpeed_t>(value % 16 / 4);
        }

        void notifyMidiIn(midi::messageType_t message, uint8_t channel, uint16_t index, uint16_t value)
        {
            uint8_t command = 0;
            uint8_t p1      = static_cast<uint8_t>(index);
            uint8_t p2      = static_cast<uint8_t>(value);

            switch (message)
            {
            case midi::messageType_t::NOTE_ON:
                command = UMP_MIDI_NOTE_ON;
                break;

            case midi::messageType_t::NOTE_OFF:
                command = UMP_MIDI_NOTE_OFF;
                break;

            case midi::messageType_t::CONTROL_CHANGE:
                command = UMP_MIDI_CONTROL_CHANGE;
                break;

            case midi::messageType_t::PROGRAM_CHANGE:
                command = UMP_MIDI_PROGRAM_CHANGE;
                p2      = 0;
                break;

            default:
                return;
            }

            midi_ump packet = {};
            packet.data[0]  = (static_cast<uint32_t>(UMP_MT_MIDI1_CHANNEL_VOICE) << 28U) |
                              ((static_cast<uint32_t>(0U) & 0x0fU) << 24U) |
                              ((static_cast<uint32_t>(command) & 0x0fU) << 20U) |
                              ((static_cast<uint32_t>(channel - 1U) & 0x0fU) << 16U) |
                              ((static_cast<uint32_t>(p1) & 0x7fU) << 8U) |
                              (static_cast<uint32_t>(p2) & 0x7fU);

            messaging::publish(messaging::UmpSignal{
                .direction = messaging::MidiDirection::In,
                .packet    = packet,
            });

            waitForSignalDispatch();
        }

        void notifyLocal(messaging::MidiSource source,
                         midi::messageType_t   message,
                         uint8_t               channel,
                         uint16_t              index,
                         uint16_t              value)
        {
            messaging::publish(messaging::MidiSignal{
                .source         = source,
                .componentIndex = 0,
                .channel        = channel,
                .index          = index,
                .value          = value,
                .message        = message,
            });

            waitForSignalDispatch();
        }

        void notifyProgram(uint8_t channel, uint16_t program)
        {
            messaging::publish(messaging::MidiSignal{
                .source         = messaging::MidiSource::Program,
                .componentIndex = 0,
                .channel        = channel,
                .index          = program,
                .value          = 0,
                .message        = midi::messageType_t::PROGRAM_CHANGE,
            });

            waitForSignalDispatch();
        }

        static constexpr size_t                 MIDI_CHANNEL  = 1;
        static constexpr std::array<uint8_t, 3> SAMPLE_VALUES = { 0, 64, 127 };

        // these tables should match with the one at https://github.com/shanteacontrols/OpenDeck/wiki/LED-control
        std::vector<leds::brightness_t> expectedBrightnessValue = {
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::OFF,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
            leds::brightness_t::B25,
            leds::brightness_t::B50,
            leds::brightness_t::B75,
            leds::brightness_t::B100,
        };

        std::vector<leds::blinkSpeed_t> expectedBlinkSpeedValue = {
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S1000MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S500MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::S250MS,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
            leds::blinkSpeed_t::NO_BLINK,
        };

        NoOpHandlers      _handlers;
        database::Builder _builderDatabase;
        database::Admin&  _databaseAdmin = _builderDatabase.instance();
        leds::Builder     _leds          = leds::Builder(_databaseAdmin);
    };
}    // namespace

TEST_F(LEDsTest, MultiValue)
{
    if (!leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS))
    {
        return;
    }

    // MIDI_IN_NOTE_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < leds::Collection::SIZE(); i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, leds::controlType_t::MIDI_IN_NOTE_MULTI_VAL));
    }

    for (size_t led = 0; led < leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightness(value)))
                .Times(1);

            notifyMidiIn(midi::messageType_t::NOTE_ON, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

            ASSERT_EQ(expectedBlinkSpeed(value), _leds._instance.blinkSpeed(led));
        }
    }

    // MIDI_IN_CC_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < leds::Collection::SIZE(); i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, leds::controlType_t::MIDI_IN_CC_MULTI_VAL));
    }

    for (size_t led = 0; led < leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightness(value)))
                .Times(1);

            notifyMidiIn(midi::messageType_t::CONTROL_CHANGE, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

            ASSERT_EQ(expectedBlinkSpeed(value), _leds._instance.blinkSpeed(led));
        }
    }

    // LOCAL_NOTE_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < leds::Collection::SIZE(); i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, leds::controlType_t::LOCAL_NOTE_MULTI_VAL));
    }

    for (size_t led = 0; led < leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightness(value)))
                .Times(1);

            notifyLocal(messaging::MidiSource::Button, midi::messageType_t::NOTE_ON, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

            ASSERT_EQ(expectedBlinkSpeed(value), _leds._instance.blinkSpeed(led));
        }
    }

    // same test for analog components
    for (size_t led = 0; led < leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightness(value)))
                .Times(1);

            notifyLocal(messaging::MidiSource::Analog, midi::messageType_t::NOTE_ON, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

            ASSERT_EQ(expectedBlinkSpeed(value), _leds._instance.blinkSpeed(led));
        }
    }

    // LOCAL_CC_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < leds::Collection::SIZE(); i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, leds::controlType_t::LOCAL_CC_MULTI_VAL));
    }

    for (size_t led = 0; led < leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightness(value)))
                .Times(1);

            notifyLocal(messaging::MidiSource::Button, midi::messageType_t::CONTROL_CHANGE, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

            ASSERT_EQ(expectedBlinkSpeed(value), _leds._instance.blinkSpeed(led));
        }
    }

    for (size_t led = 0; led < leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (auto value : SAMPLE_VALUES)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightness(value)))
                .Times(1);

            notifyLocal(messaging::MidiSource::Analog, midi::messageType_t::CONTROL_CHANGE, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

            ASSERT_EQ(expectedBlinkSpeed(value), _leds._instance.blinkSpeed(led));
        }
    }
}

TEST_F(LEDsTest, SingleValue)
{
    if (!leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS))
    {
        return;
    }

    // MIDI_IN_NOTE_SINGLE_VAL
    //----------------------------------

    for (auto activationValue : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < leds::Collection::SIZE(); i++)
        {
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, leds::controlType_t::MIDI_IN_NOTE_SINGLE_VAL));
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::ACTIVATION_VALUE, i, activationValue));
        }

        for (size_t led = 0; led < leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS); led++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, leds::brightness_t::OFF))
                .Times(leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS));

            _leds._instance.setAllOff();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_leds._hwa, setState(_, value == activationValue ? leds::brightness_t::B100 : leds::brightness_t::OFF))
                    .Times(1);

                notifyMidiIn(midi::messageType_t::NOTE_ON, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

                ASSERT_EQ(leds::blinkSpeed_t::NO_BLINK, _leds._instance.blinkSpeed(led));
            }
        }
    }

    // MIDI_IN_CC_SINGLE_VAL
    //----------------------------------

    for (auto activationValue : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < leds::Collection::SIZE(); i++)
        {
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, leds::controlType_t::MIDI_IN_CC_SINGLE_VAL));
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::ACTIVATION_VALUE, i, activationValue));
        }

        for (size_t led = 0; led < leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS); led++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, leds::brightness_t::OFF))
                .Times(leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS));

            _leds._instance.setAllOff();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_leds._hwa, setState(_, value == activationValue ? leds::brightness_t::B100 : leds::brightness_t::OFF))
                    .Times(1);

                notifyMidiIn(midi::messageType_t::CONTROL_CHANGE, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

                ASSERT_EQ(leds::blinkSpeed_t::NO_BLINK, _leds._instance.blinkSpeed(led));
            }
        }
    }

    // LOCAL_NOTE_SINGLE_VAL
    //----------------------------------

    for (auto activationValue : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < leds::Collection::SIZE(); i++)
        {
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, leds::controlType_t::LOCAL_NOTE_SINGLE_VAL));
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::ACTIVATION_VALUE, i, activationValue));
        }

        for (size_t led = 0; led < leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS); led++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, leds::brightness_t::OFF))
                .Times(leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS));

            _leds._instance.setAllOff();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_leds._hwa, setState(_, value == activationValue ? leds::brightness_t::B100 : leds::brightness_t::OFF))
                    .Times(1);

                notifyLocal(messaging::MidiSource::Button, midi::messageType_t::NOTE_ON, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

                ASSERT_EQ(leds::blinkSpeed_t::NO_BLINK, _leds._instance.blinkSpeed(led));
            }
        }
    }

    // LOCAL_CC_SINGLE_VAL
    //----------------------------------

    for (auto activationValue : SAMPLE_VALUES)
    {
        for (size_t i = 0; i < leds::Collection::SIZE(); i++)
        {
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, leds::controlType_t::LOCAL_CC_SINGLE_VAL));
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::ACTIVATION_VALUE, i, activationValue));
        }

        for (size_t led = 0; led < leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS); led++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, leds::brightness_t::OFF))
                .Times(leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS));

            _leds._instance.setAllOff();

            for (auto value : SAMPLE_VALUES)
            {
                EXPECT_CALL(_leds._hwa, setState(_, value == activationValue ? leds::brightness_t::B100 : leds::brightness_t::OFF))
                    .Times(1);

                notifyLocal(messaging::MidiSource::Button, midi::messageType_t::CONTROL_CHANGE, MIDI_CHANNEL, static_cast<uint16_t>(led), value);

                ASSERT_EQ(leds::blinkSpeed_t::NO_BLINK, _leds._instance.blinkSpeed(led));
            }
        }
    }
}

TEST_F(LEDsTest, SingleLEDstate)
{
    if (!leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS))
    {
        return;
    }

    static constexpr size_t MIDI_ID = 0;

    // By default, leds are configured to react on MIDI Note On.
    // Note 0 should turn the first LED on
    EXPECT_CALL(_leds._hwa, setState(MIDI_ID, leds::brightness_t::B100))
        .Times(1);

    notifyMidiIn(midi::messageType_t::NOTE_ON, MIDI_CHANNEL, MIDI_ID, 127);

    EXPECT_CALL(_leds._hwa, setState(MIDI_ID, leds::brightness_t::OFF))
        .Times(1);

    // now turn the LED off
    notifyMidiIn(midi::messageType_t::NOTE_ON, MIDI_CHANNEL, MIDI_ID, 0);

    if (leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS) < 3)
    {
        return;
    }

    // configure RGB LED 0
    ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::RGB_ENABLE, 0, 1));

    // now turn it on - expect three LEDs to be on

    EXPECT_CALL(_leds._hwa, setState(_leds._hwa.rgbComponentFromRgb(0, leds::rgbComponent_t::R), leds::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(_leds._hwa.rgbComponentFromRgb(0, leds::rgbComponent_t::G), leds::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(_leds._hwa.rgbComponentFromRgb(0, leds::rgbComponent_t::B), leds::brightness_t::B100))
        .Times(1);

    notifyMidiIn(midi::messageType_t::NOTE_ON, MIDI_CHANNEL, MIDI_ID, 127);
}

TEST_F(LEDsTest, ProgramChangeWithOffset)
{
    if (leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS) < 4)
    {
        return;
    }

    // configure first four LEDs to indicate program change
    static constexpr size_t PC_LEDS = 4;

    for (size_t i = 0; i < PC_LEDS; i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, leds::controlType_t::PC_SINGLE_VAL));
    }

    // notify program change
    uint8_t program = 0;

    // first LED should be on, rest is off
    EXPECT_CALL(_leds._hwa, setState(_, leds::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(_, leds::brightness_t::OFF))
        .Times(3);

    notifyProgram(MIDI_CHANNEL, program);

    // now increase the program by 1
    program++;

    // second LED should be on, rest is off
    EXPECT_CALL(_leds._hwa, setState(0, leds::brightness_t::OFF))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(1, leds::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(2, leds::brightness_t::OFF))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(3, leds::brightness_t::OFF))
        .Times(1);

    notifyProgram(MIDI_CHANNEL, program);

    // change the MIDI program offset
    MidiProgram.setOffset(1);

    // nothing should change yet
    // second LED should be on, rest is off
    EXPECT_CALL(_leds._hwa, setState(0, leds::brightness_t::OFF))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(1, leds::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(2, leds::brightness_t::OFF))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(3, leds::brightness_t::OFF))
        .Times(1);

    notifyProgram(MIDI_CHANNEL, program);

    // enable LED sync with offset
    ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::GLOBAL, leds::setting_t::USE_MIDI_PROGRAM_OFFSET, 1));

    // notify the program 1 again
    // this time, due to the offset, first LED should be on, and the rest should be off
    // when sync is active, all activation IDs for LEDs that use program change message type are incremented by the program offset
    EXPECT_CALL(_leds._hwa, setState(_, leds::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(_, leds::brightness_t::OFF))
        .Times(3);

    notifyProgram(MIDI_CHANNEL, program);
}

TEST_F(LEDsTest, StaticLEDsOnInitially)
{
    constexpr size_t LED_INDEX = 0;
    ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, LED_INDEX, leds::controlType_t::STATIC));

    // Once init() is called, all LEDs should be turned off
    EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
        .Times(leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS));

    // LED_INDEX should be turned on
    EXPECT_CALL(_leds._hwa, setState(LED_INDEX, leds::brightness_t::B100))
        .Times(1);

    _leds._instance.init();
}

TEST_F(LEDsTest, GlobalChannel)
{
    if (!leds::Collection::SIZE(leds::GROUP_DIGITAL_OUTPUTS))
    {
        return;
    }

    constexpr auto LED_INDEX       = 0;
    const auto     DEFAULT_CHANNEL = _leds._database.read(database::Config::Section::leds_t::CHANNEL, LED_INDEX);
    const auto     GLOBAL_CHANNEL  = DEFAULT_CHANNEL + 1;
    constexpr auto ON_VALUE        = 127;

    ASSERT_TRUE(_leds._database.update(database::Config::Section::global_t::MIDI_SETTINGS, midi::setting_t::GLOBAL_CHANNEL, GLOBAL_CHANNEL));
    ASSERT_TRUE(_leds._database.update(database::Config::Section::global_t::MIDI_SETTINGS, midi::setting_t::USE_GLOBAL_CHANNEL, true));
    ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, LED_INDEX, leds::controlType_t::MIDI_IN_NOTE_MULTI_VAL));

    EXPECT_CALL(_leds._hwa, setState(_, _))
        .Times(0);

    // this shouldn't turn the led on because global channel is used instead of the default one

    notifyMidiIn(midi::messageType_t::NOTE_ON, DEFAULT_CHANNEL, LED_INDEX, ON_VALUE);

    // verify that the led LED_INDEX is turned on with global channel
    EXPECT_CALL(_leds._hwa, setState(_, expectedBrightness(ON_VALUE)))
        .Times(1);

    notifyMidiIn(midi::messageType_t::NOTE_ON, GLOBAL_CHANNEL, LED_INDEX, ON_VALUE);

    // disable the global channel but now use omni channel instead
    ASSERT_TRUE(_leds._database.update(database::Config::Section::global_t::MIDI_SETTINGS, midi::setting_t::USE_GLOBAL_CHANNEL, false));
    ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CHANNEL, LED_INDEX, midi::OMNI_CHANNEL));

    for (size_t i = 0; i < 16; i++)
    {
        // the led should be turned on for every received channel
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightness(ON_VALUE)))
            .Times(1);

        notifyMidiIn(midi::messageType_t::NOTE_ON, i, LED_INDEX, ON_VALUE);
    }

    // same test, but this time use omni as global channel
    ASSERT_TRUE(_leds._database.update(database::Config::Section::global_t::MIDI_SETTINGS, midi::setting_t::GLOBAL_CHANNEL, midi::OMNI_CHANNEL));
    ASSERT_TRUE(_leds._database.update(database::Config::Section::global_t::MIDI_SETTINGS, midi::setting_t::USE_GLOBAL_CHANNEL, true));
    ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CHANNEL, LED_INDEX, 1));

    for (size_t i = 0; i < 16; i++)
    {
        // the led should be turned on for every received channel
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightness(ON_VALUE)))
            .Times(1);

        notifyMidiIn(midi::messageType_t::NOTE_ON, i, LED_INDEX, ON_VALUE);
    }
}

#endif
