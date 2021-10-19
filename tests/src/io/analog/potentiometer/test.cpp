#ifdef ANALOG_SUPPORTED

#include "unity/Framework.h"
#include "io/analog/Analog.h"
#include "io/buttons/Buttons.h"
#include "database/Database.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include "stubs/database/DB_ReadWrite.h"
#include "stubs/AnalogFilter.h"
#include "stubs/ButtonsFilter.h"
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

    class HWAButtons : public IO::Buttons::HWA
    {
        public:
        HWAButtons() {}

        bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
        {
            numberOfReadings = 1;
            states           = _state[index];
            return true;
        }

        bool _state[MAX_NUMBER_OF_BUTTONS] = {};
    } _hwaButtons;

    Util::MessageDispatcher _dispatcher;
    Listener                _listener;
    DBstorageMock           _dbStorageMock;
    Database                _database = Database(_dbStorageMock, true);
    AnalogFilterStub        _analogFilter;
    ButtonsFilterStub       _buttonsFilter;
    IO::Analog              _analog(_hwaAnalog, _analogFilter, _database, _dispatcher);
    IO::Buttons             _buttons(_hwaButtons, _buttonsFilter, _database, _dispatcher);
}    // namespace

TEST_SETUP()
{
    // init checks - no point in running further tests if these conditions fail
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

    // set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        // enable all analog components
        TEST_ASSERT(_database.update(Database::Section::analog_t::enable, i, 1) == true);

        // disable invert state
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 0) == true);

        // configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(_database.update(Database::Section::analog_t::type, i, Analog::type_t::potentiometerControlChange) == true);

        // set all lower limits to 0
        TEST_ASSERT(_database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        // set all upper limits to MIDI_7_BIT_VALUE_MAX
        TEST_ASSERT(_database.update(Database::Section::analog_t::upperLimit, i, MIDI::MIDI_7_BIT_VALUE_MAX) == true);

        // midi channel
        TEST_ASSERT(_database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    // feed all the values from minimum to maximum
    // expect the following:
    // first value is 0
    // last value is 127

    for (int i = 0; i <= 127; i++)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 128), _listener._dispatchMessage.size());

    // all received messages should be control change
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

    // now go backward

    _listener._dispatchMessage.clear();

    for (int i = 127; i >= 0; i--)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    // expect one message less since the last one was 127
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

    // try to feed value larger than 127
    // no response should be received
    _hwaAnalog.adcReturnValue = 10000;
    _analog.update();

    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());
}

