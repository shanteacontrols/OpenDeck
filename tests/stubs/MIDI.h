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
    Database::Admin&         _databaseAdmin = _testDatabase._instance;
    Protocol::MIDI::Database _database      = Protocol::MIDI::Database(_testDatabase._instance);
    HWAMIDIUSB               _hwaMIDIUSB;
    HWAMIDIDIN               _hwaMIDIDIN;
    HWAMIDIBLE               _hwaMIDIBLE;
    Protocol::MIDI           _instance = Protocol::MIDI(_hwaMIDIUSB,
                                              _hwaMIDIDIN,
                                              _hwaMIDIBLE,
                                              _database);
};