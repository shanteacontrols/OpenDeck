#include "unity/Framework.h"
#include "io/leds/LEDs.h"
#include "io/display/Display.h"
#include "io/common/CInfo.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"
#include "database/Database.h"
#include "stubs/database/DB_ReadWrite.h"

#ifdef LEDS_SUPPORTED

namespace
{
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
            return false;
        }

        bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            return false;
        }

        bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            midiPacket.push_back(USBMIDIpacket);
            return true;
        }

        std::vector<MIDI::USBMIDIpacket_t> midiPacket;
    } hwaMIDI;

    class HWALEDs : public IO::LEDs::HWA
    {
        public:
        HWALEDs()
        {
            brightness.resize(MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS, IO::LEDs::brightness_t::bOff);
        }

        void setState(size_t index, IO::LEDs::brightness_t brightness) override
        {
            this->brightness.at(index) = brightness;
        }

        size_t rgbSignalIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
        {
            return rgbIndex * 3 + static_cast<uint8_t>(rgbComponent);
        }

        size_t rgbIndex(size_t singleLEDindex) override
        {
            return singleLEDindex / 3;
        }

        std::vector<IO::LEDs::brightness_t> brightness;

    } hwaLEDs;

    DBstorageMock dbStorageMock;
    Database      database = Database(dbStorageMock, true);
    MIDI          midi(hwaMIDI);
    ComponentInfo cInfo;

    IO::LEDs leds(hwaLEDs, database);

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

    IO::U8X8    u8x8(hwaU8X8);
    IO::Display display(u8x8, database);
}    // namespace

TEST_SETUP()
{
    //init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);
    //always start from known state
    database.factoryReset();

    midi.init();
    midi.setChannelSendZeroStart(true);
    midi.enableUSBMIDI();
}