TEST_CASE(PitchBendTest)
{
    using namespace IO;

    // set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        // enable all analog components
        TEST_ASSERT(_database.update(Database::Section::analog_t::enable, i, 1) == true);

        // disable invert state
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 0) == true);

        // configure all analog components as potentiometers with Pitch Bend MIDI message
        TEST_ASSERT(_database.update(Database::Section::analog_t::type, i, Analog::type_t::pitchBend) == true);

        // set all lower limits to 0
        TEST_ASSERT(_database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        // set all upper limits to MIDI_14_BIT_VALUE_MAX
        TEST_ASSERT(_database.update(Database::Section::analog_t::upperLimit, i, MIDI::MIDI_14_BIT_VALUE_MAX) == true);

        // midi channel
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

    // now go backward
    _listener._dispatchMessage.clear();

    for (int i = 16383; i >= 0; i--)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    // one message less
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

    // set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        // enable all analog components
        TEST_ASSERT(_database.update(Database::Section::analog_t::enable, i, 1) == true);

        // disable invert state
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 0) == true);

        // configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(_database.update(Database::Section::analog_t::type, i, Analog::type_t::potentiometerControlChange) == true);

        // set all lower limits to 0
        TEST_ASSERT(_database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        // set all upper limits to MIDI_14_BIT_VALUE_MAX
        TEST_ASSERT(_database.update(Database::Section::analog_t::upperLimit, i, MIDI::MIDI_14_BIT_VALUE_MAX) == true);

        // midi channel
        TEST_ASSERT(_database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    // enable inversion
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 1) == true);

    for (int i = 0; i <= 127; i++)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 128), _listener._dispatchMessage.size());

    // first value should be 127
    // last value should be 0

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

    // funky setup: set lower limit to 127, upper to 0 while inversion is enabled
    // result should be the same as when default setup is used (no inversion / 0 as lower limit, 127 as upper limit)

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 1) == true);
        TEST_ASSERT(_database.update(Database::Section::analog_t::lowerLimit, i, 127) == true);
        TEST_ASSERT(_database.update(Database::Section::analog_t::upperLimit, i, 0) == true);
        _analog.debounceReset(i);
    }

    // feed all the values again
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

    // now disable inversion
    _listener._dispatchMessage.clear();

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 0) == true);
        _analog.debounceReset(i);
    }

    // feed all the values again
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

    // set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        // enable all analog components
        TEST_ASSERT(_database.update(Database::Section::analog_t::enable, i, 1) == true);

        // disable invert state
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 0) == true);

        // configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(_database.update(Database::Section::analog_t::type, i, Analog::type_t::potentiometerControlChange) == true);

        // set all lower limits to 0
        TEST_ASSERT(_database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        // set all upper limits to 100
        TEST_ASSERT(_database.update(Database::Section::analog_t::upperLimit, i, scaledUpper) == true);

        // midi channel
        TEST_ASSERT(_database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    for (int i = 0; i <= 127; i++)
    {
        _hwaAnalog.adcReturnValue = i;
        _analog.update();
    }

    // since the values are scaled, verify that all the messages aren't received
    TEST_ASSERT((MAX_NUMBER_OF_ANALOG * 128) > _listener._dispatchMessage.size());

    // first value should be 0
    // last value should match the configured scaled value (scaledUpper)
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.at(i).midiValue);
        TEST_ASSERT_EQUAL_UINT32(scaledUpper, _listener._dispatchMessage.at(_listener._dispatchMessage.size() - MAX_NUMBER_OF_ANALOG + i).midiValue);
    }

    // now scale minimum value as well
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

    // now enable inversion
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

