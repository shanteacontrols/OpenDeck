#ifdef ANALOG_SUPPORTED

#include "unity/Framework.h"
#include "io/analog/Analog.h"
#include "database/Database.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include "stubs/database/DB_ReadWrite.h"
#include "stubs/AnalogFilter.h"
#include "stubs/Listener.h"
#include "util/messaging/Messaging.h"

namespace
{
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
    } _hwaAnalog;

    Util::MessageDispatcher _dispatcher;
    Listener                _listener;
    DBstorageMock           _dbStorageMock;
    Database                _database = Database(_dbStorageMock, true);
    AnalogFilterStub        _analogFilter;
    IO::Analog              _analog(_hwaAnalog, _analogFilter, _database, _dispatcher);
}    // namespace

TEST_SETUP()
{
    //init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(_database.init() == true);
    TEST_ASSERT(_database.factoryReset() == true);

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        _analog.debounceReset(i);

    static bool listenerActive = false;

    if (!listenerActive)
    {
        _dispatcher.listen(Util::MessageDispatcher::messageSource_t::analog,
                           Util::MessageDispatcher::listenType_t::nonFwd,
                           [](const Util::MessageDispatcher::message_t& dispatchMessage) {
                               _listener.messageListener(dispatchMessage);
                           });

        listenerActive = true;
    }

    _listener._dispatchMessage.clear();
}

TEST_CASE(CCtest)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(_database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(_database.update(Database::Section::analog_t::type, i, Analog::type_t::potentiometerControlChange) == true);

        //set all lower limits to 0
        TEST_ASSERT(_database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to MIDI_7_BIT_VALUE_MAX
        TEST_ASSERT(_database.update(Database::Section::analog_t::upperLimit, i, MIDI::MIDI_7_BIT_VALUE_MAX) == true);

        //midi channel
        TEST_ASSERT(_database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    //feed all the values from minimum to maximum
    //expect the following:
    //first value is 0
    //last value is 127

    for (int i = 0; i <= 127; i++)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 128), _listener._dispatchMessage.size());

    //all received messages should be control change
    for (int i = 0; i < _listener._dispatchMessage.size(); i++)
        TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::controlChange, _listener._dispatchMessage.at(i).message);

    uint8_t expectedMIDIvalue = 0;

    for (int i = 0; i < _listener._dispatchMessage.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, _listener._dispatchMessage.at(index).midiValue);
        }

        expectedMIDIvalue++;
    }

    //now go backward

    _listener._dispatchMessage.clear();

    for (int i = 127; i >= 0; i--)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    //expect one message less since the last one was 127
    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 127), _listener._dispatchMessage.size());

    expectedMIDIvalue = 126;

    for (int i = 0; i < _listener._dispatchMessage.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, _listener._dispatchMessage.at(index).midiValue);
        }

        expectedMIDIvalue--;
    }

    _listener._dispatchMessage.clear();

    //try to feed value larger than 127
    //no response should be received
    _hwaAnalog.adcReturnValue = 10000;
    _analog.update();

    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());
}

TEST_CASE(PitchBendTest)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(_database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with Pitch Bend MIDI message
        TEST_ASSERT(_database.update(Database::Section::analog_t::type, i, Analog::type_t::pitchBend) == true);

        //set all lower limits to 0
        TEST_ASSERT(_database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        TEST_ASSERT(_database.update(Database::Section::analog_t::upperLimit, i, MIDI::MIDI_14_BIT_VALUE_MAX) == true);

        //midi channel
        TEST_ASSERT(_database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    for (int i = 0; i <= 16383; i++)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 16384), _listener._dispatchMessage.size());

    // //all received messages should be pitch bend
    for (int i = 0; i < _listener._dispatchMessage.size(); i++)
        TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::pitchBend, _listener._dispatchMessage.at(i).message);

    uint16_t expectedMIDIvalue = 0;

    for (int i = 0; i < _listener._dispatchMessage.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, _listener._dispatchMessage.at(index).midiValue);
        }

        expectedMIDIvalue++;
    }

    //now go backward
    _listener._dispatchMessage.clear();

    for (int i = 16383; i >= 0; i--)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    //one message less
    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 16383), _listener._dispatchMessage.size());

    expectedMIDIvalue = 16382;

    for (int i = 0; i < _listener._dispatchMessage.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, _listener._dispatchMessage.at(index).midiValue);
        }

        expectedMIDIvalue--;
    }
}

