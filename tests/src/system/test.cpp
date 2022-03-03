#ifndef USB_LINK_MCU

#include "unity/Framework.h"
#include "system/System.h"
#include "system/Builder.h"
#include "database/Database.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include "stubs/database/DB_ReadWrite.h"
#include "helpers/MIDI.h"

namespace
{
    typedef struct
    {
        uint8_t             channel;    ///< MIDI channel on which the message was received (1-16)
        MIDI::messageType_t type;       ///< The type of the message
        uint8_t             data1;      ///< First data byte (0-127)
        uint8_t             data2;      ///< Second data byte (0-127, 0 if message length is 2 bytes)
    } channelMIDImessage_t;

    DBstorageMock _dbStorageMock;
    Database      _database(_dbStorageMock, true);

    class HWALEDs : public System::Builder::HWA::IO::LEDs
    {
        public:
        HWALEDs() = default;

        void setState(size_t index, IO::LEDs::brightness_t brightness) override
        {
        }

        size_t rgbSignalIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
        {
            return 0;
        }

        size_t rgbIndex(size_t singleLEDindex) override
        {
            return 0;
        }
    } _hwaLEDs;

    class HWAAnalog : public System::Builder::HWA::IO::Analog
    {
        public:
        HWAAnalog() {}

        bool value(size_t index, uint16_t& value) override
        {
            value = adcReturnValue;
            return true;
        }

        uint32_t adcReturnValue;
    } _hwaAnalog;

    class HWAButtons : public System::Builder::HWA::IO::Buttons
    {
        public:
        HWAButtons() = default;

        bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
        {
            return false;
        }

        size_t buttonToEncoderIndex(size_t index) override
        {
            return 0;
        }
    } _hwaButtons;

    class HWAEncoders : public System::Builder::HWA::IO::Encoders
    {
        public:
        HWAEncoders() = default;

        bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
        {
            return false;
        }
    } _hwaEncoders;

    class HWATouchscreen : public System::Builder::HWA::IO::Touchscreen
    {
        public:
        HWATouchscreen() = default;

        bool init() override
        {
            return false;
        }

        bool deInit() override
        {
            return false;
        }

        bool write(uint8_t value) override
        {
            return false;
        }

        bool read(uint8_t& value) override
        {
            return false;
        }

        bool allocated(IO::Common::interface_t interface) override
        {
            return false;
        }
    } _hwaTouchscreen;

    class HWATouchscreenCDCPassthrough : public System::Builder::HWA::IO::CDCPassthrough
    {
        public:
        HWATouchscreenCDCPassthrough() = default;

        bool init() override
        {
            return false;
        }

        bool deInit() override
        {
            return false;
        }

        bool uartRead(uint8_t& value) override
        {
            return false;
        }

        bool uartWrite(uint8_t value) override
        {
            return false;
        }

        bool cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize) override
        {
            return false;
        }

        bool cdcWrite(uint8_t* buffer, size_t size) override
        {
            return false;
        }