TEST_CASE(ButtonForwarding)
{
    using namespace IO;

    // set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        // enable all analog components
        TEST_ASSERT(_database.update(Database::Section::analog_t::enable, i, 1) == true);

        // disable invert state
        TEST_ASSERT(_database.update(Database::Section::analog_t::invert, i, 0) == true);

        // configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(_database.update(Database::Section::analog_t::type, i, Analog::type_t::potentiometerControlChange) == true);

        // set all lower limits to 0
        TEST_ASSERT(_database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        // set all upper limits to MIDI_7_BIT_VALUE_MAX
        TEST_ASSERT(_database.update(Database::Section::analog_t::upperLimit, i, MIDI::MIDI_7_BIT_VALUE_MAX) == true);

        // midi channel
        TEST_ASSERT(_database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    // configure one analog component to be button type
    const size_t index = 1;
    TEST_ASSERT(_database.update(Database::Section::analog_t::type, index, Analog::type_t::button) == true);

    // configure button with the same index (+offset) to certain parameters
    TEST_ASSERT(_database.update(Database::Section::button_t::type, MAX_NUMBER_OF_BUTTONS + index, Buttons::type_t::momentary) == true);
    TEST_ASSERT(_database.update(Database::Section::button_t::midiChannel, MAX_NUMBER_OF_BUTTONS + index, 2) == true);
    TEST_ASSERT(_database.update(Database::Section::button_t::midiMessage, MAX_NUMBER_OF_BUTTONS + index, Buttons::messageType_t::controlChangeReset) == true);
    TEST_ASSERT(_database.update(Database::Section::button_t::velocity, MAX_NUMBER_OF_BUTTONS + index, 100) == true);

    std::vector<Util::MessageDispatcher::message_t> dispatchMessageAnalogFwd;
    std::vector<Util::MessageDispatcher::message_t> dispatchMessageButtons;

    _dispatcher.listen(Util::MessageDispatcher::messageSource_t::analog,
                       Util::MessageDispatcher::listenType_t::forward,
                       [&](const Util::MessageDispatcher::message_t& dispatchMessage) {
                           dispatchMessageAnalogFwd.push_back(dispatchMessage);
                       });

    _dispatcher.listen(Util::MessageDispatcher::messageSource_t::buttons,
                       Util::MessageDispatcher::listenType_t::nonFwd,
                       [&](const Util::MessageDispatcher::message_t& dispatchMessage) {
                           dispatchMessageButtons.push_back(dispatchMessage);
                       });

    _hwaAnalog.adcReturnValue = 0xFFFF;
    _analog.update();

    // analog class should just forward the message with the original component index
    // the rest of the message doesn't matter
    TEST_ASSERT_EQUAL_UINT32(1, dispatchMessageAnalogFwd.size());
    TEST_ASSERT_EQUAL_UINT32(index, dispatchMessageAnalogFwd.at(0).componentIndex);

    // button class should receive this message and push new button message with index being MAX_NUMBER_OF_BUTTONS + index
    TEST_ASSERT_EQUAL_UINT32(1, dispatchMessageButtons.size());
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_BUTTONS + index, dispatchMessageButtons.at(0).componentIndex);

    // verify the rest of the message

    // midi index should be the original one - indexing restarts for each new button group (physical buttons / analog / touchscreen)
    TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::controlChange, dispatchMessageButtons.at(0).message);
    TEST_ASSERT_EQUAL_UINT32(index, dispatchMessageButtons.at(0).midiIndex);
    TEST_ASSERT_EQUAL_UINT32(100, dispatchMessageButtons.at(0).midiValue);
    TEST_ASSERT_EQUAL_UINT32(2, dispatchMessageButtons.at(0).midiChannel);

    _hwaAnalog.adcReturnValue = 0;
    _analog.update();

    // since the button was configured in controlChangeReset reset mode, another message should be sent from buttons
    TEST_ASSERT_EQUAL_UINT32(2, dispatchMessageAnalogFwd.size());
    TEST_ASSERT_EQUAL_UINT32(index, dispatchMessageAnalogFwd.at(1).componentIndex);
    TEST_ASSERT_EQUAL_UINT32(2, dispatchMessageButtons.size());
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_BUTTONS + index, dispatchMessageButtons.at(1).componentIndex);
    TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::controlChange, dispatchMessageButtons.at(1).message);
    TEST_ASSERT_EQUAL_UINT32(index, dispatchMessageButtons.at(1).midiIndex);
    TEST_ASSERT_EQUAL_UINT32(0, dispatchMessageButtons.at(1).midiValue);
    TEST_ASSERT_EQUAL_UINT32(2, dispatchMessageButtons.at(1).midiChannel);

    // repeat the update - analog class sends forwarding message again since it's not in charge of filtering it
    // buttons class shouldn't send anything new since the state of the button hasn't changed
    _analog.update();

    TEST_ASSERT_EQUAL_UINT32(3, dispatchMessageAnalogFwd.size());
    TEST_ASSERT_EQUAL_UINT32(2, dispatchMessageButtons.size());

    // similar test with the button message type being normal CC
    dispatchMessageButtons.clear();
    dispatchMessageAnalogFwd.clear();

    TEST_ASSERT(_database.update(Database::Section::button_t::midiMessage, MAX_NUMBER_OF_BUTTONS + index, Buttons::messageType_t::controlChange) == true);

    _hwaAnalog.adcReturnValue = 0xFFFF;
    _analog.update();

    TEST_ASSERT_EQUAL_UINT32(1, dispatchMessageAnalogFwd.size());
    TEST_ASSERT_EQUAL_UINT32(index, dispatchMessageAnalogFwd.at(0).componentIndex);
    TEST_ASSERT_EQUAL_UINT32(1, dispatchMessageButtons.size());
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_BUTTONS + index, dispatchMessageButtons.at(0).componentIndex);
    TEST_ASSERT_EQUAL_UINT32(MIDI::messageType_t::controlChange, dispatchMessageButtons.at(0).message);
    TEST_ASSERT_EQUAL_UINT32(index, dispatchMessageButtons.at(0).midiIndex);
    TEST_ASSERT_EQUAL_UINT32(100, dispatchMessageButtons.at(0).midiValue);
    TEST_ASSERT_EQUAL_UINT32(2, dispatchMessageButtons.at(0).midiChannel);

    _hwaAnalog.adcReturnValue = 0;
    _analog.update();

    // this time buttons shouldn't send anything new since standard CC is a latching message
    TEST_ASSERT_EQUAL_UINT32(2, dispatchMessageAnalogFwd.size());
    TEST_ASSERT_EQUAL_UINT32(1, dispatchMessageButtons.size());
}

#endif