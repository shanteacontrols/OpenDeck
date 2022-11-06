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
        for (size_t i = 0; i < io::Analog::Collection::SIZE(); i++)
        {
            _filter.updateLastValue(i, value);
        }
    }

    private:
    TestDatabase _testDatabase;

    public:
    database::Admin&     _databaseAdmin = _testDatabase._instance;
    io::Analog::Database _database      = io::Analog::Database(_testDatabase._instance);
    HWAAnalog            _hwa;
    AnalogFilterStub     _filter;
    io::Analog           _instance = io::Analog(_hwa, _filter, _database);
};