        bool allocated(IO::Common::interface_t interface) override
        {
            return false;
        }
    } _hwaCDCPassthrough;

    class HWAI2C : public System::Builder::HWA::IO::I2C
    {
        public:
        HWAI2C() = default;

        bool init() override
        {
            return true;
        }

        bool write(uint8_t address, uint8_t* buffer, size_t size) override
        {
            return true;
        }

        bool deviceAvailable(uint8_t address) override
        {
            return true;
        }
    } _hwaI2C;

    class HWAMIDIUSB : public System::Builder::HWA::Protocol::MIDI::USB
    {
        public:
        HWAMIDIUSB() = default;

        bool supported() override
        {
            return true;
        }

        bool init() override
        {
            reset();
            return true;
        }

        bool deInit() override
        {
            reset();
            return true;
        }

        bool read(MIDI::usbMIDIPacket_t& packet) override
        {
            if (!_readPackets.size())
                return false;

            packet = _readPackets.at(0);
            _readPackets.erase(_readPackets.begin());

            return true;
        }

        bool write(MIDI::usbMIDIPacket_t& packet) override
        {
            _writePackets.push_back(packet);
            return true;
        }

        void reset()
        {
            _readPackets.clear();
            _writePackets.clear();
        }

        size_t totalWrittenChannelMessages()
        {
            size_t cnt = 0;

            for (size_t i = 0; i < _writePackets.size(); i++)
            {
                auto messageType = MIDIlib::Base::typeFromStatusByte(_writePackets.at(i)[MIDI::USB_DATA1]);

                if (MIDIlib::Base::isChannelMessage(messageType))
                    cnt++;
            }

            return cnt;
        }

        std::vector<MIDI::usbMIDIPacket_t> _readPackets  = {};
        std::vector<MIDI::usbMIDIPacket_t> _writePackets = {};
    } _hwaMIDIUSB;

    class HWAMIDIDIN : public System::Builder::HWA::Protocol::MIDI::DIN
    {
        public:
        HWAMIDIDIN() = default;

        bool supported() override
        {
#ifdef DIN_MIDI_SUPPORTED
            return true;
#else
            return false;
#endif
        }

        bool init() override
        {
            reset();
            return true;
        }

        bool deInit() override
        {
            reset();
            return true;
        }

        bool setLoopback(bool state) override
        {
            _loopbackEnabled = state;
            return true;
        }

        bool read(uint8_t& data) override
        {
            if (!_readPackets.size())
                return false;

            data = _readPackets.at(0);
            _readPackets.erase(_readPackets.begin());

            return true;
        }

        bool write(uint8_t data) override
        {
            _writePackets.push_back(data);
            return true;
        }

        bool allocated(IO::Common::interface_t interface) override
        {
            return false;
        }

        void reset()
        {
            _readPackets.clear();
            _writePackets.clear();

            _loopbackEnabled = false;
        }

        size_t totalWrittenChannelMessages()
        {
            size_t cnt = 0;

            class HWASerialMIDI : public MIDIlib::SerialMIDI::HWA
            {
                public:
                HWASerialMIDI(std::vector<uint8_t>& packetsToParse)
                    : _packetsToParse(packetsToParse)
                {}

                bool init()
                {
                    return true;
                }

                bool deInit()
                {
                    return true;
                }

                bool write(uint8_t data)
                {
                    return true;
                }

                bool read(uint8_t& data)
                {
                    if (_packetsToParse.empty())
                    {
                        return false;
                    }

                    data = _packetsToParse.at(0);
                    _packetsToParse.erase(_packetsToParse.begin());

                    return true;
                }

                private:
                std::vector<uint8_t>& _packetsToParse;
            };

            // create new MIDI object which will read all written data
            HWASerialMIDI       _hwaSerialMIDI(_writePackets);
            MIDIlib::SerialMIDI _parseDinMIDI(_hwaSerialMIDI);
            _parseDinMIDI.init();

            while (_writePackets.size())
            {
                if (_parseDinMIDI.read())
                {
                    if (_parseDinMIDI.isChannelMessage(_parseDinMIDI.type()))
                    {
                        cnt++;
                    }
                }
            }

            return cnt;
        }

        std::vector<uint8_t> _readPackets     = {};
        std::vector<uint8_t> _writePackets    = {};
        bool                 _loopbackEnabled = false;
    } _hwaMIDIDIN;

    class HWADMX : public System::Builder::HWA::Protocol::DMX
    {
        public:
        HWADMX() = default;

        bool init() override
        {
            return false;
        }

        bool deInit() override
        {
            return false;
        }

        bool readUSB(uint8_t* buffer, size_t& size, const size_t maxSize) override
        {
            return false;
        }

        bool writeUSB(uint8_t* buffer, size_t size) override
        {
            return false;
        }

        bool updateChannel(uint16_t channel, uint8_t value) override
        {
            return false;
        }

        void packetComplete() override
        {
        }

        bool uniqueID(Protocol::DMX::uniqueID_t& uniqueID) override
        {
            return false;
        }

        bool allocated(IO::Common::interface_t interface) override
        {
            return false;
        }
    } _hwaDMX;

    class HWASystem : public System::Builder::HWA::System
    {
        public:
        HWASystem() = default;

        bool init() override
        {
            return true;
        }

        void update() override
        {
        }

        void reboot(FwSelector::fwType_t type) override
        {
        }

        void registerOnUSBconnectionHandler(System::Instance::usbConnectionHandler_t&& usbConnectionHandler) override
        {
        }
    } _hwaSystem;

    class HWABuilder : public ::System::Builder::HWA
    {
        public:
        HWABuilder() = default;

        ::System::Builder::HWA::IO& io() override
        {
            return _hwaIO;
        }

        ::System::Builder::HWA::Protocol& protocol() override
        {
            return _hwaProtocol;
        }

        ::System::Builder::HWA::System& system() override
        {
            return _hwaSystem;
        }

        private:
        class HWAIO : public ::System::Builder::HWA::IO
        {
            public:
            ::System::Builder::HWA::IO::LEDs& leds() override
            {
                return _hwaLEDs;
            }

            ::System::Builder::HWA::IO::Analog& analog() override
            {
                return _hwaAnalog;
            }

            ::System::Builder::HWA::IO::Buttons& buttons() override
            {
                return _hwaButtons;
            }

            ::System::Builder::HWA::IO::Encoders& encoders() override
            {
                return _hwaEncoders;
            }

            ::System::Builder::HWA::IO::Touchscreen& touchscreen() override
            {
                return _hwaTouchscreen;
            }

            ::System::Builder::HWA::IO::CDCPassthrough& cdcPassthrough() override
            {
                return _hwaCDCPassthrough;
            }

            ::System::Builder::HWA::IO::I2C& i2c() override
            {
                return _hwaI2C;
            }
        } _hwaIO;

        class HWAProtocol : public ::System::Builder::HWA::Protocol
        {
            public:
            ::System::Builder::HWA::Protocol::MIDI& midi()
            {
                return _hwaMIDI;
            }

            ::System::Builder::HWA::Protocol::DMX& dmx()
            {
                return _hwaDMX;
            }

            private:
            class HWAProtocolMIDI : public ::System::Builder::HWA::Protocol::MIDI
            {
                public:
                ::System::Builder::HWA::Protocol::MIDI::USB& usb() override
                {
                    return _hwaMIDIUSB;
                }

                ::System::Builder::HWA::Protocol::MIDI::DIN& din() override
                {
                    return _hwaMIDIDIN;
                }
            } _hwaMIDI;
        } _hwaProtocol;
    } _hwa;

    System::Builder  _builder(_hwa, _database);
    System::Instance systemStub(_builder.hwa(), _builder.components());

    void sendAndVerifySysExRequest(const std::vector<uint8_t> request, const std::vector<uint8_t> expectedResponse)
    {
        _hwaMIDIUSB._readPackets = MIDIHelper::rawSysExToUSBPackets(request);

        // now just call system which will call midi.read which in turn will read the filled packets
        while (_hwaMIDIUSB._readPackets.size())
        {
            systemStub.run();
        }

        // response is now in _hwaMIDIUSB._writePackets (board has sent this to host)
        // convert it to raw bytes for easier parsing
        auto rawBytes = MIDIHelper::usbSysExToRawBytes(_hwaMIDIUSB._writePackets);

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

        for (size_t i = 0; i < rawBytes.size(); i++)
        {
            // it's possible that the first response isn't sysex but some component message
            if (rawBytes.at(i) != expectedResponse.at(0))
                continue;

            responseVerified = true;

            // once F0 is found, however, it should be expected response
            for (size_t byte = 0; byte < expectedResponse.size(); byte++)
            {
                if (expectedResponse.at(byte) != rawBytes.at(byte + i))
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

        _hwaMIDIUSB._writePackets.clear();
        TEST_ASSERT(responseVerified == true);
    }
}    // namespace

TEST_SETUP()
{
    _hwaMIDIUSB.reset();
    _hwaMIDIDIN.reset();
}

TEST_CASE(SystemInit)
{
    // on init, factory reset is performed so everything is in its default state
    TEST_ASSERT(systemStub.init() == true);

    // enable din midi via write in database
#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT(_database.update(Database::Section::global_t::midiFeatures, Protocol::MIDI::feature_t::dinEnabled, 1) == true);

    // init system again and verify that din midi is enabled
    TEST_ASSERT(systemStub.init() == true);

    TEST_ASSERT(_database.read(Database::Section::global_t::midiFeatures, Protocol::MIDI::feature_t::dinEnabled) == true);
#endif

    TEST_ASSERT(_hwaMIDIDIN._loopbackEnabled == false);

    // now enable din to din thru, init system again and verify that both din midi and loopback are enabled
#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT(_database.update(Database::Section::global_t::midiFeatures, Protocol::MIDI::feature_t::dinThruDin, 1) == true);
    TEST_ASSERT(systemStub.init() == true);
    TEST_ASSERT(_database.read(Database::Section::global_t::midiFeatures, Protocol::MIDI::feature_t::dinEnabled) == true);
    TEST_ASSERT(_hwaMIDIDIN._loopbackEnabled == true);
#endif
}

TEST_CASE(ForcedResendOnPresetChange)
{
    if (_database.getSupportedPresets() <= 1)
        return;

    _database.factoryReset();
    TEST_ASSERT(systemStub.init() == true);

    const size_t ENABLED_ANALOG_COMPONENTS = IO::Analog::Collection::size(IO::Analog::GROUP_ANALOG_INPUTS) ? 1 : 0;

    if (ENABLED_ANALOG_COMPONENTS)
    {
        // enable first analog component in first two presets
        TEST_ASSERT(_database.setPreset(1) == true);
        TEST_ASSERT(_database.update(Database::Section::analog_t::enable, 0, 1) == true);

        TEST_ASSERT(_database.setPreset(0) == true);
        TEST_ASSERT(_database.update(Database::Section::analog_t::enable, 0, 1) == true);

        _hwaMIDIDIN.reset();
        _hwaMIDIUSB.reset();

        _hwaAnalog.adcReturnValue = 0xFFFF;

        systemStub.run();    // buttons
        systemStub.run();    // encoders
        systemStub.run();    // analog

        TEST_ASSERT_EQUAL_UINT32(1, _hwaMIDIUSB._writePackets.size());

        // verify the content of the message
        TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::controlChange, _hwaMIDIUSB._writePackets.at(0)[MIDI::USB_EVENT] << 4);
        TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::controlChange, _hwaMIDIUSB._writePackets.at(0)[MIDI::USB_DATA1]);
        TEST_ASSERT_EQUAL_UINT32(0, _hwaMIDIUSB._writePackets.at(0)[MIDI::USB_DATA2]);
        TEST_ASSERT(_hwaMIDIUSB._writePackets.at(0)[MIDI::USB_DATA3] > 0);    // due to filtering it is not certain what the value will be

        // now change preset and verify that the same midi message is repeated
        _hwaMIDIDIN.reset();
        _hwaMIDIUSB.reset();
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

    _hwaMIDIDIN.reset();
    _hwaMIDIUSB.reset();

    std::vector<uint8_t> generatedSysExReq;
    uint8_t              newPreset = 1;
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::presets, Database::presetSetting_t::activePreset, newPreset, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,                                                                // ack
                                0x00,                                                                // msg part
                                0x01,                                                                // set
                                0x00,                                                                // single
                                static_cast<uint8_t>(System::Config::block_t::global),               // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::presets),    // preset section
                                0x00,                                                                // MSB index (active preset)
                                static_cast<uint8_t>(Database::presetSetting_t::activePreset),       // LSB index (active preset)
                                0x00,                                                                // MSB new value
                                newPreset,                                                           // LSB new value
                                0xF7 });

