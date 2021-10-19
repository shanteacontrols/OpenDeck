#ifdef ENCODERS_SUPPORTED

#include "unity/Framework.h"
#include "io/encoders/Encoders.h"
#include "core/src/general/Timing.h"
#include "database/Database.h"
#include "stubs/database/DB_ReadWrite.h"
#include "stubs/EncodersFilter.h"
#include "stubs/Listener.h"

namespace
{
    class HWAEncoders : public IO::Encoders::HWA
    {
        public:
        HWAEncoders()
        {}

        bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
        {
            numberOfReadings = 1;
            states           = _state;

            return true;
        }

        // use the same state for all encoders
        uint32_t _state = 0;
    } _hwaEncoders;

    Util::MessageDispatcher _dispatcher;
    Listener                _listener;
    DBstorageMock           _dbStorageMock;
    Database                _database = Database(_dbStorageMock, true);
    EncodersFilterStub      _encodersFilter;
    IO::Encoders            _encoders = IO::Encoders(_hwaEncoders, _encodersFilter, 1, _database, _dispatcher);
}    // namespace

TEST_SETUP()
{
    // init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(_database.init() == true);

    static bool listenerActive = false;

    if (!listenerActive)
    {
        _dispatcher.listen(Util::MessageDispatcher::messageSource_t::encoders,
                           Util::MessageDispatcher::listenType_t::nonFwd,
                           [](const Util::MessageDispatcher::message_t& dispatchMessage) {
                               _listener.messageListener(dispatchMessage);
                           });

        listenerActive = true;
    }

    _listener._dispatchMessage.clear();
}

TEST_CASE(StateDecoding)
{
    using namespace IO;

    // set known state
    for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
    {
        // enable all encoders
        TEST_ASSERT(_database.update(Database::Section::encoder_t::enable, i, 1) == true);

        // disable invert state
        TEST_ASSERT(_database.update(Database::Section::encoder_t::invert, i, 0) == true);

        // set type of message to Encoders::type_t::controlChange7Fh01h
        TEST_ASSERT(_database.update(Database::Section::encoder_t::mode, i, Encoders::type_t::controlChange7Fh01h) == true);

        // set single pulse per step
        TEST_ASSERT(_database.update(Database::Section::encoder_t::pulsesPerStep, i, 1) == true);
    }

    auto setState = [](uint8_t state) {
        _hwaEncoders._state = state;
        _encoders.update();
    };

    auto verifyValue = [](MIDI::messageType_t message, uint16_t value) {
        for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
        {
            TEST_ASSERT_EQUAL_UINT32(message, _listener._dispatchMessage.at(i).message);
            TEST_ASSERT_EQUAL_UINT32(value, _listener._dispatchMessage.at(i).midiValue);
        }
    };

    // test expected permutations for ccw and cw directions

    // clockwise: 00, 10, 11, 01

    setState(0b00);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());
    _listener._dispatchMessage.clear();

    setState(0b10);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    setState(0b11);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    setState(0b01);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    setState(0b10);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());
    _listener._dispatchMessage.clear();

    setState(0b11);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    setState(0b01);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    setState(0b00);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    setState(0b11);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());
    _listener._dispatchMessage.clear();

    setState(0b01);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    setState(0b00);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    setState(0b10);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    setState(0b01);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());
    _listener._dispatchMessage.clear();

    setState(0b00);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    setState(0b10);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    setState(0b11);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    // counter-clockwise: 00, 01, 11, 10

    setState(0b00);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());
    _listener._dispatchMessage.clear();

    setState(0b01);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
    _listener._dispatchMessage.clear();

    setState(0b11);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
    _listener._dispatchMessage.clear();

    setState(0b10);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
    _listener._dispatchMessage.clear();

    setState(0b01);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());
    _listener._dispatchMessage.clear();

    setState(0b11);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
    _listener._dispatchMessage.clear();

    setState(0b10);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
    _listener._dispatchMessage.clear();

    setState(0b00);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
    _listener._dispatchMessage.clear();

    setState(0b11);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());
    _listener._dispatchMessage.clear();

    setState(0b10);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
    _listener._dispatchMessage.clear();

    setState(0b00);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
    _listener._dispatchMessage.clear();

    setState(0b01);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
    _listener._dispatchMessage.clear();

    setState(0b10);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());
    _listener._dispatchMessage.clear();

    setState(0b00);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
    _listener._dispatchMessage.clear();

    setState(0b01);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
    _listener._dispatchMessage.clear();

    setState(0b11);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
    _listener._dispatchMessage.clear();

    // this time configure 4 pulses per step
    for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
    {
        TEST_ASSERT(_database.update(Database::Section::encoder_t::pulsesPerStep, i, 4) == true);
        _encoders.resetValue(i);
    }

    // clockwise: 00, 10, 11, 01

    // initial state doesn't count as pulse, 4 more needed
    setState(0b00);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());

    // 1
    setState(0b10);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());

    // 2
    setState(0b11);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());

    // 3
    setState(0b01);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());

    // 4
    // pulse should be registered
    setState(0b00);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    // 1
    setState(0b10);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());

    // 2
    setState(0b11);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());

    // 3
    setState(0b01);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());

    // 4
    setState(0b00);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 1);
    _listener._dispatchMessage.clear();

    // now move to opposite direction
    // don't start from 0b00 state again
    // counter-clockwise: 01, 11, 10, 00

    setState(0b01);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());

    setState(0b11);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());

    setState(0b10);
    TEST_ASSERT_EQUAL_UINT32(0, _listener._dispatchMessage.size());

    setState(0b00);
    TEST_ASSERT_EQUAL_UINT32(MAX_NUMBER_OF_ENCODERS, _listener._dispatchMessage.size());
    verifyValue(MIDI::messageType_t::controlChange, 127);
}

#endif