TEST_CASE(Inversion)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(_database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(_database.update(Database::Section::analog_t::type, i, Analog::type_t::potentiometerControlChange) == true);

        //set all lower limits to 0
        TEST_ASSERT(_database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        TEST_ASSERT(_database.update(Database::Section::analog_t::upperLimit, i, MIDI::MIDI_14_BIT_VALUE_MAX) == true);

        //midi channel
        TEST_ASSERT(_database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    //enable inversion
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 1) == true);

    for (int i = 0; i <= 127; i++)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 128), _listener._dispatchMessage.size());

    //first value should be 127
    //last value should be 0

    uint16_t expectedMIDIvalue = 127;

    for (int i = 0; i < _listener._dispatchMessage.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, _listener._dispatchMessage.at(index).midiValue);
        }

        expectedMIDIvalue--;
    }

    _listener._dispatchMessage.clear();

    //funky setup: set lower limit to 127, upper to 0 while inversion is enabled
    //result should be the same as when default setup is used (no inversion / 0 as lower limit, 127 as upper limit)

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 1) == true);
        TEST_ASSERT(_database.update(Database::Section::analog_t::lowerLimit, i, 127) == true);
        TEST_ASSERT(_database.update(Database::Section::analog_t::upperLimit, i, 0) == true);
        _analog.debounceReset(i);
    }

    //feed all the values again
    for (int i = 0; i <= 127; i++)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 128), _listener._dispatchMessage.size());

    expectedMIDIvalue = 0;

    for (int i = 0; i < _listener._dispatchMessage.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, _listener._dispatchMessage.at(index).midiValue);
        }

        expectedMIDIvalue++;
    }

    _listener._dispatchMessage.clear();

    //now disable inversion
    _listener._dispatchMessage.clear();

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 0) == true);
        _analog.debounceReset(i);
    }

    //feed all the values again
    for (int i = 0; i <= 127; i++)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 128), _listener._dispatchMessage.size());

    expectedMIDIvalue = 127;

    for (int i = 0; i < _listener._dispatchMessage.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, _listener._dispatchMessage.at(index).midiValue);
        }

        expectedMIDIvalue--;
    }
}

TEST_CASE(Scaling)
{
    using namespace IO;

    const uint32_t scaledLower = 11;
    const uint32_t scaledUpper = 100;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(_database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(_database.update(Database::Section::analog_t::type, i, Analog::type_t::potentiometerControlChange) == true);

        //set all lower limits to 0
        TEST_ASSERT(_database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to 100
        TEST_ASSERT(_database.update(Database::Section::analog_t::upperLimit, i, scaledUpper) == true);

        //midi channel
        TEST_ASSERT(_database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    for (int i = 0; i <= 127; i++)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    //since the values are scaled, verify that all the messages aren't received
    TEST_ASSERT((MAX_NUMBER_OF_ANALOG * 128) > _listener._dispatchMessage.size());

    //first value should be 0
    //last value should match the configured scaled value (scaledUpper)
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.at(i).midiValue);
        TEST_ASSERT_EQUAL_UINT32(scaledUpper, _listener._dispatchMessage.at(_listener._dispatchMessage.size() - MAX_NUMBER_OF_ANALOG + i).midiValue);
    }

    //now scale minimum value as well
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        TEST_ASSERT(_database.update(Database::Section::analog_t::lowerLimit, i, scaledLower) == true);

    _listener._dispatchMessage.clear();

    for (int i = 0; i <= 127; i++)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    TEST_ASSERT((MAX_NUMBER_OF_ANALOG * 128) > _listener._dispatchMessage.size());

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(_listener._dispatchMessage.at(i).midiValue >= scaledLower);
        TEST_ASSERT(_listener._dispatchMessage.at(_listener._dispatchMessage.size() - MAX_NUMBER_OF_ANALOG + i).midiValue <= scaledUpper);
    }

    //now enable inversion
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 1) == true);
        _analog.debounceReset(i);
    }

    _listener._dispatchMessage.clear();

    for (int i = 0; i <= 127; i++)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(_listener._dispatchMessage.at(i).midiValue >= scaledUpper);
        TEST_ASSERT(_listener._dispatchMessage.at(_listener._dispatchMessage.size() - MAX_NUMBER_OF_ANALOG + i).midiValue <= scaledLower);
    }
}

#endif