#ifdef DIN_MIDI_SUPPORTED
    // enable DIN midi as well - same data needs to be sent there
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::midiFeatures, Protocol::MIDI::feature_t::dinEnabled, 1, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,                                                                     // set
                                0x00,                                                                     // single
                                static_cast<uint8_t>(System::Config::block_t::global),                    // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::midiFeatures),    // midi section
                                0x00,                                                                     // MSB index (DIN enabled)
                                static_cast<uint8_t>(MIDI::feature_t::dinEnabled),                        // LSB index (DIN enabled)
                                0x00,                                                                     // MSB new value (DIN enabled)
                                0x01,                                                                     // LSB new value (DIN enabled)
                                0xF7 });
#endif

    // values will be forcefully resent after a timeout
    // fake the passage of time here first
    core::timing::detail::rTime_ms += System::Instance::PRESET_CHANGE_NOTIFY_DELAY;
    _hwaMIDIUSB._writePackets.clear();
    systemStub.run();

    // the preset has been changed several times successively, but only one notification of that event is reported in in PRESET_CHANGE_NOTIFY_DELAYms
    // all buttons should resend their state
    // all enabled analog components should be sent as well if analog components are supported (only 1 in this case)

    TEST_ASSERT_EQUAL_UINT32((IO::Buttons::Collection::size(IO::Buttons::GROUP_DIGITAL_INPUTS) + ENABLED_ANALOG_COMPONENTS), _hwaMIDIUSB.totalWrittenChannelMessages());

