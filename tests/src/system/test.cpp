#include <vector>
#include "unity/src/unity.h"
#include "unity/Helpers.h"
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
#include "stubs/MIDIHelper.h"

namespace
{
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

        bool isDigitalInputAvailable() override
        {
            return false;
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
            return true;
        }

        bool dinRead(uint8_t& data) override
        {
            return false;
        }

        bool dinWrite(uint8_t data) override
        {
            dinPacketOut.push_back(data);
            return true;
        }

        bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            if (!usbPacketIn.size())
                return false;

            USBMIDIpacket = usbPacketIn.at(0);
            usbPacketIn.erase(usbPacketIn.begin());

            return true;
        }

        bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            usbPacketOut.push_back(USBMIDIpacket);
            return true;
        }

        std::vector<MIDI::USBMIDIpacket_t> usbPacketIn  = {};
        std::vector<MIDI::USBMIDIpacket_t> usbPacketOut = {};
        std::vector<uint8_t>               dinPacketIn  = {};
        std::vector<uint8_t>               dinPacketOut = {};
    } hwaMIDI;

    class HWALEDs : public IO::LEDs::HWA
    {
        public:
        HWALEDs() {}

        void setState(size_t index, IO::LEDs::brightness_t brightness) override
        {
        }

        size_t rgbSingleComponentIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
        {
            return 0;
        }

        size_t rgbIndex(size_t singleLEDindex) override
        {
            return 0;
        }

        void setFadeSpeed(size_t transitionSpeed) override
        {
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

        bool state(size_t index) override
        {
            return false;
        }
    } hwaButtons;

    class ButtonsFilter : public IO::Buttons::Filter
    {
        public:
        bool isFiltered(size_t index, bool value, bool& filteredValue) override
        {
            return true;
        }

        void reset(size_t index) override
        {
        }
    } buttonsFilter;

    class HWAEncoders : public IO::Encoders::HWA
    {
        public:
        HWAEncoders()
        {}

        uint8_t state(size_t index) override
        {
            return 0;
        }
    } hwaEncoders;

    DBstorageMock   dbStorageMock;
    Database        database(dbStorageMock, true);
    MIDI            midi(hwaMIDI);
    ComponentInfo   cInfo;
    IO::LEDs        leds(hwaLEDs, database);
    IO::U8X8        u8x8(hwaU8X8);
    IO::Display     display(u8x8, database);
    IO::Analog      analog(hwaAnalog, analogFilter, database, midi, leds, display, cInfo);
    IO::Buttons     buttons(hwaButtons, buttonsFilter, database, midi, leds, display, cInfo);
    IO::Encoders    encoders(hwaEncoders, database, midi, display, cInfo);
    IO::Touchscreen touchscreen(database, cInfo);
    System          systemStub(hwaSystem, cInfo, database, midi, buttons, encoders, analog, leds, display, touchscreen);
}    // namespace

#ifndef USB_LINK_MCU
TEST_SETUP()
{
    hwaSystem.reset();
    midi.init();
    midi.enableUSBMIDI();
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

    TEST_ASSERT(systemStub.init() == true);

//init system again and verify that din midi is enabled
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

    std::vector<uint8_t> request;
    std::vector<uint8_t> response;

    auto sendRequest = [&]() {
        MIDIHelper::sysExToUSBMIDIPacket(request, hwaMIDI.usbPacketIn);

        size_t sz = hwaMIDI.usbPacketIn.size();

        for (size_t i = 0; i < sz; i++)
            systemStub.run();
    };

    auto verifyResponse = [&]() {
        std::vector<uint8_t> parsed;
        TEST_ASSERT(MIDIHelper::parseUSBSysEx(hwaMIDI.usbPacketOut, parsed) == true);
        TEST_ASSERT(parsed == response);
        hwaMIDI.usbPacketIn.clear();
        hwaMIDI.usbPacketOut.clear();
    };

    request = {
        0xF0,
        0x00,
        0x53,
        0x43,
        0x00,
        0x00,
        0x01,
        0xF7
    };

    response = {
        0xF0,
        0x00,
        0x53,
        0x43,
        0x01,
        0x00,
        0x01,
        0xF7
    };

    sendRequest();
    verifyResponse();

    //try to enable DIN midi via sysex
    //verify that it is disabled first
    TEST_ASSERT(hwaSystem.dinMIDIenabled == false);
    TEST_ASSERT(database.read(Database::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled)) == false);
    midi.sendNoteOn(127, 127, 1);
    TEST_ASSERT_EQUAL_UINT32(0, hwaMIDI.dinPacketOut.size());
    hwaMIDI.usbPacketOut.clear();

    request = {
        0xF0,
        0x00,
        0x53,
        0x43,
        0x00,
        0x00,
        0x01,    //set
        0x00,    //single
        0x00,    //global block
        0x00,    //midi features
        0x00,
        0x03,    //din midi
        0x00,
        0x01,    //enable
        0xF7
    };

#ifdef DIN_MIDI_SUPPORTED
    response = {
        0xF0,
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
        0xF7
    };
#else
    response = {
        0xF0,
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
        0xF7
    };
#endif

    sendRequest();
    verifyResponse();

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
    TEST_ASSERT_EQUAL_UINT32(3, hwaMIDI.dinPacketOut.size());
#else
    TEST_ASSERT_EQUAL_UINT32(0, hwaMIDI.dinPacketOut.size());
#endif

    hwaMIDI.usbPacketOut.clear();
    hwaMIDI.usbPacketIn.clear();
    hwaMIDI.dinPacketOut.clear();
    hwaMIDI.dinPacketIn.clear();

    //send handshake again and verify that the response isn't sent to din midi
    request = {
        0xF0,
        0x00,
        0x53,
        0x43,
        0x00,
        0x00,
        0x01,
        0xF7
    };

    response = {
        0xF0,
        0x00,
        0x53,
        0x43,
        0x01,
        0x00,
        0x01,
        0xF7
    };

    sendRequest();
    verifyResponse();

    TEST_ASSERT_EQUAL_UINT32(0, hwaMIDI.dinPacketOut.size());
}
#endif