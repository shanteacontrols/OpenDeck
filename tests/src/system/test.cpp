#ifndef USB_LINK_MCU

#include "unity/Framework.h"
#include "system/System.h"
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

    class HWALEDs : public System::HWA::IO::LEDs
    {
        public:
        HWALEDs() {}

        bool supported() override
        {
#ifdef LEDS_SUPPORTED
            return true;
#else
            return false;
#endif
        }

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

    class HWAAnalog : public System::HWA::IO::Analog
    {
        public:
        HWAAnalog() {}

        bool supported() override
        {
#ifdef ANALOG_SUPPORTED
            return true;
#else
            return false;
#endif
        }

        bool value(size_t index, uint16_t& value) override
        {
            value = adcReturnValue;
            return true;
        }

        uint32_t adcReturnValue;
    } _hwaAnalog;

    class HWAButtons : public System::HWA::IO::Buttons
    {
        public:
        HWAButtons() {}

        bool supported() override
        {
#ifdef BUTTONS_SUPPORTED
            return true;
#else
            return false;
#endif
        }

        bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
        {
            return false;
        }

        size_t buttonToEncoderIndex(size_t index) override
        {
            return 0;
        }
    } _hwaButtons;

    class HWAEncoders : public System::HWA::IO::Encoders
    {
        public:
        HWAEncoders()
        {}

        bool supported() override
        {
#ifdef ENCODERS_SUPPORTED
            return true;
#else
            return false;
#endif
        }

        bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
        {
            return false;
        }
    } _hwaEncoders;

    class HWATouchscreen : public System::HWA::IO::Touchscreen
    {
        public:
        HWATouchscreen() = default;

        bool supported() override
        {
#ifdef TOUCHSCREEN_SUPPORTED
            return true;
#else
            return false;
#endif
        }

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
    } _hwaTouchscreen;

    class HWATouchscreenCDCPassthrough : public System::HWA::IO::CDCPassthrough
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

        private:
    } _hwaCDCPassthrough;

    class HWADisplay : public System::HWA::IO::Display
    {
        public:
        HWADisplay() {}

        bool supported() override
        {
#ifdef DISPLAY_SUPPORTED
            return true;
#else
            return false;
#endif
        }

        bool init() override
        {
            return true;
        }

        bool deInit() override
        {
            return true;
        }

        bool write(uint8_t address, uint8_t* buffer, size_t size) override
        {
            return true;
        }
    } _hwaDisplay;

    class HWAMIDI : public System::HWA::Protocol::MIDI, public MIDI::HWA
    {
        public:
        HWAMIDI() = default;

        bool dinSupported() override
        {
#ifdef DIN_MIDI_SUPPORTED
            return true;
#else
            return false;
#endif
        }

        bool init(::MIDI::interface_t interface) override
        {
            reset();
            return true;
        }

        bool deInit(::MIDI::interface_t interface) override
        {
            reset();
            return true;
        }

        bool setDINLoopback(bool state) override
        {
            _loopbackEnabled = state;
            return true;
        }

        bool dinRead(uint8_t& data) override
        {
            if (!dinReadPackets.size())
                return false;

            data = dinReadPackets.at(0);
            dinReadPackets.erase(dinReadPackets.begin());

            return true;
        }

        bool dinWrite(uint8_t data) override
        {
            dinWritePackets.push_back(data);
            return true;
        }

        bool usbRead(::MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            if (!usbReadPackets.size())
                return false;

            USBMIDIpacket = usbReadPackets.at(0);
            usbReadPackets.erase(usbReadPackets.begin());

            return true;
        }

        bool usbWrite(::MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            usbWritePackets.push_back(USBMIDIpacket);
            return true;
        }

        void reset()
        {
            usbReadPackets.clear();
            usbWritePackets.clear();
            dinReadPackets.clear();
            dinWritePackets.clear();

            _loopbackEnabled = false;
        }

        std::vector<::MIDI::USBMIDIpacket_t> usbReadPackets   = {};
        std::vector<::MIDI::USBMIDIpacket_t> usbWritePackets  = {};
        std::vector<uint8_t>                 dinReadPackets   = {};
        std::vector<uint8_t>                 dinWritePackets  = {};
        bool                                 _loopbackEnabled = false;
    } _hwaMIDI;

    class HWADMX : public System::HWA::Protocol::DMX
    {
        public:
        HWADMX() = default;

        bool supported() override
        {
#ifdef DMX_SUPPORTED
            return true;
#else
            return false;
#endif
        }

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
    } _hwaDMX;

    class HWASystem : public System::HWA
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

        void registerOnUSBconnectionHandler(System::usbConnectionHandler_t&& usbConnectionHandler) override
        {
        }

        bool serialPeripheralAllocated(System::serialPeripheral_t peripheral) override
        {
            return false;
        }

        bool uniqueID(System::uniqueID_t& uniqueID) override
        {
            return false;
        }

        System::HWA::IO& io() override
        {
            return _hwaIO;
        }

        System::HWA::Protocol& protocol() override
        {
            return _hwaProtocol;
        }

        private:
        class SystemHWAIO : public System::HWA::IO
        {
            public:
            SystemHWAIO() = default;

            System::HWA::IO::LEDs& leds() override
            {
                return _hwaLEDs;
            }

            System::HWA::IO::Analog& analog() override
            {
                return _hwaAnalog;
            }

            System::HWA::IO::Buttons& buttons() override
            {
                return _hwaButtons;
            }

            System::HWA::IO::Encoders& encoders() override
            {
                return _hwaEncoders;
            }

            System::HWA::IO::Touchscreen& touchscreen() override
            {
                return _hwaTouchscreen;
            }

            System::HWA::IO::CDCPassthrough& cdcPassthrough() override
            {
                return _hwaCDCPassthrough;
            }

            System::HWA::IO::Display& display() override
            {
                return _hwaDisplay;
            }
        } _hwaIO;

        class SystemHWAProtocol : public System::HWA::Protocol
        {
            public:
            System::HWA::Protocol::MIDI& midi()
            {
                return _hwaMIDI;
            }

            System::HWA::Protocol::DMX& dmx()
            {
                return _hwaDMX;
            }
        } _hwaProtocol;
    } _hwaSystem;
}    // namespace

