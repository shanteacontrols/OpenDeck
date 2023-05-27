#ifndef PROJECT_TARGET_USB_OVER_SERIAL_HOST

#include "tests/Common.h"
#include "tests/stubs/System.h"
#include "tests/stubs/Listener.h"
#include "tests/helpers/MIDI.h"
#include "util/configurable/Configurable.h"

using namespace io;

namespace
{
    class SystemTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
        }

        void TearDown() override
        {
            ConfigHandler.clear();
            MIDIDispatcher.clear();
            _listener._event.clear();
        }

        int supportedPresets()
        {
            LOG(INFO) << "Checking the number of supported presets";

            auto response = _helper.sendRawSysExToStub(std::vector<uint8_t>({ 0xF0,
                                                                              0x00,
                                                                              0x53,
                                                                              0x43,
                                                                              0x00,
                                                                              0x00,
                                                                              SYSEX_CR_SUPPORTED_PRESETS,
                                                                              0xF7 }));

            return response.at(8);
        }

        void handshake()
        {
            LOG(INFO) << "Sending handshake";

            std::vector<uint8_t> expected = { 0xF0,
                                              0x00,
                                              0x53,
                                              0x43,
                                              0x01,
                                              0x00,
                                              0x01,
                                              0xF7 };

            auto response = _helper.sendRawSysExToStub(std::vector<uint8_t>({ 0xF0,
                                                                              0x00,
                                                                              0x53,
                                                                              0x43,
                                                                              0x00,
                                                                              0x00,
                                                                              0x01,
                                                                              0xF7 }));

            // verify status byte
            ASSERT_EQ(expected, response);
        }

        void fakeTimeAndRunSystem()
        {
            // preset change will be reported after PRESET_CHANGE_NOTIFY_DELAY ms
            // fake the passage of time here
            core::timing::detail::ms = core::timing::detail::ms + sys::Instance::PRESET_CHANGE_NOTIFY_DELAY;

            // clear out everything before running to parse with clean state
            _system._hwaMIDIUSB.clear();
            _system._hwaMIDIDIN.clear();
            _listener._event.clear();
            _system._instance.run();
        }

        Listener   _listener;
        TestSystem _system;
        MIDIHelper _helper = MIDIHelper(_system);
    };
}    // namespace

TEST_F(SystemTest, ForcedResendOnPresetChange)
{
    MIDIDispatcher.listen(messaging::eventType_t::TOUCHSCREEN_LED,
                          [this](const messaging::event_t& dispatchMessage)
                          {
                              _listener.messageListener(dispatchMessage);
                          });

    // on init, all LEDs are turned off by calling hwa interface - irrelevant here
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::OFF))
        .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

    // Loopback state will be set regardless of whether DIN is enabled or not.
    // Since it is not, it should be called with false argument.
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    ASSERT_TRUE(_system._instance.init());

    // for touchscreen, events are reported through dispatcher
    ASSERT_EQ(LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());

    for (size_t i = 0; i < LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::OFF), _listener._event.at(i).value);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    handshake();

    static constexpr size_t ANALOG_INDEX              = 0;
    static constexpr size_t ENABLED_ANALOG_COMPONENTS = io::Analog::Collection::SIZE(io::Analog::GROUP_ANALOG_INPUTS) ? 1 : 0;

    auto enableAnalog = [&]()
    {
        if (ENABLED_ANALOG_COMPONENTS)
        {
            ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::analog_t::ENABLE, ANALOG_INDEX, 1));
        }
    };

    enableAnalog();

    auto presets = supportedPresets();

    LOG(INFO) << "Supported presets: " << presets;

    if (presets < 2)
    {
        LOG(INFO) << "Not enough supported presets for further tests, exiting";
        return;
    }

    uint8_t newPreset = 1;

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::PRESETS,
                                                      database::Config::presetSetting_t::ACTIVE_PRESET,
                                                      newPreset));

    enableAnalog();

#ifdef PROJECT_TARGET_SUPPORT_DIN_MIDI
    // enable DIN midi as well - same data needs to be sent there

    EXPECT_CALL(_system._hwaMIDIDIN, init())
        .WillOnce(Return(true));

    // loopback shouldn't be changed

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::MIDI_SETTINGS,
                                                      protocol::MIDI::setting_t::DIN_ENABLED,
                                                      1));
