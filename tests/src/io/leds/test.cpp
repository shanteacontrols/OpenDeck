#include "tests/Common.h"
#include "tests/stubs/LEDs.h"
#include "tests/stubs/Listener.h"
#include "application/util/configurable/Configurable.h"

#ifdef LEDS_SUPPORTED

using namespace io;

namespace
{
    class LEDsTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_leds._databaseAdmin.init());
            ASSERT_TRUE(_leds._databaseAdmin.factoryReset());
            ASSERT_EQ(0, _leds._databaseAdmin.getPreset());

            // LEDs calls HWA only for digital out group - for the other groups controls is done via dispatcher.
            // Once init() is called, all LEDs should be turned off
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
                .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

            _leds._instance.init();
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            MIDIDispatcher.clear();
        }

        static constexpr size_t MIDI_CHANNEL = 1;

        // these tables should match with the one at https://github.com/shanteacontrols/OpenDeck/wiki/LED-control
        std::vector<LEDs::brightness_t> expectedBrightnessValue = {
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::OFF,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
            LEDs::brightness_t::B25,
            LEDs::brightness_t::B50,
            LEDs::brightness_t::B75,
            LEDs::brightness_t::B100,
        };

        std::vector<LEDs::blinkSpeed_t> expectedBlinkSpeedValue = {
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S1000MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S500MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::S250MS,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
            LEDs::blinkSpeed_t::NO_BLINK,
        };

        TestLEDs _leds;
    };
}    // namespace

TEST_F(LEDsTest, MultiValue)
{
    if (!LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        return;
    }

    // MIDI_IN_NOTE_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < LEDs::Collection::SIZE(); i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::MIDI_IN_NOTE_MULTI_VAL));
    }

    for (size_t led = 0; led < LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (uint8_t value = 0; value < 128; value++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(value)))
                .Times(1);

            MIDIDispatcher.notify(messaging::eventType_t::MIDI_IN,
                                  { 0,                             // componentIndex - irrelevant
                                    MIDI_CHANNEL,                  // midiChannel
                                    static_cast<uint16_t>(led),    // midiIndex
                                    value,                         // midiValue
                                    0,                             // sysEx
                                    0,                             // sysExLength
                                    MIDI::messageType_t::NOTE_ON });

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    // MIDI_IN_CC_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < LEDs::Collection::SIZE(); i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::MIDI_IN_CC_MULTI_VAL));
    }

    for (size_t led = 0; led < LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (uint8_t value = 0; value < 128; value++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(value)))
                .Times(1);

            MIDIDispatcher.notify(messaging::eventType_t::MIDI_IN,
                                  { 0,                             // componentIndex - irrelevant
                                    MIDI_CHANNEL,                  // midiChannel
                                    static_cast<uint16_t>(led),    // midiIndex
                                    value,                         // midiValue
                                    0,                             // sysEx
                                    0,                             // sysExLength
                                    MIDI::messageType_t::CONTROL_CHANGE });

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    // LOCAL_NOTE_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < LEDs::Collection::SIZE(); i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::LOCAL_NOTE_MULTI_VAL));
    }

    for (size_t led = 0; led < LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (uint8_t value = 0; value < 128; value++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(value)))
                .Times(1);

            MIDIDispatcher.notify(messaging::eventType_t::BUTTON,
                                  { 0,                             // componentIndex - irrelevant
                                    MIDI_CHANNEL,                  // midiChannel
                                    static_cast<uint16_t>(led),    // midiIndex
                                    value,                         // midiValue
                                    0,                             // sysEx
                                    0,                             // sysExLength
                                    MIDI::messageType_t::NOTE_ON });

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    // same test for analog components
    for (size_t led = 0; led < LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (uint8_t value = 0; value < 128; value++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(value)))
                .Times(1);

            MIDIDispatcher.notify(messaging::eventType_t::ANALOG,
                                  { 0,                             // componentIndex - irrelevant
                                    MIDI_CHANNEL,                  // midiChannel
                                    static_cast<uint16_t>(led),    // midiIndex
                                    value,                         // midiValue
                                    0,                             // sysEx
                                    0,                             // sysExLength
                                    MIDI::messageType_t::NOTE_ON });

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    // LOCAL_CC_MULTI_VAL
    //----------------------------------

    for (size_t i = 0; i < LEDs::Collection::SIZE(); i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::LOCAL_CC_MULTI_VAL));
    }

    for (size_t led = 0; led < LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (uint8_t value = 0; value < 128; value++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(value)))
                .Times(1);

            MIDIDispatcher.notify(messaging::eventType_t::BUTTON,
                                  { 0,                             // componentIndex - irrelevant
                                    MIDI_CHANNEL,                  // midiChannel
                                    static_cast<uint16_t>(led),    // midiIndex
                                    value,                         // midiValue
                                    0,                             // sysEx
                                    0,                             // sysExLength
                                    MIDI::messageType_t::CONTROL_CHANGE });

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    for (size_t led = 0; led < LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (uint8_t value = 0; value < 128; value++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(value)))
                .Times(1);

            MIDIDispatcher.notify(messaging::eventType_t::ANALOG,
                                  { 0,                             // componentIndex - irrelevant
                                    MIDI_CHANNEL,                  // midiChannel
                                    static_cast<uint16_t>(led),    // midiIndex
                                    value,                         // midiValue
                                    0,                             // sysEx
                                    0,                             // sysExLength
                                    MIDI::messageType_t::CONTROL_CHANGE });

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }
}

