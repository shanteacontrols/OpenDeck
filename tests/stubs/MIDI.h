#pragma once

#include "HWAMIDI.h"
#include "Database.h"

class TestMIDI
{
    public:
    TestMIDI() = default;

    private:
    TestDatabase _testDatabase;

    public:
    Database::Instance& _database = _testDatabase._instance;
    HWAMIDIUSB          _hwaMIDIUSB;
    HWAMIDIDIN          _hwaMIDIDIN;
    HWAMIDIBLE          _hwaMIDIBLE;
    Protocol::MIDI      _instance = Protocol::MIDI(_hwaMIDIUSB,
                                              _hwaMIDIDIN,
                                              _hwaMIDIBLE,
                                              _database);
};