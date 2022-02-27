#include "unity/Framework.h"
#include "io/leds/LEDs.h"
#include "core/src/general/Timing.h"
#include "database/Database.h"
#include "stubs/database/DB_ReadWrite.h"
#include "stubs/Listener.h"

#ifdef LEDS_SUPPORTED

namespace
{
    class HWALEDs : public IO::LEDs::HWA
    {
        public:
        HWALEDs()
        {
            _brightness.resize(IO::LEDs::Collection::size(), IO::LEDs::brightness_t::bOff);
        }

        void setState(size_t index, IO::LEDs::brightness_t brightness) override
        {
            _brightness.at(index) = brightness;
        }

        size_t rgbSignalIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
        {
            return rgbIndex * 3 + static_cast<uint8_t>(rgbComponent);
        }

        size_t rgbIndex(size_t singleLEDindex) override
        {
            return singleLEDindex / 3;
        }

        std::vector<IO::LEDs::brightness_t> _brightness;

    } _hwaLEDs;

    Listener      _listener;
    DBstorageMock _dbStorageMock;
    Database      _database = Database(_dbStorageMock, true);
    IO::LEDs      _leds(_hwaLEDs, _database);
}    // namespace

TEST_SETUP()
{
    // init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(_database.init() == true);

    // always start from known state
    _database.factoryReset();
}

