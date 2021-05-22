#ifndef USB_LINK_MCU

#include "unity/Framework.h"
#include "io/buttons/Buttons.h"
#include "io/encoders/Encoders.h"
#include "io/analog/Analog.h"
#include "io/leds/LEDs.h"
#include "io/touchscreen/Touchscreen.h"
#include "io/common/CInfo.h"
#include "system/System.h"
#include "database/Database.h"
#include "midi/src/MIDI.h"
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

    class SystemHWA : public System::HWA
    {
        public:
        SystemHWA() = default;

        bool init() override
        {
            return true;
        }

        void reset()
        {
            dinMIDIenabled  = false;
            loopbackEnabled = false;
        }

        void reboot(FwSelector::fwType_t type) override
        {
        }

        void enableDINMIDI(bool loopback) override
        {
            dinMIDIenabled  = true;
            loopbackEnabled = loopback;
        }

        void disableDINMIDI() override
        {
            dinMIDIenabled  = false;
            loopbackEnabled = false;
        }

        void registerOnUSBconnectionHandler(System::usbConnectionHandler_t usbConnectionHandler) override
        {
        }

        bool dinMIDIenabled  = false;
        bool loopbackEnabled = false;
    } hwaSystem;

    class HWAMIDI : public MIDI::HWA
    {
        public:
        HWAMIDI() = default;

        bool init() override
        {
            clear();
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

        bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            if (!usbPacketFromBoard.size())
                return false;

            USBMIDIpacket = usbPacketFromBoard.at(0);
            usbPacketFromBoard.erase(usbPacketFromBoard.begin());

            return true;
        }

        bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            usbPacketToBoard.push_back(USBMIDIpacket);
            return true;
        }

        void clear()
        {
            usbPacketFromBoard.clear();
            usbPacketToBoard.clear();
            dinPacketToBoard.clear();
            dinPacketFromBoard.clear();
        }

        std::vector<MIDI::USBMIDIpacket_t> usbPacketFromBoard = {};
        std::vector<MIDI::USBMIDIpacket_t> usbPacketToBoard   = {};
        std::vector<uint8_t>               dinPacketToBoard   = {};
        std::vector<uint8_t>               dinPacketFromBoard = {};
    } hwaMIDI;

    class HWALEDs : public IO::LEDs::HWA
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
    } hwaLEDs;

    class HWAAnalog : public IO::Analog::HWA
    {
        public:
        HWAAnalog() {}

        bool value(size_t index, uint16_t& value) override
        {
            value = adcReturnValue;
            return true;
        }

        uint32_t adcReturnValue;
    } hwaAnalog;

    class HWAU8X8 : public IO::U8X8::HWAI2C
    {
        public:
        HWAU8X8() {}

        bool init() override
        {
            return true;
        }

        bool deInit() override
        {
            return true;
        }

        bool write(uint8_t address, uint8_t* data, size_t size) override
        {
            return true;
        }
    } hwaU8X8;

    class AnalogFilterStub : public IO::Analog::Filter
    {
        public:
        AnalogFilterStub() {}

        IO::Analog::adcType_t adcType() override
        {
#ifdef ADC_12_BIT
            return IO::Analog::adcType_t::adc12bit;
#else
            return IO::Analog::adcType_t::adc10bit;
#endif
        }

        bool isFiltered(size_t index, IO::Analog::type_t type, uint16_t value, uint16_t& filteredValue) override
        {
            filteredValue = value;
            return true;
        }

        void reset(size_t index) override
        {
        }
    } analogFilter;

    class HWAButtons : public IO::Buttons::HWA
    {
        public:
        HWAButtons() {}

        bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
        {
            return false;
        }
    } hwaButtons;

    class ButtonsFilter : public IO::Buttons::Filter
    {
        public:
        bool isFiltered(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
        {
            return true;
        }
    } buttonsFilter;

    class HWAEncoders : public IO::Encoders::HWA
    {
        public:
        HWAEncoders()
        {}

        bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
        {
            return false;
        }
    } hwaEncoders;

    class EncodersFilter : public IO::Encoders::Filter
    {
        public:
        bool isFiltered(size_t                    index,
                        IO::Encoders::position_t  position,
                        IO::Encoders::position_t& filteredPosition,
                        uint32_t                  sampleTakenTime) override
        {
            return true;
        }

        void reset(size_t index) override
        {
        }

        uint32_t lastMovementTime(size_t index) override
        {
            return 0;
        }
    } encodersFilter;

    DBstorageMock   dbStorageMock;
    Database        database(dbStorageMock, true);
    MIDI            midi(hwaMIDI);
    ComponentInfo   cInfo;
    IO::LEDs        leds(hwaLEDs, database);
    IO::U8X8        u8x8(hwaU8X8);
    IO::Display     display(u8x8, database);
    IO::Analog      analog(hwaAnalog, analogFilter, database, midi, leds, display, cInfo);
    IO::Buttons     buttons(hwaButtons, buttonsFilter, database, midi, leds, display, cInfo);
    IO::Encoders    encoders(hwaEncoders, encodersFilter, 1, database, midi, display, cInfo);
    IO::Touchscreen touchscreen(database, cInfo);
    System          systemStub(hwaSystem, cInfo, database, midi, buttons, encoders, analog, leds, display, touchscreen);

    void sendSysExRequest(const std::vector<uint8_t> request, std::vector<MIDI::USBMIDIpacket_t>& buffer)
    {
        class HWAFillMIDI : public MIDI::HWA
        {
            public:
            HWAFillMIDI(std::vector<MIDI::USBMIDIpacket_t>& buffer)
                : _buffer(buffer)
            {}

            bool init() override
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

        //create temp midi object which will fill internal hwaMIDI buffer of the real midi object when send is called
        //calling the real midi read will then result in parsing those filled bytes by the temp object
        MIDI fillMIDI(hwaFillMIDI);

        fillMIDI.enableUSBMIDI();
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

            bool init() override
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

        parseMIDI.enableUSBMIDI();
        parseMIDI.enableDINMIDI();
        parseMIDI.setInputChannel(MIDI_CHANNEL_OMNI);

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
                    chMessage = true;
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
    hwaSystem.reset();
    midi.init();
    midi.enableUSBMIDI();
    midi.disableDINMIDI();
}

TEST_CASE(SystemInit)
{
    //on init, factory reset is performed so everything is in its default state
    TEST_ASSERT(systemStub.init() == true);

    //verify that din midi is disabled
    TEST_ASSERT(hwaSystem.dinMIDIenabled == false);
    TEST_ASSERT(hwaSystem.loopbackEnabled == false);

    //now enable din midi via write in database
    TEST_ASSERT(database.update(Database::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled), 1) == true);

    //init system again and verify that din midi is enabled
    TEST_ASSERT(systemStub.init() == true);

#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT(hwaSystem.dinMIDIenabled == true);
    TEST_ASSERT(hwaSystem.loopbackEnabled == false);
#else
    //nothing should change
    TEST_ASSERT(hwaSystem.dinMIDIenabled == false);
    TEST_ASSERT(hwaSystem.loopbackEnabled == false);
#endif

    //now enable din to din merge, init system again and verify that both din midi and loopback are enabled
    TEST_ASSERT(database.update(Database::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::mergeEnabled), 1) == true);
    TEST_ASSERT(database.update(Database::Section::global_t::midiMerge, static_cast<size_t>(System::midiMerge_t::mergeType), static_cast<int32_t>(System::midiMergeType_t::DINtoDIN)) == true);

    TEST_ASSERT(systemStub.init() == true);

#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT(hwaSystem.dinMIDIenabled == true);
    TEST_ASSERT(hwaSystem.loopbackEnabled == true);
#else
    //nothing should change
    TEST_ASSERT(hwaSystem.dinMIDIenabled == false);
    TEST_ASSERT(hwaSystem.loopbackEnabled == false);
#endif
}