TEST_SETUP()
{
    _hwaMIDI.reset();
}

TEST_CASE(SystemInit)
{
    System systemStub(_hwaSystem, _database);

    // on init, factory reset is performed so everything is in its default state
    TEST_ASSERT(systemStub.init() == true);

    // enable din midi via write in database
#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT(_database.update(Database::Section::global_t::midiFeatures, System::midiFeature_t::dinEnabled, 1) == true);

    // init system again and verify that din midi is enabled
    TEST_ASSERT(systemStub.init() == true);

    TEST_ASSERT(_database.read(Database::Section::global_t::midiFeatures, System::midiFeature_t::dinEnabled) == true);
#endif

    TEST_ASSERT(_hwaMIDI._loopbackEnabled == false);

    // now enable din to din merge, init system again and verify that both din midi and loopback are enabled
#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT(_database.update(Database::Section::global_t::midiFeatures, System::midiFeature_t::mergeEnabled, 1) == true);
    TEST_ASSERT(_database.update(Database::Section::global_t::midiMerge, System::midiMerge_t::mergeType, System::midiMergeType_t::DINtoDIN) == true);

    TEST_ASSERT(systemStub.init() == true);

    TEST_ASSERT(_database.read(Database::Section::global_t::midiFeatures, System::midiFeature_t::dinEnabled) == true);
    TEST_ASSERT(_hwaMIDI._loopbackEnabled == true);
#endif
}