TEST_CASE(VerifyBrightnessAndBlinkSpeed)
{
    if (IO::LEDs::Collection::size(IO::LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        // these tables should match with the one at https://github.com/shanteacontrols/OpenDeck/wiki/LED-control
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

        // midiInNoteMultiVal
        //----------------------------------

        for (int i = 0; i < IO::LEDs::Collection::size(); i++)
            TEST_ASSERT(_database.update(Database::Section::leds_t::controlType, i, IO::LEDs::controlType_t::midiInNoteMultiVal) == true);

        // push messages to dispatcher
        // leds class listens to midi in message source
        // verify that after each push correct brightness and led blink speed are set
        for (uint16_t i = 0; i < 128; i++)
        {
            // MIDI ID 0, channel 0, value, MIDI in
            MIDIDispatcher.notify(Messaging::eventSource_t::midiIn, { 0, 0, 0, i, 0, 0, MIDI::messageType_t::noteOn }, Messaging::listenType_t::nonFwd);

            // read only the first response - it's possible midi state will be set for multiple LEDs
            TEST_ASSERT_EQUAL_UINT32(expectedBrightnessValue.at(i), _hwaLEDs._brightness.at(0));
            TEST_ASSERT_EQUAL_UINT32(expectedBlinkSpeedValue.at(i), _leds.blinkSpeed(0));
        }

        _leds.setAllOff();

        // midiInCCMultiVal
        //----------------------------------

        for (int i = 0; i < IO::LEDs::Collection::size(); i++)
            TEST_ASSERT(_database.update(Database::Section::leds_t::controlType, i, IO::LEDs::controlType_t::midiInCCMultiVal) == true);

        for (uint16_t i = 0; i < 128; i++)
        {
            // MIDI ID 0, channel 0, value, MIDI in
            MIDIDispatcher.notify(Messaging::eventSource_t::midiIn, { 0, 0, 0, i, 0, 0, MIDI::messageType_t::controlChange }, Messaging::listenType_t::nonFwd);

            // read only the first response - it's possible midi state will be set for multiple LEDs
            TEST_ASSERT_EQUAL_UINT32(expectedBrightnessValue.at(i), _hwaLEDs._brightness.at(0));
            TEST_ASSERT_EQUAL_UINT32(expectedBlinkSpeedValue.at(i), _leds.blinkSpeed(0));
        }

        _leds.setAllOff();

        // localNoteMultiVal
        //----------------------------------

        for (int i = 0; i < IO::LEDs::Collection::size(); i++)
            TEST_ASSERT(_database.update(Database::Section::leds_t::controlType, i, IO::LEDs::controlType_t::localNoteMultiVal) == true);

        for (uint16_t i = 0; i < 128; i++)
        {
            // MIDI ID 0, channel 0, value, MIDI in
            MIDIDispatcher.notify(Messaging::eventSource_t::buttons, { 0, 0, 0, i, 0, 0, MIDI::messageType_t::noteOn }, Messaging::listenType_t::nonFwd);

            // read only the first response - it's possible midi state will be set for multiple LEDs
            TEST_ASSERT_EQUAL_UINT32(expectedBrightnessValue.at(i), _hwaLEDs._brightness.at(0));
            TEST_ASSERT_EQUAL_UINT32(expectedBlinkSpeedValue.at(i), _leds.blinkSpeed(0));
        }

        _leds.setAllOff();

        // same test for analog components
        for (uint16_t i = 0; i < 128; i++)
        {
            // MIDI ID 0, channel 0, value, MIDI in
            MIDIDispatcher.notify(Messaging::eventSource_t::analog, { 0, 0, 0, i, 0, 0, MIDI::messageType_t::noteOn }, Messaging::listenType_t::nonFwd);

            // read only the first response - it's possible midi state will be set for multiple LEDs
            TEST_ASSERT_EQUAL_UINT32(expectedBrightnessValue.at(i), _hwaLEDs._brightness.at(0));
            TEST_ASSERT_EQUAL_UINT32(expectedBlinkSpeedValue.at(i), _leds.blinkSpeed(0));
        }

        _leds.setAllOff();

        // localCCMultiVal
        //----------------------------------

        for (int i = 0; i < IO::LEDs::Collection::size(); i++)
            TEST_ASSERT(_database.update(Database::Section::leds_t::controlType, i, IO::LEDs::controlType_t::localCCMultiVal) == true);

        // continously call _leds.midiToState with increasing MIDI value
        // verify that each call results in correct brightness and led blink speed
        for (uint16_t i = 0; i < 128; i++)
        {
            // MIDI ID 0, channel 0, value, MIDI in
            MIDIDispatcher.notify(Messaging::eventSource_t::buttons, { 0, 0, 0, i, 0, 0, MIDI::messageType_t::controlChange }, Messaging::listenType_t::nonFwd);

            // read only the first response - it's possible midi state will be set for multiple LEDs
            TEST_ASSERT_EQUAL_UINT32(expectedBrightnessValue.at(i), _hwaLEDs._brightness.at(0));
            TEST_ASSERT_EQUAL_UINT32(expectedBlinkSpeedValue.at(i), _leds.blinkSpeed(0));
        }

        _leds.setAllOff();

        // same test for analog components
        for (uint16_t i = 0; i < 128; i++)
        {
            // MIDI ID 0, channel 0, value, MIDI in
            MIDIDispatcher.notify(Messaging::eventSource_t::analog, { 0, 0, 0, i, 0, 0, MIDI::messageType_t::controlChange }, Messaging::listenType_t::nonFwd);

            // read only the first response - it's possible midi state will be set for multiple LEDs
            TEST_ASSERT_EQUAL_UINT32(expectedBrightnessValue.at(i), _hwaLEDs._brightness.at(0));
            TEST_ASSERT_EQUAL_UINT32(expectedBlinkSpeedValue.at(i), _leds.blinkSpeed(0));
        }

        _leds.setAllOff();
    }
}

TEST_CASE(SingleLEDstate)
{
    if (IO::LEDs::Collection::size(IO::LEDs::GROUP_DIGITAL_OUTPUTS))
    {
        // by default, leds are configured to react on MIDI Note on, channel 0
        // note 0 should turn the first LED on
        MIDIDispatcher.notify(Messaging::eventSource_t::midiIn, { 0, 0, 0, 127, 0, 0, MIDI::messageType_t::noteOn }, Messaging::listenType_t::nonFwd);
        TEST_ASSERT_EQUAL_UINT32(IO::LEDs::brightness_t::b100, _hwaLEDs._brightness.at(0));

        // now turn the LED off
        MIDIDispatcher.notify(Messaging::eventSource_t::midiIn, { 0, 0, 0, 0, 0, 0, MIDI::messageType_t::noteOn }, Messaging::listenType_t::nonFwd);
        TEST_ASSERT_EQUAL_UINT32(IO::LEDs::brightness_t::bOff, _hwaLEDs._brightness.at(0));

        if (IO::LEDs::Collection::size(IO::LEDs::GROUP_DIGITAL_OUTPUTS) >= 3)
        {
            // configure RGB LED 0
            TEST_ASSERT(_database.update(Database::Section::leds_t::rgbEnable, 0, 1) == true);

            // now turn it on
            MIDIDispatcher.notify(Messaging::eventSource_t::midiIn, { 0, 0, 0, 127, 0, 0, MIDI::messageType_t::noteOn }, Messaging::listenType_t::nonFwd);

            // three LEDs should be on now
            TEST_ASSERT_EQUAL_UINT32(IO::LEDs::brightness_t::b100, _hwaLEDs._brightness.at(_hwaLEDs.rgbSignalIndex(0, IO::LEDs::rgbIndex_t::r)));
            TEST_ASSERT_EQUAL_UINT32(IO::LEDs::brightness_t::b100, _hwaLEDs._brightness.at(_hwaLEDs.rgbSignalIndex(0, IO::LEDs::rgbIndex_t::g)));
            TEST_ASSERT_EQUAL_UINT32(IO::LEDs::brightness_t::b100, _hwaLEDs._brightness.at(_hwaLEDs.rgbSignalIndex(0, IO::LEDs::rgbIndex_t::b)));
        }
    }
}

#endif