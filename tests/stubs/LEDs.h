#pragma once

#include "HWALEDs.h"
#include "stubs/Database.h"

class TestLEDs
{
    public:
    TestLEDs() = default;

    private:
    TestDatabase _testDatabase;

    public:
    Database::Admin&   _databaseAdmin = _testDatabase._instance;
    IO::LEDs::Database _database      = IO::LEDs::Database(_testDatabase._instance);
    HWALEDs            _hwa;
    IO::LEDs           _instance = IO::LEDs(_hwa, _database);
};