#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT_EQUAL_UINT32((IO::Buttons::Collection::size(IO::Buttons::GROUP_DIGITAL_INPUTS) + ENABLED_ANALOG_COMPONENTS), _hwaMIDIDIN.totalWrittenChannelMessages());
#endif

    // now switch preset again - on usb same amount of messages should be received
    // nothing should be received on din

    newPreset = 0;
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::presets, Database::presetSetting_t::activePreset, newPreset, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,                                                                // set
                                0x00,                                                                // single
                                static_cast<uint8_t>(System::Config::block_t::global),               // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::presets),    // preset section
                                0x00,                                                                // MSB index (active preset)
                                static_cast<uint8_t>(Database::presetSetting_t::activePreset),       // LSB index (active preset)
                                0x00,                                                                // MSB new value
                                newPreset,                                                           // LSB new value
                                0xF7 });

    core::timing::detail::rTime_ms += System::Instance::PRESET_CHANGE_NOTIFY_DELAY;
    _hwaMIDIUSB._writePackets.clear();
    _hwaMIDIDIN._writePackets.clear();
    systemStub.run();

    TEST_ASSERT_EQUAL_UINT32((IO::Buttons::Collection::size(IO::Buttons::GROUP_DIGITAL_INPUTS) + ENABLED_ANALOG_COMPONENTS), _hwaMIDIUSB.totalWrittenChannelMessages());

