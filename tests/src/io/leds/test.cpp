#include "framework/Framework.h"
#include "stubs/LEDs.h"
#include "stubs/Listener.h"

#ifdef LEDS_SUPPORTED

using namespace IO;

namespace
{
    class LEDsTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_leds._database.init());
            ASSERT_TRUE(_leds._database.factoryReset());
            ASSERT_EQ(0, _leds._database.getPreset());

            // LEDs calls HWA only for digital out group - for the other groups controls is done via dispatcher.
            // Once init() is called, all LEDs should be turned off
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
                .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

            _leds._instance.init();
        }

        void TearDown() override
        {
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

TEST_F(LEDsTest, VerifyBrightnessAndBlinkSpeed)
{
    if (!LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        return;
    }

    // MIDI_IN_NOTE_MULTI_VAL
    //----------------------------------

    for (int i = 0; i < LEDs::Collection::size(); i++)
    {
        ASSERT_TRUE(_leds._database.update(Database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::MIDI_IN_NOTE_MULTI_VAL));
    }

    for (uint8_t led = 0; led < LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (uint8_t value = 0; value < 128; value++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(value)))
                .Times(1);

            MIDIDispatcher.notify(Messaging::eventType_t::MIDI_IN,
                                  { 0,               // componentIndex - irrelevant
                                    MIDI_CHANNEL,    // midiChannel
                                    led,             // midiIndex
                                    value,           // midiValue
                                    0,               // sysEx
                                    0,               // sysExLength
                                    MIDI::messageType_t::NOTE_ON });

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    // MIDI_IN_CC_MULTI_VAL
    //----------------------------------

    for (int i = 0; i < LEDs::Collection::size(); i++)
    {
        ASSERT_TRUE(_leds._database.update(Database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::MIDI_IN_CC_MULTI_VAL));
    }

    for (uint8_t led = 0; led < LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (uint8_t value = 0; value < 128; value++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(value)))
                .Times(1);

            MIDIDispatcher.notify(Messaging::eventType_t::MIDI_IN,
                                  { 0,               // componentIndex - irrelevant
                                    MIDI_CHANNEL,    // midiChannel
                                    led,             // midiIndex
                                    value,           // midiValue
                                    0,               // sysEx
                                    0,               // sysExLength
                                    MIDI::messageType_t::CONTROL_CHANGE });

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    // LOCAL_NOTE_MULTI_VAL
    //----------------------------------

    for (int i = 0; i < LEDs::Collection::size(); i++)
    {
        ASSERT_TRUE(_leds._database.update(Database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::LOCAL_NOTE_MULTI_VAL));
    }

    for (uint8_t led = 0; led < LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (uint8_t value = 0; value < 128; value++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(value)))
                .Times(1);

            MIDIDispatcher.notify(Messaging::eventType_t::BUTTON,
                                  { 0,               // componentIndex - irrelevant
                                    MIDI_CHANNEL,    // midiChannel
                                    led,             // midiIndex
                                    value,           // midiValue
                                    0,               // sysEx
                                    0,               // sysExLength
                                    MIDI::messageType_t::NOTE_ON });

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    // same test for analog components
    for (uint8_t led = 0; led < LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (uint8_t value = 0; value < 128; value++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(value)))
                .Times(1);

            MIDIDispatcher.notify(Messaging::eventType_t::ANALOG,
                                  { 0,               // componentIndex - irrelevant
                                    MIDI_CHANNEL,    // midiChannel
                                    led,             // midiIndex
                                    value,           // midiValue
                                    0,               // sysEx
                                    0,               // sysExLength
                                    MIDI::messageType_t::NOTE_ON });

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    // localCCMultiVal
    //----------------------------------

    for (int i = 0; i < LEDs::Collection::size(); i++)
    {
        ASSERT_TRUE(_leds._database.update(Database::Config::Section::leds_t::CONTROL_TYPE, i, LEDs::controlType_t::LOCAL_CC_MULTI_VAL));
    }

    for (uint8_t led = 0; led < LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (uint8_t value = 0; value < 128; value++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(value)))
                .Times(1);

            MIDIDispatcher.notify(Messaging::eventType_t::BUTTON,
                                  { 0,               // componentIndex - irrelevant
                                    MIDI_CHANNEL,    // midiChannel
                                    led,             // midiIndex
                                    value,           // midiValue
                                    0,               // sysEx
                                    0,               // sysExLength
                                    MIDI::messageType_t::CONTROL_CHANGE });

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    for (uint8_t led = 0; led < LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS); led++)
    {
        EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(0)))
            .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

        _leds._instance.setAllOff();

        for (uint8_t value = 0; value < 128; value++)
        {
            EXPECT_CALL(_leds._hwa, setState(_, expectedBrightnessValue.at(value)))
                .Times(1);

            MIDIDispatcher.notify(Messaging::eventType_t::ANALOG,
                                  { 0,               // componentIndex - irrelevant
                                    MIDI_CHANNEL,    // midiChannel
                                    led,             // midiIndex
                                    value,           // midiValue
                                    0,               // sysEx
                                    0,               // sysExLength
                                    MIDI::messageType_t::CONTROL_CHANGE });

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }
}

TEST_F(LEDsTest, SingleLEDstate)
{
    if (!LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        return;
    }

    static constexpr size_t MIDI_ID = 0;

    // By default, leds are configured to react on MIDI Note On.
    // Note 0 should turn the first LED on
    EXPECT_CALL(_leds._hwa, setState(MIDI_ID, LEDs::brightness_t::B100))
        .Times(1);

    MIDIDispatcher.notify(Messaging::eventType_t::MIDI_IN,
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
    MIDIDispatcher.notify(Messaging::eventType_t::MIDI_IN,
                          {
                              0,
                              MIDI_CHANNEL,
                              MIDI_ID,
                              0,
                              0,
                              0,
                              MIDI::messageType_t::NOTE_ON,
                          });

    if (LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS) < 3)
    {
        return;
    }

    // configure RGB LED 0
    ASSERT_TRUE(_leds._database.update(Database::Config::Section::leds_t::RGB_ENABLE, 0, 1));

    // now turn it on - expect three LEDs to be on

    EXPECT_CALL(_leds._hwa, setState(_leds._hwa.rgbComponentFromRGB(0, LEDs::rgbComponent_t::R), LEDs::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(_leds._hwa.rgbComponentFromRGB(0, LEDs::rgbComponent_t::G), LEDs::brightness_t::B100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(_leds._hwa.rgbComponentFromRGB(0, LEDs::rgbComponent_t::B), LEDs::brightness_t::B100))
        .Times(1);

    MIDIDispatcher.notify(Messaging::eventType_t::MIDI_IN,
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

#endif