TEST_CASE(Requests)
{
    database.factoryReset();

    //send fake usb midi message containing the handshake
    //verify that the response is valid

    hwaMIDI.usbPacketToBoard.clear();

    //handshake
    sendSysExRequest({ 0xF0,
                       0x00,
                       0x53,
                       0x43,
                       0x00,
                       0x00,
                       0x01,
                       0xF7 },
                     hwaMIDI.usbPacketFromBoard);

    verifyResponse({ 0xF0,
                     0x00,
                     0x53,
                     0x43,
                     0x01,
                     0x00,
                     0x01,
                     0xF7 },
                   hwaMIDI.usbPacketToBoard);

    hwaMIDI.usbPacketFromBoard.clear();

    //verify that din midi is disabled
    TEST_ASSERT(hwaSystem.dinMIDIenabled == false);
    TEST_ASSERT(database.read(Database::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled)) == false);

    midi.sendNoteOn(127, 127, 1);
    TEST_ASSERT_EQUAL_UINT32(0, hwaMIDI.dinPacketFromBoard.size());

    std::vector<uint8_t> generatedSysExReq;
    MIDIHelper::generateSysExSetReq(System::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled), 1, generatedSysExReq);

    hwaMIDI.clear();

    sendSysExRequest(generatedSysExReq, hwaMIDI.usbPacketFromBoard);