#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT_EQUAL_UINT32(0, _hwaMIDIDIN.totalWrittenChannelMessages());
#endif

    // and finally, back to the preset in which din midi is enabled
    // this will verify that din is properly enabled again

    _hwaMIDIUSB._writePackets.clear();
    _hwaMIDIDIN._writePackets.clear();
    newPreset = 1;
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::presets, Database::presetSetting_t::activePreset, newPreset, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,                                                                // ack
                                0x00,                                                                // msg part
                                0x01,                                                                // set
                                0x00,                                                                // single
                                static_cast<uint8_t>(System::Config::block_t::global),               // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::presets),    // preset section
                                0x00,                                                                // MSB index (active preset)
                                static_cast<uint8_t>(Database::presetSetting_t::activePreset),       // LSB index (active preset)
                                0x00,                                                                // MSB new value
                                newPreset,                                                           // LSB new value
                                0xF7 });

    core::timing::detail::rTime_ms += System::Instance::PRESET_CHANGE_NOTIFY_DELAY;
    _hwaMIDIUSB._writePackets.clear();
    _hwaMIDIDIN._writePackets.clear();
    systemStub.run();

    TEST_ASSERT_EQUAL_UINT32((IO::Buttons::Collection::size(IO::Buttons::GROUP_DIGITAL_INPUTS) + ENABLED_ANALOG_COMPONENTS), _hwaMIDIUSB.totalWrittenChannelMessages());

