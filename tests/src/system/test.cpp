#ifndef USB_LINK_MCU

#include "framework/Framework.h"
#include "stubs/System.h"
#include "stubs/Listener.h"
#include "helpers/MIDI.h"

using namespace IO;

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
            MIDIDispatcher.clear();
            _listener._event.clear();
        }

        void processIncoming(Messaging::event_t event)
        {
            _system._hwaMIDIUSB.clear();
            _system._hwaMIDIDIN.clear();

            _system._hwaMIDIUSB._readPackets = MIDIHelper::midiToUsbPackets(event);

            // don't care about hwa calls here
            EXPECT_CALL(_system._hwaButtons, state(_, _, _))
                .Times(AnyNumber());

            EXPECT_CALL(_system._hwaAnalog, value(_, _))
                .Times(AnyNumber());

            // now just call system which will call midi.read which in turn will read the filled packets
            while (_system._hwaMIDIUSB._readPackets.size())
            {
                _system._instance.run();
            }
        }

        void sendAndVerifySysExRequest(std::vector<uint8_t> request, const std::vector<uint8_t> expectedResponse)
        {
            Messaging::event_t event;
            event.sysEx       = &request[0];
            event.sysExLength = request.size();
            event.message     = MIDI::messageType_t::systemExclusive;

            processIncoming(event);

            bool responseVerified = false;

            // std::cout << "request / expected / received:" << std::endl;

            // for (size_t i = 0; i < request.size(); i++)
            //     std::cout << static_cast<int>(request.at(i)) << " ";
            // std::cout << std::endl;

            // for (size_t i = 0; i < expectedResponse.size(); i++)
            //     std::cout << static_cast<int>(expectedResponse.at(i)) << " ";
            // std::cout << std::endl;

            // for (size_t i = 0; i < rawBytes.size(); i++)
            //     std::cout << static_cast<int>(rawBytes.at(i)) << " ";
            // std::cout << std::endl;

            for (size_t i = 0; i < _system._hwaMIDIUSB._writeParser.writtenMessages().size(); i++)
            {
                // it's possible that the first response isn't sysex but some component message
                if (_system._hwaMIDIUSB._writeParser.writtenMessages().at(0).sysexArray[0] != expectedResponse.at(0))
                {
                    continue;
                }

                responseVerified = true;

                // once F0 is found, however, it should be expected response
                for (size_t byte = 0; byte < expectedResponse.size(); byte++)
                {
                    if (expectedResponse.at(byte) != _system._hwaMIDIUSB._writeParser.writtenMessages().at(i).sysexArray[byte])
                    {
                        responseVerified = false;
                        break;
                    }
                }

                if (responseVerified)
                {
                    break;
                }
            }

            _system._hwaMIDIUSB._writePackets.clear();
            ASSERT_TRUE(responseVerified);
        }

        uint16_t readFromDevice(std::vector<uint8_t> request, bool customReq = false)
        {
            Messaging::event_t event;
            event.sysEx       = &request[0];
            event.sysExLength = request.size();
            event.message     = MIDI::messageType_t::systemExclusive;

            processIncoming(event);

            auto response     = _system._hwaMIDIUSB._writeParser.writtenMessages().at(0).sysexArray;
            auto responseSize = _system._hwaMIDIUSB._writeParser.writtenMessages().at(0).length;

            if (customReq)
            {
                return response[8];
            }
            else
            {
                // last two bytes are result
                auto merged = Util::Conversion::Merge14bit(response[responseSize - 3], response[responseSize - 2]);
                return merged.value();
            }
        }

        void fakeTimeAndRunSystem()
        {
            // preset change will be reported after PRESET_CHANGE_NOTIFY_DELAY ms
            // fake the passage of time here
            core::timing::detail::rTime_ms += System::Instance::PRESET_CHANGE_NOTIFY_DELAY;

            // clear out everything before running to parse with clean state
            _system._hwaMIDIUSB.clear();
            _system._hwaMIDIDIN.clear();
            _listener._event.clear();
            _system._instance.run();
        }

        std::vector<uint8_t> _generatedSysExReq;
        Listener             _listener;
        TestSystem           _system;
    };

}    // namespace

