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

    std::vector<channelMIDImessage_t> channelMIDImessages;

    DBstorageMock _dbStorageMock;
    Database      _database(_dbStorageMock, true);

    class HWALEDs : public System::HWA::IO::LEDs
    {
        public:
        HWALEDs() {}

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

        bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
        {
            return false;
        }
    } _hwaEncoders;

    class HWATouchscreen : public System::HWA::IO::Touchscreen
    {
        public:
        HWATouchscreen() = default;

        bool init()
        {
            return false;
        }

        bool deInit()
        {
            return false;
        }

        bool write(uint8_t value)
        {
            return false;
        }

        bool read(uint8_t& value)
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
            if (!dinPacketToBoard.size())
                return false;

            data = dinPacketToBoard.at(0);
            dinPacketToBoard.erase(dinPacketToBoard.begin());

            return true;
        }

        bool dinWrite(uint8_t data) override
        {
            dinPacketFromBoard.push_back(data);
            return true;
        }

        bool usbRead(::MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            if (!usbPacketFromBoard.size())
                return false;

            USBMIDIpacket = usbPacketFromBoard.at(0);
            usbPacketFromBoard.erase(usbPacketFromBoard.begin());

            return true;
        }

        bool usbWrite(::MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            usbPacketToBoard.push_back(USBMIDIpacket);
            return true;
        }

        void reset()
        {
            usbPacketFromBoard.clear();
            usbPacketToBoard.clear();
            dinPacketToBoard.clear();
            dinPacketFromBoard.clear();

            _loopbackEnabled = false;
        }

        std::vector<::MIDI::USBMIDIpacket_t> usbPacketFromBoard = {};
        std::vector<::MIDI::USBMIDIpacket_t> usbPacketToBoard   = {};
        std::vector<uint8_t>                 dinPacketToBoard   = {};
        std::vector<uint8_t>                 dinPacketFromBoard = {};
        bool                                 _loopbackEnabled   = false;
    } _hwaMIDI;

    class HWADMX : public System::HWA::Protocol::DMX
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
    } _hwaDMX;

    class HWASystem : public System::HWA
    {
        public:
        HWASystem() = default;

        bool init() override
        {
            return true;
        }

        void reboot(FwSelector::fwType_t type) override
        {
        }

        void registerOnUSBconnectionHandler(System::usbConnectionHandler_t usbConnectionHandler) override
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

    System systemStub(_hwaSystem, _database);

    void sendSysExRequest(const std::vector<uint8_t> request, std::vector<MIDI::USBMIDIpacket_t>& buffer)
    {
        class HWAFillMIDI : public MIDI::HWA
        {
            public:
            HWAFillMIDI(std::vector<MIDI::USBMIDIpacket_t>& buffer)
                : _buffer(buffer)
            {}

            bool init(MIDI::interface_t interface) override
            {
                return true;
            }

            bool deInit(MIDI::interface_t interface) override
            {
                return true;
            }

            bool dinRead(uint8_t& data) override
            {
                return false;
            }

            bool dinWrite(uint8_t data) override
            {
                return false;
            }

            bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
            {
                return false;
            }

            bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
            {
                _buffer.push_back(USBMIDIpacket);
                return true;
            }

            std::vector<MIDI::USBMIDIpacket_t>& _buffer;
        } hwaFillMIDI(buffer);

        //create temp midi object which will fill internal _hwaMIDI buffer of the real midi object when send is called
        //calling the real midi read will then result in parsing those filled bytes by the temp object
        MIDI fillMIDI(hwaFillMIDI);

        fillMIDI.init(MIDI::interface_t::usb);
        fillMIDI.sendSysEx(request.size(), &request[0], true);

        //store this in a variable since every midi.read call will decrement the size of buffer
        size_t packetSize = hwaFillMIDI._buffer.size();

        //now just call system which will call midi.read which in turn will read the filled packets
        for (size_t i = 0; i < packetSize; i++)
            systemStub.run();
    };

    void verifyResponse(const std::vector<uint8_t> response, std::vector<MIDI::USBMIDIpacket_t>& buffer)
    {
        class HWAParseMIDI : public MIDI::HWA
        {
            public:
            HWAParseMIDI(std::vector<MIDI::USBMIDIpacket_t>& buffer)
                : _buffer(buffer)
            {}

            bool init(MIDI::interface_t interface) override
            {
                return true;
            }

            bool deInit(MIDI::interface_t interface) override
            {
                return true;
            }

            bool dinRead(uint8_t& data) override
            {
                return false;
            }

            bool dinWrite(uint8_t data) override
            {
                _parsed.push_back(data);
                return true;
            }

            bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
            {
                if (!_buffer.size())
                    return false;

                USBMIDIpacket = _buffer.at(0);
                _buffer.erase(_buffer.begin());

                return true;
            }

            bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
            {
                return true;
            }

            std::vector<MIDI::USBMIDIpacket_t>& _buffer;
            std::vector<uint8_t>                _parsed;

        } hwaParseMIDI(buffer);

        MIDI parseMIDI(hwaParseMIDI);

        parseMIDI.init(MIDI::interface_t::all);
        parseMIDI.setInputChannel(MIDI::MIDI_CHANNEL_OMNI);

        //create temp midi object which will read written usb packets and pass them back as DIN MIDI array
        //for easier parsing

        size_t packetSize              = hwaParseMIDI._buffer.size();
        size_t sysExResponseIndexStart = 0;

        for (size_t i = 0; i < packetSize; i++)
        {
            if (parseMIDI.read(MIDI::interface_t::usb, MIDI::filterMode_t::fullDIN))
            {
                auto    messageType = parseMIDI.getType(MIDI::interface_t::usb);
                uint8_t data1       = parseMIDI.getData1(MIDI::interface_t::usb);
                uint8_t data2       = parseMIDI.getData2(MIDI::interface_t::usb);
                uint8_t channel     = parseMIDI.getChannel(MIDI::interface_t::usb);

                bool chMessage = false;

                switch (messageType)
                {
                case MIDI::messageType_t::noteOn:
                case MIDI::messageType_t::noteOff:
                case MIDI::messageType_t::controlChange:
                case MIDI::messageType_t::programChange:
                {
                    chMessage = true;
                }
                break;

                default:
                    break;
                }

                if (chMessage)
                {
                    channelMIDImessages.push_back({
                        channel,
                        messageType,
                        data1,
                        data2,
                    });
                }
            }
        }

        for (sysExResponseIndexStart = 0; sysExResponseIndexStart < hwaParseMIDI._parsed.size(); sysExResponseIndexStart++)
        {
            if (hwaParseMIDI._parsed.at(sysExResponseIndexStart) != response.at(0))
                continue;

            for (size_t i = 0; i < hwaParseMIDI._parsed.size() - sysExResponseIndexStart; i++)
                TEST_ASSERT_EQUAL_UINT32(response.at(i), hwaParseMIDI._parsed.at(i + sysExResponseIndexStart));

            break;
        }
    };
}    // namespace