#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT_EQUAL_UINT32((IO::Buttons::Collection::size(IO::Buttons::GROUP_DIGITAL_INPUTS) + ENABLED_ANALOG_COMPONENTS), _hwaMIDIDIN.totalWrittenChannelMessages());
#endif
}

#ifdef LEDS_SUPPORTED
TEST_CASE(PresetChangeIndicatedOnLEDs)
{
    if (_database.getSupportedPresets() <= 1)
        return;

    auto fakeTime = []() {
        // preset change will be reported after PRESET_CHANGE_NOTIFY_DELAY ms
        // fake the passage of time here
        core::timing::detail::rTime_ms += System::Instance::PRESET_CHANGE_NOTIFY_DELAY;
        systemStub.run();

        // after running the system also clear out everything that is possibly sent out to avoid
        // missdetection of sysex responses
        _hwaMIDIUSB._writePackets.clear();
    };

    _database.factoryReset();
    TEST_ASSERT(systemStub.init() == true);

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

    std::vector<uint8_t> generatedSysExReq;

    // configure the first LED to indicate current preset
    // its activation ID is 0 so it should be on only in first preset
    MIDIHelper::generateSysExSetReq(System::Config::Section::leds_t::controlType, 0, IO::LEDs::controlType_t::preset, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
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
                                0x00,
                                0x00,
                                static_cast<uint8_t>(IO::LEDs::controlType_t::preset),
                                0xF7 });

    // switch preset
    uint8_t newPreset = 1;
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::presets, Database::presetSetting_t::activePreset, newPreset, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,                                                                // ack
                                0x00,                                                                // msg part
                                0x01,                                                                // set
                                0x00,                                                                // single
                                static_cast<uint8_t>(System::Config::block_t::global),               // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::presets),    // preset section
                                0x00,                                                                // MSB index (active preset)
                                static_cast<uint8_t>(Database::presetSetting_t::activePreset),       // LSB index (active preset)
                                0x00,                                                                // MSB new value
                                newPreset,                                                           // LSB new value
                                0xF7 });

    // fake the passage of time after each preset change
    fakeTime();

    // verify the led is off
    MIDIHelper::generateSysExGetReq(System::Config::Section::leds_t::testColor, 0, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
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

    // now switch to preset 0 and expect the LED 0 to be on
    newPreset = 0;
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::presets, Database::presetSetting_t::activePreset, newPreset, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,                                                                // ack
                                0x00,                                                                // msg part
                                0x01,                                                                // set
                                0x00,                                                                // single
                                static_cast<uint8_t>(System::Config::block_t::global),               // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::presets),    // preset section
                                0x00,                                                                // MSB index (active preset)
                                static_cast<uint8_t>(Database::presetSetting_t::activePreset),       // LSB index (active preset)
                                0x00,                                                                // MSB new value
                                newPreset,                                                           // LSB new value
                                0xF7 });

    MIDIHelper::generateSysExGetReq(System::Config::Section::leds_t::testColor, 0, generatedSysExReq);

    // verify first that the led is still off if timeout hasn't passed
    sendAndVerifySysExRequest(generatedSysExReq,
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

    fakeTime();

    // should be on by now
    sendAndVerifySysExRequest(generatedSysExReq,
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
                                0x01,    // LED state - on
                                0xF7 });

    // switch back to preset 1 and verify that the led is turned off
    newPreset = 1;
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::presets, Database::presetSetting_t::activePreset, newPreset, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,                                                                // ack
                                0x00,                                                                // msg part
                                0x01,                                                                // set
                                0x00,                                                                // single
                                static_cast<uint8_t>(System::Config::block_t::global),               // global block
                                static_cast<uint8_t>(System::Config::Section::global_t::presets),    // preset section
                                0x00,                                                                // MSB index (active preset)
                                static_cast<uint8_t>(Database::presetSetting_t::activePreset),       // LSB index (active preset)
                                0x00,                                                                // MSB new value
                                newPreset,                                                           // LSB new value
                                0xF7 });

    MIDIHelper::generateSysExGetReq(System::Config::Section::leds_t::testColor, 0, generatedSysExReq);

    // should be still on - timeout hasn't occured
    sendAndVerifySysExRequest(generatedSysExReq,
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
                                0x01,    // LED state - on
                                0xF7 });

    fakeTime();

    sendAndVerifySysExRequest(generatedSysExReq,
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

    // re-init the system - verify that the led 0 is on on startup to indicate current preset
    TEST_ASSERT(systemStub.init() == true);

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

    MIDIHelper::generateSysExGetReq(System::Config::Section::leds_t::testColor, 0, generatedSysExReq);

    // initially the state should be off
    sendAndVerifySysExRequest(generatedSysExReq,
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

    fakeTime();

    sendAndVerifySysExRequest(generatedSysExReq,
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
                                0x01,    // LED state - on
                                0xF7 });
}