TEST_F(SystemTest, ForcedResendOnPresetChange)
{
    MIDIDispatcher.listen(Messaging::eventSource_t::leds,
                          Messaging::listenType_t::fwd,
                          [this](const Messaging::event_t& dispatchMessage) {
                              _listener.messageListener(dispatchMessage);
                          });

    // on init, all LEDs are turned off by calling hwa interface - irrelevant here
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::bOff))
        .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

    // Loopback state will be set regardless of whether DIN is enabled or not.
    // Since it is not, it should be called with false argument.
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    ASSERT_TRUE(_system._instance.init());

    // for touchscreen, events are reported through dispatcher
    ASSERT_EQ(LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());

    for (size_t i = 0; i < LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::bOff), _listener._event.at(i).midiValue);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    // handshake
    sendAndVerifySysExRequest({ 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x00,
                                0x00,
                                0x01,
                                0xF7 },
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,
                                0xF7 });

    static constexpr size_t ANALOG_INDEX              = 0;
    static constexpr size_t ENABLED_ANALOG_COMPONENTS = IO::Analog::Collection::size(IO::Analog::GROUP_ANALOG_INPUTS) ? 1 : 0;

    auto enableAnalog = [&]() {
        if (ENABLED_ANALOG_COMPONENTS)
        {
            MIDIHelper::generateSysExSetReq(System::Config::Section::analog_t::enable,
                                            ANALOG_INDEX,
                                            1,
                                            _generatedSysExReq);

            sendAndVerifySysExRequest(_generatedSysExReq,
                                      { 0xF0,
                                        0x00,
                                        0x53,
                                        0x43,
                                        0x01,                                                               // ack
                                        0x00,                                                               // msg part
                                        0x01,                                                               // set
                                        0x00,                                                               // single
                                        static_cast<uint8_t>(System::Config::block_t::analog),              // block
                                        static_cast<uint8_t>(System::Config::Section::analog_t::enable),    // section
                                        0x00,                                                               // MSB index
                                        static_cast<uint8_t>(ANALOG_INDEX),                                 // LSB index
                                        0x00,                                                               // MSB new value
                                        1,                                                                  // LSB new value
                                        0xF7 });
        }
    };

    enableAnalog();

    auto supportedPresets = readFromDevice(
        { 0xF0,
          0x00,
          0x53,
          0x43,
          0x00,
          0x00,
          SYSEX_CR_SUPPORTED_PRESETS,
          0xF7 },
        true);

    LOG(INFO) << "Supported presets: " << static_cast<int>(supportedPresets);

    if (supportedPresets < 2)
    {
        LOG(INFO) << "Not enough supported presets for further tests, exiting";
        return;
    }

    uint8_t newPreset = 1;
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::presets,
                                    Database::Config::presetSetting_t::activePreset,
                                    newPreset,
                                    _generatedSysExReq);

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,                                                                     // ack
                                0x00,                                                                     // msg part
                                0x01,                                                                     // set
                                0x00,                                                                     // single
                                static_cast<uint8_t>(System::Config::block_t::global),                    // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::presets),         // preset section
                                0x00,                                                                     // MSB index (active preset)
                                static_cast<uint8_t>(Database::Config::presetSetting_t::activePreset),    // LSB index (active preset)
                                0x00,                                                                     // MSB new value
                                newPreset,                                                                // LSB new value
                                0xF7 });

    enableAnalog();

#ifdef DIN_MIDI_SUPPORTED
    // enable DIN midi as well - same data needs to be sent there
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::midiSettings,
                                    Protocol::MIDI::setting_t::dinEnabled,
                                    1,
                                    _generatedSysExReq);

    EXPECT_CALL(_system._hwaMIDIDIN, init())
        .WillOnce(Return(true));

    // loopback shouldn't be changed

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,                                                                     // set
                                0x00,                                                                     // single
                                static_cast<uint8_t>(System::Config::block_t::global),                    // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::midiSettings),    // midi section
                                0x00,                                                                     // MSB index (DIN enabled)
                                static_cast<uint8_t>(MIDI::setting_t::dinEnabled),                        // LSB index (DIN enabled)
                                0x00,                                                                     // MSB new value (DIN enabled)
                                0x01,                                                                     // LSB new value (DIN enabled)
                                0xF7 });