TEST_CASE(ForcedResendOnPresetChange)
{
    System systemStub(_hwaSystem, _database);

    auto sendAndVerifySysExRequest = [&](const std::vector<uint8_t> request, const std::vector<uint8_t> response) {
        _hwaMIDI.usbReadPackets = MIDIHelper::rawSysExToUSBPackets(request);
        // store this in a variable since every midi.read call will decrement the size of buffer
        auto packetSize = _hwaMIDI.usbReadPackets.size();

        // now just call system which will call midi.read which in turn will read the filled packets
        for (size_t i = 0; i < packetSize; i++)
            systemStub.run();

        // response is now in _hwaMIDI.usbWritePackets (board has sent this to host)
        // convert it to raw bytes for easier parsing
        auto rawBytes = MIDIHelper::usbSysExToRawBytes(_hwaMIDI.usbWritePackets);

        for (size_t i = 0; i < rawBytes.size(); i++)
        {
            // it's possible that the first response isn't sysex but some component message
            if (rawBytes.at(i) != response.at(0))
                continue;

            // once F0 is found, however, it should be expected response
            for (size_t sysExByte = 0; sysExByte < rawBytes.size() - i; sysExByte++)
                TEST_ASSERT_EQUAL_UINT32(rawBytes.at(sysExByte), response.at(sysExByte));
        }
    };

    _database.factoryReset();
    TEST_ASSERT(systemStub.init() == true);

    // enable first analog component in first two presets
    TEST_ASSERT(_database.setPreset(1) == true);
    TEST_ASSERT(_database.update(Database::Section::analog_t::enable, 0, 1) == true);

    TEST_ASSERT(_database.setPreset(0) == true);
    TEST_ASSERT(_database.update(Database::Section::analog_t::enable, 0, 1) == true);

    _hwaMIDI.reset();

    // unrealistic value - also expect filter to scale this to maximum MIDI value
    _hwaAnalog.adcReturnValue = 0xFFFF;

    systemStub.run();    // buttons
    systemStub.run();    // encoders
    systemStub.run();    // analog

    TEST_ASSERT_EQUAL_UINT32(1, _hwaMIDI.usbWritePackets.size());

    // verify the content of the message
    TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::controlChange, _hwaMIDI.usbWritePackets.at(0).Event << 4);
    TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::controlChange, _hwaMIDI.usbWritePackets.at(0).Data1);
    TEST_ASSERT_EQUAL_UINT32(0, _hwaMIDI.usbWritePackets.at(0).Data2);
    TEST_ASSERT_EQUAL_UINT32(127, _hwaMIDI.usbWritePackets.at(0).Data3);

    // now change preset and verify that the same midi message is repeated
    _hwaMIDI.reset();

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

    _hwaMIDI.reset();

    std::vector<uint8_t> generatedSysExReq;
    MIDIHelper::generateSysExSetReq(System::Section::global_t::presets, static_cast<size_t>(System::presetSetting_t::activePreset), 1, generatedSysExReq);

    sendAndVerifySysExRequest(generatedSysExReq,
                              { 0xF0,
                                0x00,
                                0x53,
                                0x43,
                                0x01,
                                0x00,
                                0x01,    // set
                                0x00,    // single
                                0x00,    // block 0 (global)
                                0x02,    // section 2 (presets)
                                0x00,    // active preset
                                0x00,
                                0x00,    // preset 1
                                0x01,
                                0xF7 });

    // values will be forcefully resent after a timeout
    // fake the passage of time here first
    core::timing::detail::rTime_ms += System::FORCED_VALUE_RESEND_DELAY;
    _hwaMIDI.usbWritePackets.clear();
    systemStub.run();

    size_t channelMessages = 0;

    for (size_t i = 0; i < _hwaMIDI.usbWritePackets.size(); i++)
    {
        auto messageType = MIDI::getTypeFromStatusByte(_hwaMIDI.usbWritePackets.at(i).Data1);

        if (MIDI::isChannelMessage(messageType))
            channelMessages++;
    }

    // since the preset has been changed 3 times by now, all buttons should resend their state and all enabled analog components (only 1 in this case)
    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_BUTTONS + 1) * 3, channelMessages);
}
#endif