#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "io/buttons/Buttons.h"
#include "io/leds/LEDs.h"
#include "io/common/CInfo.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"
#include "database/Database.h"
#include "stubs/database/DB_ReadWrite.h"

#ifdef LEDS_SUPPORTED

namespace
{
    bool buttonState[MAX_NUMBER_OF_BUTTONS] = {};

    class DBhandlers : public Database::Handlers
    {
        public:
        DBhandlers() {}

        void presetChange(uint8_t preset) override
        {
            if (presetChangeHandler != nullptr)
                presetChangeHandler(preset);
        }

        void factoryResetStart() override
        {
            if (factoryResetStartHandler != nullptr)
                factoryResetStartHandler();
        }

        void factoryResetDone() override
        {
            if (factoryResetDoneHandler != nullptr)
                factoryResetDoneHandler();
        }

        void initialized() override
        {
            if (initHandler != nullptr)
                initHandler();
        }

        //actions which these handlers should take depend on objects making
        //up the entire system to be initialized
        //therefore in interface we are calling these function pointers which
        // are set in application once we have all objects ready
        void (*presetChangeHandler)(uint8_t preset) = nullptr;
        void (*factoryResetStartHandler)()          = nullptr;
        void (*factoryResetDoneHandler)()           = nullptr;
        void (*initHandler)()                       = nullptr;
    } dbHandlers;

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
        HWALEDs() {}

        void setState(size_t index, IO::LEDs::brightness_t brightness) override
        {
            this->index.push_back(index);
            this->brightness.push_back(brightness);
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

        std::vector<size_t>                 index;
        std::vector<IO::LEDs::brightness_t> brightness;

    } hwaLEDs;

    class HWAButtons : public IO::Buttons::HWA
    {
        public:
        HWAButtons() {}

        bool state(size_t index) override
        {
            return buttonState[index];
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

    DBstorageMock dbStorageMock;
    Database      database = Database(dbHandlers, dbStorageMock, true);
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
    IO::Buttons buttons = IO::Buttons(hwaButtons, buttonsFilter, database, midi, leds, display, cInfo);
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

    auto reset = [&]() {
        leds.setAllOff();
        hwaLEDs.brightness.clear();
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
        hwaLEDs.brightness.clear();
    }

    reset();

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

        TEST_ASSERT_EQUAL_UINT32(0, hwaLEDs.brightness.size());
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
        hwaLEDs.brightness.clear();
    }

    reset();

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

        TEST_ASSERT_EQUAL_UINT32(0, hwaLEDs.brightness.size());
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
        hwaLEDs.brightness.clear();
    }

    reset();

    //test incorrect parameters
    for (size_t i = 0; i < 128; i++)
    {
        leds.midiToState(MIDI::messageType_t::noteOn, 0, i, 0, IO::LEDs::dataSource_t::external);
        TEST_ASSERT_EQUAL_UINT32(0, hwaLEDs.brightness.size());
    }

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
        hwaLEDs.brightness.clear();
    }

    reset();

    //in midi in mode nothing should be sent
    for (size_t i = 0; i < 128; i++)
    {
        leds.midiToState(MIDI::messageType_t::controlChange, 0, i, 0, IO::LEDs::dataSource_t::external);
        TEST_ASSERT_EQUAL_UINT32(0, hwaLEDs.brightness.size());
    }
}
#endif

#endif