#if MAX_NUMBER_OF_LEDS > 0
TEST_CASE(VerifyBrightnessAndBlinkSpeed)
{
    //these tables should match with the one at https://github.com/paradajz/OpenDeck/wiki/LED-control
    std::vector<IO::LEDs::brightness_t> expectedBrightnessValue = {
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::bOff,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
        IO::LEDs::brightness_t::b25,
        IO::LEDs::brightness_t::b50,
        IO::LEDs::brightness_t::b75,
        IO::LEDs::brightness_t::b100,
    };

    std::vector<IO::LEDs::blinkSpeed_t> expectedBlinkSpeedValue = {
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s1000ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s500ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::s250ms,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
        IO::LEDs::blinkSpeed_t::noBlink,
    };

    //midiInNoteMultiVal
    //----------------------------------

    for (int i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
        TEST_ASSERT(database.update(Database::Section::leds_t::controlType, i, static_cast<int32_t>(IO::LEDs::controlType_t::midiInNoteMultiVal)) == true);

    //continously call leds.midiToState with increasing MIDI value
    //verify that each call results in correct brightness and led blink speed
    for (size_t i = 0; i < 128; i++)
    {
        //MIDI ID 0, channel 0, value, MIDI in
        leds.midiToState(MIDI::messageType_t::noteOn, 0, i, 0, IO::LEDs::dataSource_t::external);

        //read only the first response - it's possible midi state will be set for multiple LEDs
        TEST_ASSERT_EQUAL_UINT32(expectedBrightnessValue.at(i), hwaLEDs.brightness.at(0));
        TEST_ASSERT_EQUAL_UINT32(expectedBlinkSpeedValue.at(i), leds.blinkSpeed(0));
    }

    leds.setAllOff();

    //test incorrect parameters
    for (size_t i = 0; i < 128; i++)
    {
        leds.midiToState(MIDI::messageType_t::noteOn, 0, i, 0, IO::LEDs::dataSource_t::internal);
        leds.midiToState(MIDI::messageType_t::controlChange, 0, i, 0, IO::LEDs::dataSource_t::internal);
        leds.midiToState(MIDI::messageType_t::controlChange, 0, i, 0, IO::LEDs::dataSource_t::external);
        leds.midiToState(MIDI::messageType_t::programChange, 0, i, 0, IO::LEDs::dataSource_t::external);

        //test all other channels with otherwise correct params
        for (int channel = 1; channel < 16; channel++)
            leds.midiToState(MIDI::messageType_t::noteOn, 0, i, channel, IO::LEDs::dataSource_t::external);
    }

    //midiInCCMultiVal
    //----------------------------------

    for (int i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
        TEST_ASSERT(database.update(Database::Section::leds_t::controlType, i, static_cast<int32_t>(IO::LEDs::controlType_t::midiInCCMultiVal)) == true);

    //continously call leds.midiToState with increasing MIDI value
    //verify that each call results in correct brightness and led blink speed
    for (size_t i = 0; i < 128; i++)
    {
        //MIDI ID 0, channel 0, value, MIDI in
        leds.midiToState(MIDI::messageType_t::controlChange, 0, i, 0, IO::LEDs::dataSource_t::external);

        //read only the first response - it's possible midi state will be set for multiple LEDs
        TEST_ASSERT_EQUAL_UINT32(expectedBrightnessValue.at(i), hwaLEDs.brightness.at(0));
        TEST_ASSERT_EQUAL_UINT32(expectedBlinkSpeedValue.at(i), leds.blinkSpeed(0));
    }

    leds.setAllOff();

    //test incorrect parameters
    for (size_t i = 0; i < 128; i++)
    {
        leds.midiToState(MIDI::messageType_t::controlChange, 0, i, 0, IO::LEDs::dataSource_t::internal);
        leds.midiToState(MIDI::messageType_t::noteOn, 0, i, 0, IO::LEDs::dataSource_t::internal);
        leds.midiToState(MIDI::messageType_t::controlChange, 0, i, 0, IO::LEDs::dataSource_t::internal);
        leds.midiToState(MIDI::messageType_t::programChange, 0, i, 0, IO::LEDs::dataSource_t::external);

        //test all other channels with otherwise correct params
        for (int channel = 1; channel < 16; channel++)
            leds.midiToState(MIDI::messageType_t::noteOn, 0, i, channel, IO::LEDs::dataSource_t::external);
    }

    //localNoteMultiVal
    //----------------------------------

    for (int i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
        TEST_ASSERT(database.update(Database::Section::leds_t::controlType, i, static_cast<int32_t>(IO::LEDs::controlType_t::localNoteMultiVal)) == true);

    //continously call leds.midiToState with increasing MIDI value
    //verify that each call results in correct brightness and led blink speed
    for (size_t i = 0; i < 128; i++)
    {
        //MIDI ID 0, channel 0, value, MIDI in
        leds.midiToState(MIDI::messageType_t::noteOn, 0, i, 0, IO::LEDs::dataSource_t::internal);

        //read only the first response - it's possible midi state will be set for multiple LEDs
        TEST_ASSERT_EQUAL_UINT32(expectedBrightnessValue.at(i), hwaLEDs.brightness.at(0));
        TEST_ASSERT_EQUAL_UINT32(expectedBlinkSpeedValue.at(i), leds.blinkSpeed(0));
    }

    leds.setAllOff();

    //test incorrect parameters
    for (size_t i = 0; i < 128; i++)
        leds.midiToState(MIDI::messageType_t::noteOn, 0, i, 0, IO::LEDs::dataSource_t::external);

    //localCCMultiVal
    //----------------------------------

    for (int i = 0; i < MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
        TEST_ASSERT(database.update(Database::Section::leds_t::controlType, i, static_cast<int32_t>(IO::LEDs::controlType_t::localCCMultiVal)) == true);

    //continously call leds.midiToState with increasing MIDI value
    //verify that each call results in correct brightness and led blink speed
    for (size_t i = 0; i < 128; i++)
    {
        //MIDI ID 0, channel 0, value, MIDI in
        leds.midiToState(MIDI::messageType_t::controlChange, 0, i, 0, IO::LEDs::dataSource_t::internal);

        //read only the first response - it's possible midi state will be set for multiple LEDs
        TEST_ASSERT_EQUAL_UINT32(expectedBrightnessValue.at(i), hwaLEDs.brightness.at(0));
        TEST_ASSERT_EQUAL_UINT32(expectedBlinkSpeedValue.at(i), leds.blinkSpeed(0));
    }

    leds.setAllOff();

    //in midi in mode nothing should be sent
    for (size_t i = 0; i < 128; i++)
        leds.midiToState(MIDI::messageType_t::controlChange, 0, i, 0, IO::LEDs::dataSource_t::external);
}

TEST_CASE(SingleLEDstate)
{
    leds.midiToState(MIDI::messageType_t::noteOn, 0, 127, 0, IO::LEDs::dataSource_t::external);

    //by default, leds are configured to react on MIDI Note on, channel 0
    //note 0 should turn the first LED on
    TEST_ASSERT_EQUAL_UINT32(IO::LEDs::brightness_t::b100, hwaLEDs.brightness.at(0));

    //now turn the LED off
    leds.midiToState(MIDI::messageType_t::noteOn, 0, 0, 0, IO::LEDs::dataSource_t::external);
    TEST_ASSERT_EQUAL_UINT32(IO::LEDs::brightness_t::bOff, hwaLEDs.brightness.at(0));

#if MAX_NUMBER_OF_LEDS >= 3
    //configure RGB LED 0
    TEST_ASSERT(database.update(Database::Section::leds_t::rgbEnable, 0, 1) == true);

    //now turn it on
    leds.midiToState(MIDI::messageType_t::noteOn, 0, 127, 0, IO::LEDs::dataSource_t::external);

    //three LEDs should be on now
    TEST_ASSERT_EQUAL_UINT32(IO::LEDs::brightness_t::b100, hwaLEDs.brightness.at(hwaLEDs.rgbSignalIndex(0, IO::LEDs::rgbIndex_t::r)));
    TEST_ASSERT_EQUAL_UINT32(IO::LEDs::brightness_t::b100, hwaLEDs.brightness.at(hwaLEDs.rgbSignalIndex(0, IO::LEDs::rgbIndex_t::g)));
    TEST_ASSERT_EQUAL_UINT32(IO::LEDs::brightness_t::b100, hwaLEDs.brightness.at(hwaLEDs.rgbSignalIndex(0, IO::LEDs::rgbIndex_t::b)));
#endif
}
#endif

#endif