#endif

    EXPECT_CALL(_system._hwaLEDs, setState(_, _))
        .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

    // only loopback setting will be called since DIN is already initialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    // values will be forcefully resent after a timeout
    // midi will also be re-initialized
    fakeTimeAndRunSystem();

    // for touchscreen, events are reported through dispatcher
    ASSERT_EQ(LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());

    for (size_t i = 0; i < LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::OFF), _listener._event.at(i).value);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    // The preset has been changed several times successively, but only one notification of that event is reported in PRESET_CHANGE_NOTIFY_DELAY ms.
    // All buttons and enabled analog components should resend their state.

    ASSERT_EQ((Buttons::Collection::SIZE(Buttons::GROUP_DIGITAL_INPUTS)) + ENABLED_ANALOG_COMPONENTS,
              _system._hwaMIDIUSB._writeParser.totalWrittenChannelMessages());

#ifdef PROJECT_TARGET_SUPPORT_DIN_MIDI
    ASSERT_EQ((Buttons::Collection::SIZE(Buttons::GROUP_DIGITAL_INPUTS)) + ENABLED_ANALOG_COMPONENTS,
              _system._hwaMIDIDIN._writeParser.totalWrittenChannelMessages());
#endif

    // Now switch preset again - on USB same amount of messages should be received.
    // Nothing should be received on DIN.

    newPreset = 0;

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::PRESETS,
                                                      database::Config::presetSetting_t::ACTIVE_PRESET,
                                                      newPreset));

    // LEDs will be refreshed - they are all off
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::OFF))
        .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

    // loopback will be set again - on preset change, MIDI is reinitialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

#ifdef PROJECT_TARGET_SUPPORT_DIN_MIDI
    // din midi isn't enabled in first preset so it will be disabled
    EXPECT_CALL(_system._hwaMIDIDIN, deInit())
        .WillOnce(Return(true));
#endif

    fakeTimeAndRunSystem();

    // for touchscreen, events are reported through dispatcher
    ASSERT_EQ(LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());

    for (size_t i = 0; i < LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::OFF), _listener._event.at(i).value);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    ASSERT_EQ((Buttons::Collection::SIZE(Buttons::GROUP_DIGITAL_INPUTS)) + ENABLED_ANALOG_COMPONENTS,
              _system._hwaMIDIUSB._writeParser.totalWrittenChannelMessages());

#ifdef PROJECT_TARGET_SUPPORT_DIN_MIDI
    ASSERT_EQ(0, _system._hwaMIDIDIN._writeParser.totalWrittenChannelMessages());
#endif

    // and finally, back to the preset in which din midi is enabled
    // this will verify that din is properly enabled again

    newPreset = 1;

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::PRESETS,
                                                      database::Config::presetSetting_t::ACTIVE_PRESET,
                                                      newPreset));

    // LEDs will be refreshed - they are all off
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::OFF))
        .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

    // loopback will be set again - on preset change, MIDI is reinitialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

#ifdef PROJECT_TARGET_SUPPORT_DIN_MIDI
    // din midi is enabled now
    EXPECT_CALL(_system._hwaMIDIDIN, init())
        .WillOnce(Return(true));
#endif

    fakeTimeAndRunSystem();

    // for touchscreen, events are reported through dispatcher
    ASSERT_EQ(LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());

    for (size_t i = 0; i < LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::OFF), _listener._event.at(i).value);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    ASSERT_EQ((Buttons::Collection::SIZE(Buttons::GROUP_DIGITAL_INPUTS)) + ENABLED_ANALOG_COMPONENTS,
              _system._hwaMIDIUSB._writeParser.totalWrittenChannelMessages());

#ifdef PROJECT_TARGET_SUPPORT_DIN_MIDI
    ASSERT_EQ((Buttons::Collection::SIZE(Buttons::GROUP_DIGITAL_INPUTS)) + ENABLED_ANALOG_COMPONENTS,
              _system._hwaMIDIDIN._writeParser.totalWrittenChannelMessages());
#endif
}

