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
    Database::Instance& _database = _testDatabase._instance;
    HWALEDs             _hwa;
    IO::LEDs            _instance = IO::LEDs(_hwa, _database);
};