TEST_CASE(ProgramIndicatedOnStartup)
{
    _database.factoryReset();
    TEST_ASSERT(systemStub.init() == true);

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

    std::vector<uint8_t> generatedSysExReq;

    // configure the first LED to indicate program change
    // its activation ID is 0 so it should be on only for program 0
    MIDIHelper::generateSysExSetReq(System::Config::Section::leds_t::controlType, 0, IO::LEDs::controlType_t::pcSingleVal, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
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
                                0x00,
                                0x00,
                                static_cast<uint8_t>(IO::LEDs::controlType_t::pcSingleVal),
                                0xF7 });

    // led should be off for now
    MIDIHelper::generateSysExGetReq(System::Config::Section::leds_t::testColor, 0, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
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

    TEST_ASSERT(systemStub.init() == true);

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

    // verify that the led is turned on on startup since initially, program for all channels is 0
    MIDIHelper::generateSysExGetReq(System::Config::Section::leds_t::testColor, 0, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
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
                                0x01,    // LED state - on
                                0xF7 });
}
#endif

#ifdef DIN_MIDI_SUPPORTED
TEST_CASE(UsbThruDin)
{
    _database.factoryReset();
    TEST_ASSERT(systemStub.init() == true);

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

    std::vector<uint8_t> generatedSysExReq;

    // enable both din midi and usb to din thru
    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::midiFeatures, Protocol::MIDI::feature_t::dinEnabled, 1, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,    // set
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::global),
                                static_cast<uint8_t>(System::Config::Section::global_t::midiFeatures),
                                0x00,                                                           // MSB index (dinEnabled)
                                static_cast<uint8_t>(Protocol::MIDI::feature_t::dinEnabled),    // LSB index (dinEnabled)
                                0x00,                                                           // MSB new value
                                1,                                                              // LSB new value
                                0xF7 });

    MIDIHelper::generateSysExSetReq(System::Config::Section::global_t::midiFeatures, Protocol::MIDI::feature_t::usbThruDin, 1, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,    // set
                                0x00,    // single
                                static_cast<uint8_t>(System::Config::block_t::global),
                                static_cast<uint8_t>(System::Config::Section::global_t::midiFeatures),
                                0x00,                                                           // MSB index (dinThruUsb)
                                static_cast<uint8_t>(Protocol::MIDI::feature_t::usbThruDin),    // LSB index (dinThruUsb)
                                0x00,                                                           // MSB new value
                                1,                                                              // LSB new value
                                0xF7 });

    // generate incoming note on USB message
    _hwaMIDIUSB._readPackets = MIDIHelper::noteOnToUsbPacket(10, 127, 1);

    // now just call system which will call midi.read which in turn will read the filled packets
    while (_hwaMIDIUSB._readPackets.size())
    {
        systemStub.run();
    }

    TEST_ASSERT_EQUAL_UINT32(1, _hwaMIDIDIN.totalWrittenChannelMessages());
}
#endif

#endif