#ifdef LEDS_SUPPORTED
TEST_F(SystemTest, PresetChangeIndicatedOnLEDs)
{
    if (LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS) < 2)
    {
        return;
    }

    MIDIDispatcher.listen(messaging::eventType_t::TOUCHSCREEN_LED,
                          [this](const messaging::event_t& dispatchMessage)
                          {
                              _listener.messageListener(dispatchMessage);
                          });

    // on init, all LEDs are turned off by calling hwa interface - irrelevant here
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::OFF))
        .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

    // Loopback state will be set regardless of whether DIN is enabled or not.
    // Since it is not, it should be called with false argument.
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    ASSERT_TRUE(_system._instance.init());

    // for touchscreen, events are reported through dispatcher
    ASSERT_EQ(LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());

    for (size_t i = 0; i < LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::OFF), _listener._event.at(i).value);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    handshake();

    auto presets = supportedPresets();

    LOG(INFO) << "Supported presets: " << presets;

    if (presets < 2)
    {
        LOG(INFO) << "Not enough supported presets for further tests, exiting";
        return;
    }

    static constexpr size_t LED_INDEX = 0;

    // Configure the first LED to indicate current preset.
    // Its activation ID is 0 so it should be on only in first preset.

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::leds_t::CONTROL_TYPE,
                                                      LED_INDEX,
                                                      LEDs::controlType_t::PRESET));

    // also configure second led

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::leds_t::CONTROL_TYPE,
                                                      LED_INDEX + 1,
                                                      LEDs::controlType_t::PRESET));

    // switch preset
    uint8_t newPreset = 1;

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::PRESETS,
                                                      database::Config::presetSetting_t::ACTIVE_PRESET,
                                                      newPreset));

    // all leds should be off in new preset
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::OFF))
        .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

    // loopback will be set again - on preset change, MIDI is reinitialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    fakeTimeAndRunSystem();

    // verify the leds are off

    ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX));
    ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX + 1));

    // configure those same two leds to indicate preset in this preset as well

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::leds_t::CONTROL_TYPE,
                                                      LED_INDEX,
                                                      LEDs::controlType_t::PRESET));

    // also configure second led

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::leds_t::CONTROL_TYPE,
                                                      LED_INDEX + 1,
                                                      LEDs::controlType_t::PRESET));

    // now switch to preset 0 and expect only the first LED to be on
    newPreset = 0;

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::PRESETS,
                                                      database::Config::presetSetting_t::ACTIVE_PRESET,
                                                      newPreset));

    // the LEDs should still be off since the timeout hasn't passed

    ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX));
    ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX + 1));

#ifdef PROJECT_TARGET_SUPPORT_DIGITAL_OUTPUTS
    {
        InSequence s;

        // On preset change, all LEDs are first turned off.
        EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::OFF))
            .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

        // After that, checks are run again and only the LEDs which match
        // criteria will be turned on. In this case, the first LED should
        // be turned on since it is configured to be on in first preset.
        EXPECT_CALL(_system._hwaLEDs, setState(LED_INDEX, LEDs::brightness_t::B100))
            .Times(1);

        // Second LED will be turned off.
        EXPECT_CALL(_system._hwaLEDs, setState(LED_INDEX + 1, LEDs::brightness_t::OFF))
            .Times(1);
    }
#endif

    // loopback will be set again - on preset change, MIDI is reinitialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    fakeTimeAndRunSystem();

    // for touchscreen, events are reported through dispatcher
    if (LED_INDEX >= LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        // check if specified LEDs are in touchscreen group - if true, two more events should be reported
        ASSERT_EQ(LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS) + 2, _listener._event.size());
    }
    else
    {
        ASSERT_EQ(LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());
    }

    for (size_t i = 0; i < LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::OFF), _listener._event.at(i).value);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    if (LED_INDEX >= LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::B100), _listener._event.at(_listener._event.size() - 2).value);
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::OFF), _listener._event.at(_listener._event.size() - 1).value);
        ASSERT_EQ(LED_INDEX, _listener._event.at(_listener._event.size() - 2).componentIndex);
        ASSERT_EQ(LED_INDEX + 1, _listener._event.at(_listener._event.size() - 1).componentIndex);
    }

    // also verify through sysex
    ASSERT_EQ(1, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX));
    ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX + 1));

    // switch to preset 1 and verify that the first LED is off and second is on
    newPreset = 1;

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::PRESETS,
                                                      database::Config::presetSetting_t::ACTIVE_PRESET,
                                                      newPreset));

    // timeout hasn't occured yet
    ASSERT_EQ(1, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX));
    ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX + 1));

#ifdef PROJECT_TARGET_SUPPORT_DIGITAL_OUTPUTS
    {
        InSequence s;

        // On preset change, all LEDs are first turned off.
        EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::OFF))
            .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

        // After that, checks are run again and only the LEDs which match
        // criteria will be turned on. In this case, the first LED should
        // be turned off since it is configured to be on in first preset
        // and we are now in second preset
        EXPECT_CALL(_system._hwaLEDs, setState(LED_INDEX, LEDs::brightness_t::OFF))
            .Times(1);

        // Second LED will be turned on.
        EXPECT_CALL(_system._hwaLEDs, setState(LED_INDEX + 1, LEDs::brightness_t::B100))
            .Times(1);
    }