#endif

    EXPECT_CALL(_system._hwaLEDs, setState(_, _))
        .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

    // only loopback setting will be called since DIN is already initialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    // values will be forcefully resent after a timeout
    // midi will also be re-initialized
    fakeTimeAndRunSystem();

    // for touchscreen, events are reported through dispatcher
    ASSERT_EQ(LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());

    for (size_t i = 0; i < LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::bOff), _listener._event.at(i).midiValue);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    // The preset has been changed several times successively, but only one notification of that event is reported in PRESET_CHANGE_NOTIFY_DELAY ms.
    // All buttons and enabled analog components should resend their state.

    ASSERT_EQ((Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS)) + ENABLED_ANALOG_COMPONENTS,
              _system._hwaMIDIUSB._writeParser.totalWrittenChannelMessages());

#ifdef DIN_MIDI_SUPPORTED
    ASSERT_EQ((Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS)) + ENABLED_ANALOG_COMPONENTS,
              _system._hwaMIDIDIN._writeParser.totalWrittenChannelMessages());
#endif

    // Now switch preset again - on USB same amount of messages should be received.
    // Nothing should be received on DIN.

    newPreset = 0;
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::presets,
                                    Database::Config::presetSetting_t::activePreset,
                                    newPreset,
                                    _generatedSysExReq);

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,                                                                     // set
                                0x00,                                                                     // single
                                static_cast<uint8_t>(System::Config::block_t::global),                    // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::presets),         // preset section
                                0x00,                                                                     // MSB index (active preset)
                                static_cast<uint8_t>(Database::Config::presetSetting_t::activePreset),    // LSB index (active preset)
                                0x00,                                                                     // MSB new value
                                newPreset,                                                                // LSB new value
                                0xF7 });

    // LEDs will be refreshed - they are all off
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::bOff))
        .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

    // loopback will be set again - on preset change, MIDI is reinitialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

#ifdef DIN_MIDI_SUPPORTED
    // din midi isn't enabled in first preset so it will be disabled
    EXPECT_CALL(_system._hwaMIDIDIN, deInit())
        .WillOnce(Return(true));
#endif

    fakeTimeAndRunSystem();

    // for touchscreen, events are reported through dispatcher
    ASSERT_EQ(LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());

    for (size_t i = 0; i < LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::bOff), _listener._event.at(i).midiValue);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    ASSERT_EQ((Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS)) + ENABLED_ANALOG_COMPONENTS,
              _system._hwaMIDIUSB._writeParser.totalWrittenChannelMessages());

#ifdef DIN_MIDI_SUPPORTED
    ASSERT_EQ(0, _system._hwaMIDIDIN._writeParser.totalWrittenChannelMessages());
#endif

    // and finally, back to the preset in which din midi is enabled
    // this will verify that din is properly enabled again

    newPreset = 1;
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::presets,
                                    Database::Config::presetSetting_t::activePreset,
                                    newPreset,
                                    _generatedSysExReq);

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,                                                                     // ack
                                0x00,                                                                     // msg part
                                0x01,                                                                     // set
                                0x00,                                                                     // single
                                static_cast<uint8_t>(System::Config::block_t::global),                    // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::presets),         // preset section
                                0x00,                                                                     // MSB index (active preset)
                                static_cast<uint8_t>(Database::Config::presetSetting_t::activePreset),    // LSB index (active preset)
                                0x00,                                                                     // MSB new value
                                newPreset,                                                                // LSB new value
                                0xF7 });

    // LEDs will be refreshed - they are all off
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::bOff))
        .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

    // loopback will be set again - on preset change, MIDI is reinitialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

#ifdef DIN_MIDI_SUPPORTED
    // din midi is enabled now
    EXPECT_CALL(_system._hwaMIDIDIN, init())
        .WillOnce(Return(true));