TEST_F(LEDsTest, SingleValue)
{
    if (!LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        return;
    }

    // MIDI_IN_NOTE_SINGLE_VAL
    //----------------------------------

    for (uint8_t activationValue = 0; activationValue < 128; activationValue++)
    {
        for (size_t i = 0; i < LEDs::Collection::SIZE(); i++)
        {
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::MIDI_IN_NOTE_SINGLE_VAL));
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::ACTIVATION_VALUE, i, activationValue));
        }

        for (size_t led = 0; led < LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, LEDs::brightness_t::OFF))
                .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

            _leds._instance.setAllOff();

            for (uint8_t value = 0; value < 128; value++)
            {
                EXPECT_CALL(_leds._hwa, setState(_, value == activationValue ? LEDs::brightness_t::B100 : LEDs::brightness_t::OFF))
                    .Times(1);

                MIDIDispatcher.notify(messaging::eventType_t::MIDI_IN,
                                      { 0,                             // componentIndex - irrelevant
                                        MIDI_CHANNEL,                  // midiChannel
                                        static_cast<uint16_t>(led),    // midiIndex
                                        value,                         // midiValue
                                        0,                             // sysEx
                                        0,                             // sysExLength
                                        MIDI::messageType_t::NOTE_ON });

                ASSERT_EQ(LEDs::blinkSpeed_t::NO_BLINK, _leds._instance.blinkSpeed(led));
            }
        }
    }

    // MIDI_IN_CC_SINGLE_VAL
    //----------------------------------

    for (uint8_t activationValue = 0; activationValue < 128; activationValue++)
    {
        for (size_t i = 0; i < LEDs::Collection::SIZE(); i++)
        {
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::MIDI_IN_CC_SINGLE_VAL));
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::ACTIVATION_VALUE, i, activationValue));
        }

        for (size_t led = 0; led < LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, LEDs::brightness_t::OFF))
                .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

            _leds._instance.setAllOff();

            for (uint8_t value = 0; value < 128; value++)
            {
                EXPECT_CALL(_leds._hwa, setState(_, value == activationValue ? LEDs::brightness_t::B100 : LEDs::brightness_t::OFF))
                    .Times(1);

                MIDIDispatcher.notify(messaging::eventType_t::MIDI_IN,
                                      { 0,                             // componentIndex - irrelevant
                                        MIDI_CHANNEL,                  // midiChannel
                                        static_cast<uint16_t>(led),    // midiIndex
                                        value,                         // midiValue
                                        0,                             // sysEx
                                        0,                             // sysExLength
                                        MIDI::messageType_t::CONTROL_CHANGE });

                ASSERT_EQ(LEDs::blinkSpeed_t::NO_BLINK, _leds._instance.blinkSpeed(led));
            }
        }
    }

    // LOCAL_NOTE_SINGLE_VAL
    //----------------------------------

    for (uint8_t activationValue = 0; activationValue < 128; activationValue++)
    {
        for (size_t i = 0; i < LEDs::Collection::SIZE(); i++)
        {
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::LOCAL_NOTE_SINGLE_VAL));
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::ACTIVATION_VALUE, i, activationValue));
        }

        for (size_t led = 0; led < LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, LEDs::brightness_t::OFF))
                .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

            _leds._instance.setAllOff();

            for (uint8_t value = 0; value < 128; value++)
            {
                EXPECT_CALL(_leds._hwa, setState(_, value == activationValue ? LEDs::brightness_t::B100 : LEDs::brightness_t::OFF))
                    .Times(1);

                MIDIDispatcher.notify(messaging::eventType_t::BUTTON,
                                      { 0,                             // componentIndex - irrelevant
                                        MIDI_CHANNEL,                  // midiChannel
                                        static_cast<uint16_t>(led),    // midiIndex
                                        value,                         // midiValue
                                        0,                             // sysEx
                                        0,                             // sysExLength
                                        MIDI::messageType_t::NOTE_ON });

                ASSERT_EQ(LEDs::blinkSpeed_t::NO_BLINK, _leds._instance.blinkSpeed(led));
            }
        }
    }

    // LOCAL_CC_SINGLE_VAL
    //----------------------------------

    for (uint8_t activationValue = 0; activationValue < 128; activationValue++)
    {
        for (size_t i = 0; i < LEDs::Collection::SIZE(); i++)
        {
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::LOCAL_CC_SINGLE_VAL));
            ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::ACTIVATION_VALUE, i, activationValue));
        }

        for (size_t led = 0; led < LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, LEDs::brightness_t::OFF))
                .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

            _leds._instance.setAllOff();

            for (uint8_t value = 0; value < 128; value++)
            {
                EXPECT_CALL(_leds._hwa, setState(_, value == activationValue ? LEDs::brightness_t::B100 : LEDs::brightness_t::OFF))
                    .Times(1);

                MIDIDispatcher.notify(messaging::eventType_t::BUTTON,
                                      { 0,                             // componentIndex - irrelevant
                                        MIDI_CHANNEL,                  // midiChannel
                                        static_cast<uint16_t>(led),    // midiIndex
                                        value,                         // midiValue
                                        0,                             // sysEx
                                        0,                             // sysExLength
                                        MIDI::messageType_t::CONTROL_CHANGE });

                ASSERT_EQ(LEDs::blinkSpeed_t::NO_BLINK, _leds._instance.blinkSpeed(led));
            }
        }
    }
}

