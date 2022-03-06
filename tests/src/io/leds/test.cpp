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
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::bOff,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
            LEDs::brightness_t::b25,
            LEDs::brightness_t::b50,
            LEDs::brightness_t::b75,
            LEDs::brightness_t::b100,
        };

        std::vector<LEDs::blinkSpeed_t> expectedBlinkSpeedValue = {
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s1000ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s500ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::s250ms,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
            LEDs::blinkSpeed_t::noBlink,
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

    // midiInNoteMultiVal
    //----------------------------------

    for (int i = 0; i < LEDs::Collection::size(); i++)
    {
        ASSERT_TRUE(_leds._database.update(Database::Config::Section::leds_t::controlType, i, LEDs::controlType_t::midiInNoteMultiVal));
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

            MIDIDispatcher.notify(Messaging::eventSource_t::midiIn,
                                  { 0,               // componentIndex - irrelevant
                                    MIDI_CHANNEL,    // midiChannel
                                    led,             // midiIndex
                                    value,           // midiValue
                                    0,               // sysEx
                                    0,               // sysExLength
                                    MIDI::messageType_t::noteOn },
                                  Messaging::listenType_t::nonFwd);

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    // midiInCCMultiVal
    //----------------------------------

    for (int i = 0; i < LEDs::Collection::size(); i++)
    {
        ASSERT_TRUE(_leds._database.update(Database::Config::Section::leds_t::controlType, i, LEDs::controlType_t::midiInCCMultiVal));
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

            MIDIDispatcher.notify(Messaging::eventSource_t::midiIn,
                                  { 0,               // componentIndex - irrelevant
                                    MIDI_CHANNEL,    // midiChannel
                                    led,             // midiIndex
                                    value,           // midiValue
                                    0,               // sysEx
                                    0,               // sysExLength
                                    MIDI::messageType_t::controlChange },
                                  Messaging::listenType_t::nonFwd);

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    // localNoteMultiVal
    //----------------------------------

    for (int i = 0; i < LEDs::Collection::size(); i++)
    {
        ASSERT_TRUE(_leds._database.update(Database::Config::Section::leds_t::controlType, i, LEDs::controlType_t::localNoteMultiVal));
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

            MIDIDispatcher.notify(Messaging::eventSource_t::buttons,
                                  { 0,               // componentIndex - irrelevant
                                    MIDI_CHANNEL,    // midiChannel
                                    led,             // midiIndex
                                    value,           // midiValue
                                    0,               // sysEx
                                    0,               // sysExLength
                                    MIDI::messageType_t::noteOn },
                                  Messaging::listenType_t::nonFwd);

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

            MIDIDispatcher.notify(Messaging::eventSource_t::analog,
                                  { 0,               // componentIndex - irrelevant
                                    MIDI_CHANNEL,    // midiChannel
                                    led,             // midiIndex
                                    value,           // midiValue
                                    0,               // sysEx
                                    0,               // sysExLength
                                    MIDI::messageType_t::noteOn },
                                  Messaging::listenType_t::nonFwd);

            ASSERT_EQ(expectedBlinkSpeedValue.at(value), _leds._instance.blinkSpeed(led));
        }
    }

    // localCCMultiVal
    //----------------------------------

    for (int i = 0; i < LEDs::Collection::size(); i++)
    {
        ASSERT_TRUE(_leds._database.update(Database::Config::Section::leds_t::controlType, i, LEDs::controlType_t::localCCMultiVal));
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

            MIDIDispatcher.notify(Messaging::eventSource_t::buttons,
                                  { 0,               // componentIndex - irrelevant
                                    MIDI_CHANNEL,    // midiChannel
                                    led,             // midiIndex
                                    value,           // midiValue
                                    0,               // sysEx
                                    0,               // sysExLength
                                    MIDI::messageType_t::controlChange },
                                  Messaging::listenType_t::nonFwd);

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

            MIDIDispatcher.notify(Messaging::eventSource_t::analog,
                                  { 0,               // componentIndex - irrelevant
                                    MIDI_CHANNEL,    // midiChannel
                                    led,             // midiIndex
                                    value,           // midiValue
                                    0,               // sysEx
                                    0,               // sysExLength
                                    MIDI::messageType_t::controlChange },
                                  Messaging::listenType_t::nonFwd);

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
    EXPECT_CALL(_leds._hwa, setState(MIDI_ID, LEDs::brightness_t::b100))
        .Times(1);

    MIDIDispatcher.notify(Messaging::eventSource_t::midiIn,
                          {
                              0,
                              MIDI_CHANNEL,
                              MIDI_ID,
                              127,
                              0,
                              0,
                              MIDI::messageType_t::noteOn,
                          },
                          Messaging::listenType_t::nonFwd);

    EXPECT_CALL(_leds._hwa, setState(MIDI_ID, LEDs::brightness_t::bOff))
        .Times(1);

    // now turn the LED off
    MIDIDispatcher.notify(Messaging::eventSource_t::midiIn,
                          {
                              0,
                              MIDI_CHANNEL,
                              MIDI_ID,
                              0,
                              0,
                              0,
                              MIDI::messageType_t::noteOn,
                          },
                          Messaging::listenType_t::nonFwd);

    if (LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS) < 3)
    {
        return;
    }

    // configure RGB LED 0
    ASSERT_TRUE(_leds._database.update(Database::Config::Section::leds_t::rgbEnable, 0, 1));

    // now turn it on - expect three LEDs to be on

    EXPECT_CALL(_leds._hwa, setState(_leds._hwa.rgbSignalIndex(0, LEDs::rgbIndex_t::r), LEDs::brightness_t::b100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(_leds._hwa.rgbSignalIndex(0, LEDs::rgbIndex_t::g), LEDs::brightness_t::b100))
        .Times(1);

    EXPECT_CALL(_leds._hwa, setState(_leds._hwa.rgbSignalIndex(0, LEDs::rgbIndex_t::b), LEDs::brightness_t::b100))
        .Times(1);

    MIDIDispatcher.notify(Messaging::eventSource_t::midiIn,
                          {
                              0,
                              MIDI_CHANNEL,
                              MIDI_ID,
                              127,
                              0,
                              0,
                              MIDI::messageType_t::noteOn,
                          },
                          Messaging::listenType_t::nonFwd);
}

#endif