#endif

    fakeTimeAndRunSystem();

    // for touchscreen, events are reported through dispatcher
    ASSERT_EQ(LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());

    for (size_t i = 0; i < LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::bOff), _listener._event.at(i).midiValue);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    ASSERT_EQ((Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS)) + ENABLED_ANALOG_COMPONENTS,
              _system._hwaMIDIUSB._writeParser.totalWrittenChannelMessages());

#ifdef DIN_MIDI_SUPPORTED
    ASSERT_EQ((Buttons::Collection::size(Buttons::GROUP_DIGITAL_INPUTS)) + ENABLED_ANALOG_COMPONENTS,
              _system._hwaMIDIDIN._writeParser.totalWrittenChannelMessages());
#endif
}

#ifdef LEDS_SUPPORTED
TEST_F(SystemTest, PresetChangeIndicatedOnLEDs)
{
    MIDIDispatcher.listen(Messaging::eventSource_t::leds,
                          Messaging::listenType_t::fwd,
                          [this](const Messaging::event_t& dispatchMessage) {
                              _listener.messageListener(dispatchMessage);
                          });

    // on init, all LEDs are turned off by calling hwa interface - irrelevant here
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::bOff))
        .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

    // Loopback state will be set regardless of whether DIN is enabled or not.
    // Since it is not, it should be called with false argument.
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    ASSERT_TRUE(_system._instance.init());

    // for touchscreen, events are reported through dispatcher
    ASSERT_EQ(LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());

    for (size_t i = 0; i < LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::bOff), _listener._event.at(i).midiValue);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    // handshake
    sendAndVerifySysExRequest({ 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x00,
                                0x00,
                                0x01,
                                0xF7 },
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,
                                0xF7 });

    auto supportedPresets = readFromDevice(
        { 0xF0,
          0x00,
          0x53,
          0x43,
          0x00,
          0x00,
          SYSEX_CR_SUPPORTED_PRESETS,
          0xF7 },
        true);

    LOG(INFO) << "Supported presets: " << static_cast<int>(supportedPresets);

    if (supportedPresets < 2)
    {
        LOG(INFO) << "Not enough supported presets for further tests, exiting";
        return;
    }

    static constexpr size_t LED_INDEX = 0;

    // Configure the first LED to indicate current preset.
    // Its activation ID is 0 so it should be on only in first preset.
    MIDIHelper::generateSysExSetReq(System::Config::Section::leds_t::controlType,
                                    LED_INDEX,
                                    LEDs::controlType_t::preset,
                                    _generatedSysExReq);

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,    // set
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::leds),
                                static_cast<uint8_t>(System::Config::Section::leds_t::controlType),
                                0x00,    // LED 0
                                LED_INDEX,
                                0x00,
                                static_cast<uint8_t>(LEDs::controlType_t::preset),
                                0xF7 });

    // switch preset
    uint8_t newPreset = 1;
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::presets,
                                    Database::Config::presetSetting_t::activePreset,
                                    newPreset,
                                    _generatedSysExReq);

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,                                                                     // ack
                                0x00,                                                                     // msg part
                                0x01,                                                                     // set
                                0x00,                                                                     // single
                                static_cast<uint8_t>(System::Config::block_t::global),                    // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::presets),         // preset section
                                0x00,                                                                     // MSB index (active preset)
                                static_cast<uint8_t>(Database::Config::presetSetting_t::activePreset),    // LSB index (active preset)
                                0x00,                                                                     // MSB new value
                                newPreset,                                                                // LSB new value
                                0xF7 });

    // all leds should be off in new preset
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::bOff))
        .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

    // loopback will be set again - on preset change, MIDI is reinitialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    fakeTimeAndRunSystem();

    // verify the led is off
    MIDIHelper::generateSysExGetReq(System::Config::Section::leds_t::testColor, LED_INDEX, _generatedSysExReq);

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x00,    // get
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::leds),
                                static_cast<uint8_t>(System::Config::Section::leds_t::testColor),
                                0x00,    // LED 0
                                LED_INDEX,
                                0x00,    // new value / blank
                                0x00,    // new value / blank
                                0x00,
                                0x00,    // LED state - off
                                0xF7 });

    // now switch to preset 0 and expect the LED 0 to be on
    newPreset = 0;
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::presets,
                                    Database::Config::presetSetting_t::activePreset,
                                    newPreset,
                                    _generatedSysExReq);

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,                                                                     // ack
                                0x00,                                                                     // msg part
                                0x01,                                                                     // set
                                0x00,                                                                     // single
                                static_cast<uint8_t>(System::Config::block_t::global),                    // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::presets),         // preset section
                                0x00,                                                                     // MSB index (active preset)
                                static_cast<uint8_t>(Database::Config::presetSetting_t::activePreset),    // LSB index (active preset)
                                0x00,                                                                     // MSB new value
                                newPreset,                                                                // LSB new value
                                0xF7 });

    MIDIHelper::generateSysExGetReq(System::Config::Section::leds_t::testColor, LED_INDEX, _generatedSysExReq);

    // verify first that the led is still off if timeout hasn't passed
    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x00,    // get
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::leds),
                                static_cast<uint8_t>(System::Config::Section::leds_t::testColor),
                                0x00,    // LED 0
                                LED_INDEX,
                                0x00,    // new value / blank
                                0x00,    // new value / blank
                                0x00,
                                0x00,    // LED state - off
                                0xF7 });

