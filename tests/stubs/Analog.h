#pragma once

#include "AnalogFilter.h"
#include "HWAAnalog.h"
#include "stubs/Database.h"

class TestAnalog
{
    public:
    TestAnalog() = default;

    void updateLastFilterValue(uint16_t value)
    {
        for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
        {
            _filter.updateLastValue(i, value);
        }
    }

    private:
    TestDatabase _testDatabase;

    public:
    Database::Admin&     _databaseAdmin = _testDatabase._instance;
    IO::Analog::Database _database      = IO::Analog::Database(_testDatabase._instance);
    HWAAnalog            _hwa;
    AnalogFilterStub     _filter;
    IO::Analog           _instance = IO::Analog(_hwa, _filter, _database);
};