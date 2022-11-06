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
    database::Admin&         _databaseAdmin = _testDatabase._instance;
    protocol::MIDI::Database _database      = protocol::MIDI::Database(_testDatabase._instance);
    HWAMIDIUSB               _hwaMIDIUSB;
    HWAMIDIDIN               _hwaMIDIDIN;
    HWAMIDIBLE               _hwaMIDIBLE;
    protocol::MIDI           _instance = protocol::MIDI(_hwaMIDIUSB,
                                              _hwaMIDIDIN,
                                              _hwaMIDIBLE,
                                              _database);
};