#ifdef DIGITAL_OUTPUTS_SUPPORTED
    {
        InSequence s;

        // On preset change, all LEDs are first turned off.
        EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::bOff))
            .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

        // After that, checks are run again and only the LEDs which match
        // criteria will be turned on. In this case, the first LED should
        // be turned on since it is configured to be on in first preset.
        EXPECT_CALL(_system._hwaLEDs, setState(LED_INDEX, LEDs::brightness_t::b100))
            .Times(1);
    }
#endif

    // loopback will be set again - on preset change, MIDI is reinitialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    fakeTimeAndRunSystem();

    // for touchscreen, events are reported through dispatcher
    if (LED_INDEX >= LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        // check if specified LED is in touchscreen group - if true, one more event should be reported
        ASSERT_EQ(LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS) + 1, _listener._event.size());
    }
    else
    {
        ASSERT_EQ(LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS), _listener._event.size());
    }

    for (size_t i = 0; i < LEDs::Collection::size(LEDs::GROUP_TOUCHSCREEN_COMPONENTS); i++)
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::bOff), _listener._event.at(i).midiValue);
        ASSERT_EQ(i, _listener._event.at(i).componentIndex);

        // rest of the values are irrelevant
    }

    if (LED_INDEX >= LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        ASSERT_EQ(static_cast<uint16_t>(LEDs::brightness_t::b100), _listener._event.at(_listener._event.size() - 1).midiValue);
        ASSERT_EQ(LED_INDEX, _listener._event.at(_listener._event.size() - 1).componentIndex);
    }

    // also verify through sysex
    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x00,    // get
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::leds),
                                static_cast<uint8_t>(System::Config::Section::leds_t::testColor),
                                0x00,    // LED 0
                                LED_INDEX,
                                0x00,    // new value / blank
                                0x00,    // new value / blank
                                0x00,
                                0x01,    // LED state - on
                                0xF7 });

    // switch back to preset 1 and verify that the led is turned off
    newPreset = 1;
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::presets,
                                    Database::Config::presetSetting_t::activePreset,
                                    newPreset,
                                    _generatedSysExReq);

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,                                                                     // ack
                                0x00,                                                                     // msg part
                                0x01,                                                                     // set
                                0x00,                                                                     // single
                                static_cast<uint8_t>(System::Config::block_t::global),                    // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::presets),         // preset section
                                0x00,                                                                     // MSB index (active preset)
                                static_cast<uint8_t>(Database::Config::presetSetting_t::activePreset),    // LSB index (active preset)
                                0x00,                                                                     // MSB new value
                                newPreset,                                                                // LSB new value
                                0xF7 });

    MIDIHelper::generateSysExGetReq(System::Config::Section::leds_t::testColor, 0, _generatedSysExReq);

    // should be still on - timeout hasn't occured
    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x00,    // get
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::leds),
                                static_cast<uint8_t>(System::Config::Section::leds_t::testColor),
                                0x00,    // LED 0
                                LED_INDEX,
                                0x00,    // new value / blank
                                0x00,    // new value / blank
                                0x00,
                                0x01,    // LED state - on
                                0xF7 });

    // all leds should be off in new preset
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::bOff))
        .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

    // loopback will be set again - on preset change, MIDI is reinitialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    fakeTimeAndRunSystem();

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x00,    // get
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::leds),
                                static_cast<uint8_t>(System::Config::Section::leds_t::testColor),
                                0x00,    // LED 0
                                LED_INDEX,
                                0x00,    // new value / blank
                                0x00,    // new value / blank
                                0x00,
                                0x00,    // LED state - off
                                0xF7 });

    // re-init the system - verify that the led 0 is on on startup after timeout to indicate current preset
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::bOff))
        .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

    // Loopback state will be set regardless of whether DIN is enabled or not.
    // Since it is not, it should be called with false argument.
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    ASSERT_TRUE(_system._instance.init());

    // handshake
    sendAndVerifySysExRequest({ 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x00,
                                0x00,
                                0x01,
                                0xF7 },
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,
                                0xF7 });

    MIDIHelper::generateSysExGetReq(System::Config::Section::leds_t::testColor, LED_INDEX, _generatedSysExReq);

    // initially the state should be off
    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x00,    // get
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::leds),
                                static_cast<uint8_t>(System::Config::Section::leds_t::testColor),
                                0x00,    // LED 0
                                0x00,
                                0x00,    // new value / blank
                                0x00,    // new value / blank
                                0x00,
                                0x00,    // LED state - off
                                0xF7 });