#endif

    // loopback will be set again - on preset change, MIDI is reinitialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    fakeTimeAndRunSystem();

    // for touchscreen, events are reported through dispatcher
    if (LED_INDEX >= LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        // check if specified LEDs are in touchscreen group - if true, two more events should be reported
        ASSERT_EQ(LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS) + 2, _listener._event.size());
    }
    else
    {
        ASSERT_EQ(LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());
    }

    for (size_t i = 0; i < LEDs::Collection::SIZE(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::OFF), _listener._event.at(i).value);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    if (LED_INDEX >= LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::OFF), _listener._event.at(_listener._event.size() - 2).value);
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::B100), _listener._event.at(_listener._event.size() - 1).value);
        ASSERT_EQ(LED_INDEX, _listener._event.at(_listener._event.size() - 2).componentIndex);
        ASSERT_EQ(LED_INDEX + 1, _listener._event.at(_listener._event.size() - 1).componentIndex);
    }

    // re-init the system - verify that the led 0 is on on startup after timeout to indicate current preset
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::OFF))
        .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

    // Loopback state will be set regardless of whether DIN is enabled or not.
    // Since it is not, it should be called with false argument.
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    ASSERT_TRUE(_system._instance.init());

    handshake();

    // initially the state should be off
    ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX));
    ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX + 1));

#ifdef PROJECT_TARGET_SUPPORT_DIGITAL_OUTPUTS
    {
        InSequence s;

        // first all off
        EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::OFF))
            .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

        // then what's needed
        EXPECT_CALL(_system._hwaLEDs, setState(LED_INDEX, LEDs::brightness_t::B100))
            .Times(1);

        EXPECT_CALL(_system._hwaLEDs, setState(LED_INDEX + 1, LEDs::brightness_t::OFF))
            .Times(1);
    }
#endif

    // loopback will be set again - on preset change, MIDI is reinitialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    fakeTimeAndRunSystem();

    // verify with sysex
    ASSERT_EQ(1, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX));
    ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX + 1));
}

TEST_F(SystemTest, ProgramIndicatedOnStartup)
{
    // on init, all LEDs are turned off by calling hwa interface - irrelevant here
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::OFF))
        .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

    // Loopback state will be set regardless of whether DIN is enabled or not.
    // Since it is not, it should be called with false argument.
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    ASSERT_TRUE(_system._instance.init());

    handshake();

    static constexpr size_t LED_INDEX = 0;

    // configure the first LED to indicate program change
    // its activation ID is 0 so it should be on only for program 0

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::leds_t::CONTROL_TYPE,
                                                      LED_INDEX,
                                                      LEDs::controlType_t::PC_SINGLE_VAL));

    // led should be off for now
    ASSERT_EQ(0, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX));

    // reinit the system again

#ifdef PROJECT_TARGET_SUPPORT_DIGITAL_OUTPUTS
    {
        InSequence s;

        EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::OFF))
            .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

        EXPECT_CALL(_system._hwaLEDs, setState(LED_INDEX, LEDs::brightness_t::B100))
            .Times(1);
    }
#endif

    // Loopback state will be set regardless of whether DIN is enabled or not.
    // Since it is not, it should be called with false argument.
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    ASSERT_TRUE(_system._instance.init());

    handshake();

    // also verify with sysex
    ASSERT_EQ(1, _helper.databaseReadFromSystemViaSysEx(sys::Config::Section::leds_t::TEST_COLOR, LED_INDEX));
}
#endif

#ifdef PROJECT_TARGET_SUPPORT_DIN_MIDI
TEST_F(SystemTest, UsbThruDin)
{
    // on init, all LEDs are turned off by calling hwa interface - irrelevant here
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::OFF))
        .Times(LEDs::Collection::SIZE(LEDs::GROUP_DIGITAL_OUTPUTS));

    // Loopback state will be set regardless of whether DIN is enabled or not.
    // Since it is not, it should be called with false argument.
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    ASSERT_TRUE(_system._instance.init());

    handshake();

    // enable both din midi and usb to din thru

    EXPECT_CALL(_system._hwaMIDIDIN, init())
        .WillOnce(Return(true));

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::MIDI_SETTINGS,
                                                      protocol::MIDI::setting_t::DIN_ENABLED,
                                                      1));

    ASSERT_TRUE(_helper.databaseWriteToSystemViaSysEx(sys::Config::Section::global_t::MIDI_SETTINGS,
                                                      protocol::MIDI::setting_t::USB_THRU_DIN,
                                                      1));

    // generate incoming USB message

    messaging::event_t event;

    event.channel = 1;
    event.index   = 0;
    event.value   = 127;
    event.message = MIDI::messageType_t::CONTROL_CHANGE;

    _helper.processIncoming(event);

    ASSERT_EQ(1, _system._hwaMIDIDIN._writeParser.totalWrittenChannelMessages());

    // again with note on - one of the LEDs should turn on as well
    event.message = MIDI::messageType_t::NOTE_ON;

#ifdef PROJECT_TARGET_SUPPORT_DIGITAL_OUTPUTS
    EXPECT_CALL(_system._hwaLEDs, setState(event.index, LEDs::brightness_t::B100))
        .Times(1);
#endif

    _helper.processIncoming(event);

    ASSERT_EQ(1, _system._hwaMIDIDIN._writeParser.totalWrittenChannelMessages());
}
#endif

#endif