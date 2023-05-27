#pragma once

#include "HWALEDs.h"
#include "tests/stubs/Database.h"

class TestLEDs
{
    public:
    TestLEDs() = default;

    private:
    TestDatabase _testDatabase;

    public:
    database::Admin&   _databaseAdmin = _testDatabase._instance;
    io::LEDs::Database _database      = io::LEDs::Database(_testDatabase._instance);
    HWALEDs            _hwa;
    io::LEDs           _instance = io::LEDs(_hwa, _database);
};