#ifdef DIGITAL_OUTPUTS_SUPPORTED
    {
        InSequence s;

        // first all off
        EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::bOff))
            .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

        // then on what's needed
        EXPECT_CALL(_system._hwaLEDs, setState(LED_INDEX, LEDs::brightness_t::b100))
            .Times(1);
    }
#endif

    // loopback will be set again - on preset change, MIDI is reinitialized
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    fakeTimeAndRunSystem();

    // verify with sysex
    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x00,    // get
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::leds),
                                static_cast<uint8_t>(System::Config::Section::leds_t::testColor),
                                0x00,    // LED 0
                                LED_INDEX,
                                0x00,    // new value / blank
                                0x00,    // new value / blank
                                0x00,
                                0x01,    // LED state - on
                                0xF7 });
}

TEST_F(SystemTest, ProgramIndicatedOnStartup)
{
    // on init, all LEDs are turned off by calling hwa interface - irrelevant here
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::bOff))
        .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

    // Loopback state will be set regardless of whether DIN is enabled or not.
    // Since it is not, it should be called with false argument.
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    ASSERT_TRUE(_system._instance.init());

    // handshake
    sendAndVerifySysExRequest({ 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x00,
                                0x00,
                                0x01,
                                0xF7 },
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,
                                0xF7 });

    static constexpr size_t LED_INDEX = 0;

    // configure the first LED to indicate program change
    // its activation ID is 0 so it should be on only for program 0
    MIDIHelper::generateSysExSetReq(System::Config::Section::leds_t::controlType,
                                    LED_INDEX,
                                    LEDs::controlType_t::pcSingleVal,
                                    _generatedSysExReq);

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,    // set
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::leds),
                                static_cast<uint8_t>(System::Config::Section::leds_t::controlType),
                                0x00,    // LED index
                                LED_INDEX,
                                0x00,
                                static_cast<uint8_t>(LEDs::controlType_t::pcSingleVal),
                                0xF7 });

    // led should be off for now
    MIDIHelper::generateSysExGetReq(System::Config::Section::leds_t::testColor, 0, _generatedSysExReq);

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x00,    // get
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::leds),
                                static_cast<uint8_t>(System::Config::Section::leds_t::testColor),
                                0x00,    // LED 0
                                LED_INDEX,
                                0x00,    // new value / blank
                                0x00,    // new value / blank
                                0x00,
                                0x00,    // LED state - off
                                0xF7 });

    // reinit the system again