TEST_SETUP()
{
    _hwaMIDI.reset();
}

TEST_CASE(SystemInit)
{
    //on init, factory reset is performed so everything is in its default state
    TEST_ASSERT(systemStub.init() == true);

    //enable din midi via write in database
#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT(_database.update(Database::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled), 1) == true);

    // init system again and verify that din midi is enabled
    TEST_ASSERT(systemStub.init() == true);

    TEST_ASSERT(_database.read(Database::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled)) == true);
#endif

    TEST_ASSERT(_hwaMIDI._loopbackEnabled == false);

    //now enable din to din merge, init system again and verify that both din midi and loopback are enabled
#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT(_database.update(Database::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::mergeEnabled), 1) == true);
    TEST_ASSERT(_database.update(Database::Section::global_t::midiMerge, static_cast<size_t>(System::midiMerge_t::mergeType), static_cast<int32_t>(System::midiMergeType_t::DINtoDIN)) == true);

    TEST_ASSERT(systemStub.init() == true);

    TEST_ASSERT(_database.read(Database::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled)) == true);
    TEST_ASSERT(_hwaMIDI._loopbackEnabled == true);
#endif
}

TEST_CASE(ForcedResendOnPresetChange)
{
    _database.factoryReset();
    TEST_ASSERT(systemStub.init() == true);

    //enable first analog component in first two presets
    TEST_ASSERT(_database.setPreset(1) == true);
    TEST_ASSERT(_database.update(Database::Section::analog_t::enable, 0, 1) == true);

    TEST_ASSERT(_database.setPreset(0) == true);
    TEST_ASSERT(_database.update(Database::Section::analog_t::enable, 0, 1) == true);

    _hwaMIDI.reset();

    //unrealistic value - also expect filter to scale this to maximum MIDI value
    _hwaAnalog.adcReturnValue = 0xFFFF;

    systemStub.run();    //buttons
    systemStub.run();    //encoders
    systemStub.run();    //analog

    TEST_ASSERT_EQUAL_UINT32(1, _hwaMIDI.usbPacketToBoard.size());

    TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::controlChange, _hwaMIDI.usbPacketToBoard.at(0).Event << 4);
    TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::controlChange, _hwaMIDI.usbPacketToBoard.at(0).Data1);
    TEST_ASSERT_EQUAL_UINT32(0, _hwaMIDI.usbPacketToBoard.at(0).Data2);
    TEST_ASSERT_EQUAL_UINT32(127, _hwaMIDI.usbPacketToBoard.at(0).Data3);

    //now change preset and verify that the same midi message is repeated
    _hwaMIDI.reset();

    //handshake
    sendSysExRequest({ 0xF0,
                       0x00,
                       0x53,
                       0x43,
                       0x00,
                       0x00,
                       0x01,
                       0xF7 },
                     _hwaMIDI.usbPacketFromBoard);

    verifyResponse({ 0xF0,
                     0x00,
                     0x53,
                     0x43,
                     0x01,
                     0x00,
                     0x01,
                     0xF7 },
                   _hwaMIDI.usbPacketToBoard);

    _hwaMIDI.reset();
    channelMIDImessages.clear();

    std::vector<uint8_t> generatedSysExReq;
    MIDIHelper::generateSysExSetReq(System::Section::global_t::presets, static_cast<size_t>(System::presetSetting_t::activePreset), 1, generatedSysExReq);

    sendSysExRequest(generatedSysExReq, _hwaMIDI.usbPacketFromBoard);

    verifyResponse({ 0xF0,
                     0x00,
                     0x53,
                     0x43,
                     0x01,
                     0x00,
                     0x01,    //set
                     0x00,    //single
                     0x00,    //block 0 (global)
                     0x02,    //section 2 (presets)
                     0x00,    //active preset
                     0x00,
                     0x00,    //preset 1
                     0x01,
                     0xF7 },
                   _hwaMIDI.usbPacketToBoard);

    //since the preset has been changed, all buttons should resend their state and all enabled analog components (only 1 in this case)
    TEST_ASSERT(channelMIDImessages.size() == MAX_NUMBER_OF_BUTTONS + 1);
}
#endif