TEST_F(LEDsTest, SingleLEDstate)
{
    if (!LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        return;
    }

    static constexpr size_t MIDI_ID = 0;

    // By default, leds are configured to react on MIDI Note On.
    // Note 0 should turn the first LED on
    EXPECT_CALL(_leds._hwa, setState(MIDI_ID, LEDs::brightness_t::B100))
        .Times(1);

    MIDIDispatcher.notify(messaging::eventType_t::MIDI_IN,
                          {
                              0,
                              MIDI_CHANNEL,
                              MIDI_ID,
                              127,
                              0,
                              0,
                              MIDI::messageType_t::NOTE_ON,
                          });

    EXPECT_CALL(_leds._hwa, setState(MIDI_ID, LEDs::brightness_t::OFF))
        .Times(1);

    // now turn the LED off
    MIDIDispatcher.notify(messaging::eventType_t::MIDI_IN,
                          {
                              0,
                              MIDI_CHANNEL,
                              MIDI_ID,
                              0,
                              0,
                              0,
                              MIDI::messageType_t::NOTE_ON,
                          });

    if (LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS) < 3)
    {
        return;
    }

    // configure RGB LED 0
    ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::RGB_ENABLE, 0, 1));

    // now turn it on - expect three LEDs to be on

    EXPECT_CALL(_leds._hwa, setState(_leds._hwa.rgbComponentFromRGB(0, LEDs::rgbComponent_t::R), LEDs::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(_leds._hwa.rgbComponentFromRGB(0, LEDs::rgbComponent_t::G), LEDs::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(_leds._hwa.rgbComponentFromRGB(0, LEDs::rgbComponent_t::B), LEDs::brightness_t::B100))
        .Times(1);

    MIDIDispatcher.notify(messaging::eventType_t::MIDI_IN,
                          {
                              0,
                              MIDI_CHANNEL,
                              MIDI_ID,
                              127,
                              0,
                              0,
                              MIDI::messageType_t::NOTE_ON,
                          });
}

TEST_F(LEDsTest, ProgramChangeWithOffset)
{
    if (LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS) < 4)
    {
        return;
    }

    // configure first four LEDs to indicate program change
    static constexpr size_t PC_LEDS = 4;

    for (size_t i = 0; i < PC_LEDS; i++)
    {
        ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::PC_SINGLE_VAL));
    }

    // notify program change
    uint8_t program = 0;

    // first LED should be on, rest is off
    EXPECT_CALL(_leds._hwa, setState(_, LEDs::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(_, LEDs::brightness_t::OFF))
        .Times(3);

    MIDIDispatcher.notify(messaging::eventType_t::PROGRAM,
                          {
                              0,
                              MIDI_CHANNEL,
                              program,
                              0,    // value unused by program change
                              0,
                              0,
                              MIDI::messageType_t::PROGRAM_CHANGE,
                          });

    // now increase the program by 1
    program++;

    // second LED should be on, rest is off
    EXPECT_CALL(_leds._hwa, setState(0, LEDs::brightness_t::OFF))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(1, LEDs::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(2, LEDs::brightness_t::OFF))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(3, LEDs::brightness_t::OFF))
        .Times(1);

    MIDIDispatcher.notify(messaging::eventType_t::PROGRAM,
                          {
                              0,
                              MIDI_CHANNEL,
                              program,
                              0,    // value unused by program change
                              0,
                              0,
                              MIDI::messageType_t::PROGRAM_CHANGE,
                          });

    // change the MIDI program offset
    MIDIProgram.setOffset(1);

    // nothing should change yet
    // second LED should be on, rest is off
    EXPECT_CALL(_leds._hwa, setState(0, LEDs::brightness_t::OFF))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(1, LEDs::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(2, LEDs::brightness_t::OFF))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(3, LEDs::brightness_t::OFF))
        .Times(1);

    MIDIDispatcher.notify(messaging::eventType_t::PROGRAM,
                          {
                              0,
                              MIDI_CHANNEL,
                              program,
                              0,    // value unused by program change
                              0,
                              0,
                              MIDI::messageType_t::PROGRAM_CHANGE,
                          });

    // enable LED sync with offset
    ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::GLOBAL, LEDs::setting_t::USE_MIDI_PROGRAM_OFFSET, 1));

    // notify the program 1 again
    // this time, due to the offset, first LED should be on, and the rest should be off
    // when sync is active, all activation IDs for LEDs that use program change message type are incremented by the program offset
    EXPECT_CALL(_leds._hwa, setState(_, LEDs::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(_, LEDs::brightness_t::OFF))
        .Times(3);

    MIDIDispatcher.notify(messaging::eventType_t::PROGRAM,
                          {
                              0,
                              MIDI_CHANNEL,
                              program,
                              0,    // value unused by program change
                              0,
                              0,
                              MIDI::messageType_t::PROGRAM_CHANGE,
                          });
}

TEST_F(LEDsTest, StaticLEDsOnInitially)
{
    constexpr size_t LED_INDEX = 0;
    ASSERT_TRUE(_leds._database.update(database::Config::Section::leds_t::CONTROL_TYPE, LED_INDEX, LEDs::controlType_t::STATIC));

    // Once init() is called, all LEDs should be turned off
    EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
        .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

    // LED_INDEX should be turned on
    EXPECT_CALL(_leds._hwa, setState(LED_INDEX, LEDs::brightness_t::B100))
        .Times(1);

    _leds._instance.init();
}

#endif