#ifdef DIGITAL_OUTPUTS_SUPPORTED
    {
        InSequence s;

        EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::bOff))
            .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

        EXPECT_CALL(_system._hwaLEDs, setState(LED_INDEX, LEDs::brightness_t::b100))
            .Times(1);
    }
#endif

    // Loopback state will be set regardless of whether DIN is enabled or not.
    // Since it is not, it should be called with false argument.
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    ASSERT_TRUE(_system._instance.init());

    // handshake
    sendAndVerifySysExRequest({ 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x00,
                                0x00,
                                0x01,
                                0xF7 },
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,
                                0xF7 });

    // also verify with sysex
    MIDIHelper::generateSysExGetReq(System::Config::Section::leds_t::testColor,
                                    LED_INDEX,
                                    _generatedSysExReq);

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x00,    // get
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::leds),
                                static_cast<uint8_t>(System::Config::Section::leds_t::testColor),
                                0x00,    // LED 0
                                LED_INDEX,
                                0x00,    // new value / blank
                                0x00,    // new value / blank
                                0x00,
                                0x01,    // LED state - on
                                0xF7 });
}
#endif

#ifdef DIN_MIDI_SUPPORTED
TEST_F(SystemTest, UsbThruDin)
{
    // on init, all LEDs are turned off by calling hwa interface - irrelevant here
    EXPECT_CALL(_system._hwaLEDs, setState(_, LEDs::brightness_t::bOff))
        .Times(LEDs::Collection::size(LEDs::GROUP_DIGITAL_OUTPUTS));

    // Loopback state will be set regardless of whether DIN is enabled or not.
    // Since it is not, it should be called with false argument.
    EXPECT_CALL(_system._hwaMIDIDIN, setLoopback(false))
        .WillOnce(Return(true));

    ASSERT_TRUE(_system._instance.init());

    // handshake
    sendAndVerifySysExRequest({ 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x00,
                                0x00,
                                0x01,
                                0xF7 },
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,
                                0xF7 });

    // enable both din midi and usb to din thru
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::midiSettings,
                                    Protocol::MIDI::setting_t::dinEnabled,
                                    1,
                                    _generatedSysExReq);

    EXPECT_CALL(_system._hwaMIDIDIN, init())
        .WillOnce(Return(true));

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,    // set
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::global),
                                static_cast<uint8_t>(System::Config::Section::global_t::midiSettings),
                                0x00,                                                           // MSB index (dinEnabled)
                                static_cast<uint8_t>(Protocol::MIDI::setting_t::dinEnabled),    // LSB index (dinEnabled)
                                0x00,                                                           // MSB new value
                                1,                                                              // LSB new value
                                0xF7 });

    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::midiSettings,
                                    Protocol::MIDI::setting_t::usbThruDin,
                                    1,
                                    _generatedSysExReq);

    sendAndVerifySysExRequest(_generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,    // set
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::global),
                                static_cast<uint8_t>(System::Config::Section::global_t::midiSettings),
                                0x00,                                                           // MSB index (dinThruUsb)
                                static_cast<uint8_t>(Protocol::MIDI::setting_t::usbThruDin),    // LSB index (dinThruUsb)
                                0x00,                                                           // MSB new value
                                1,                                                              // LSB new value
                                0xF7 });

    // generate incoming USB message

    Messaging::event_t event;

    event.midiChannel = 1;
    event.midiIndex   = 0;
    event.midiValue   = 127;
    event.message     = MIDI::messageType_t::controlChange;

    processIncoming(event);

    ASSERT_EQ(1, _system._hwaMIDIDIN._writeParser.totalWrittenChannelMessages());

    // again with note on - one of the LEDs should turn on as well
    event.message = MIDI::messageType_t::noteOn;

#ifdef DIGITAL_OUTPUTS_SUPPORTED
    EXPECT_CALL(_system._hwaLEDs, setState(event.midiIndex, LEDs::brightness_t::b100))
        .Times(1);
#endif

    processIncoming(event);

    ASSERT_EQ(1, _system._hwaMIDIDIN._writeParser.totalWrittenChannelMessages());
}
#endif

#endif