#ifdef DIN_MIDI_SUPPORTED
    verifyResponse({ 0xF0,
                     0x00,
                     0x53,
                     0x43,
                     0x01,
                     0x00,
                     0x01,    //set
                     0x00,    //single
                     0x00,    //global block
                     0x00,    //midi features
                     0x00,
                     0x03,    //din midi
                     0x00,
                     0x01,    //enable
                     0xF7 },
                   hwaMIDI.usbPacketToBoard);

#else
    verifyResponse({ 0xF0,
                     0x00,
                     0x53,
                     0x43,
                     0x0D,    //not supported error
                     0x00,
                     0x01,    //set
                     0x00,    //single
                     0x00,    //global block
                     0x00,    //midi features
                     0x00,
                     0x03,    //din midi
                     0x00,
                     0x01,    //enable
                     0xF7 },
                   hwaMIDI.usbPacketToBoard);
#endif

#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT(hwaSystem.dinMIDIenabled == true);
    TEST_ASSERT(database.read(Database::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled)) == true);
#else
    TEST_ASSERT(hwaSystem.dinMIDIenabled == false);
    TEST_ASSERT(database.read(Database::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled)) == false);
#endif

    //test din by sending a midi message
    //verify that it is received via din midi
    midi.sendNoteOn(127, 127, 1);

#ifdef DIN_MIDI_SUPPORTED
    TEST_ASSERT_EQUAL_UINT32(3, hwaMIDI.dinPacketFromBoard.size());
#else
    TEST_ASSERT_EQUAL_UINT32(0, hwaMIDI.dinPacketFromBoard.size());
#endif

    hwaMIDI.usbPacketToBoard.clear();
    hwaMIDI.usbPacketFromBoard.clear();
    hwaMIDI.dinPacketFromBoard.clear();
    hwaMIDI.dinPacketToBoard.clear();

    //send handshake again and verify that the response isn't sent to din midi
    sendSysExRequest({ 0xF0,
                       0x00,
                       0x53,
                       0x43,
                       0x00,
                       0x00,
                       0x01,
                       0xF7 },
                     hwaMIDI.usbPacketFromBoard);

    verifyResponse({ 0xF0,
                     0x00,
                     0x53,
                     0x43,
                     0x01,
                     0x00,
                     0x01,
                     0xF7 },
                   hwaMIDI.usbPacketToBoard);

    TEST_ASSERT_EQUAL_UINT32(0, hwaMIDI.dinPacketFromBoard.size());
}

TEST_CASE(ForcedResendOnPresetChange)
{
    database.factoryReset();
    TEST_ASSERT(systemStub.init() == true);

    //enable first analog component in first two presets
    TEST_ASSERT(database.setPreset(1) == true);
    TEST_ASSERT(database.update(Database::Section::analog_t::enable, 0, 1) == true);

    TEST_ASSERT(database.setPreset(0) == true);
    TEST_ASSERT(database.update(Database::Section::analog_t::enable, 0, 1) == true);

    hwaMIDI.clear();

    hwaAnalog.adcReturnValue = 127;
    analog.update();

    TEST_ASSERT_EQUAL_UINT32(1, hwaMIDI.usbPacketToBoard.size());

    TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::controlChange, hwaMIDI.usbPacketToBoard.at(0).Event << 4);
    TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::controlChange, hwaMIDI.usbPacketToBoard.at(0).Data1);
    TEST_ASSERT_EQUAL_UINT32(0, hwaMIDI.usbPacketToBoard.at(0).Data2);
    TEST_ASSERT_EQUAL_UINT32(127, hwaMIDI.usbPacketToBoard.at(0).Data3);

    //now change preset and verify that the same midi message is repeated
    hwaMIDI.clear();

    //handshake
    sendSysExRequest({ 0xF0,
                       0x00,
                       0x53,
                       0x43,
                       0x00,
                       0x00,
                       0x01,
                       0xF7 },
                     hwaMIDI.usbPacketFromBoard);

    verifyResponse({ 0xF0,
                     0x00,
                     0x53,
                     0x43,
                     0x01,
                     0x00,
                     0x01,
                     0xF7 },
                   hwaMIDI.usbPacketToBoard);

    hwaMIDI.clear();
    channelMIDImessages.clear();

    std::vector<uint8_t> generatedSysExReq;
    MIDIHelper::generateSysExSetReq(System::Section::global_t::presets, static_cast<size_t>(System::presetSetting_t::activePreset), 1, generatedSysExReq);

    sendSysExRequest(generatedSysExReq, hwaMIDI.usbPacketFromBoard);

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
                   hwaMIDI.usbPacketToBoard);

    //since the preset has been changed, all buttons should resend their state and all enabled analog components (only 1 in this case)
    TEST_ASSERT(channelMIDImessages.size() == MAX_NUMBER_OF_BUTTONS + 1);
}
#endif