#pragma once

#include "ButtonsFilter.h"
#include "HWAButtons.h"
#include "stubs/Database.h"

class TestButtons
{
    public:
    TestButtons() = default;

    private:
    TestDatabase _testDatabase;

    public:
    database::Admin&      _databaseAdmin = _testDatabase._instance;
    io::Buttons::Database _database      = io::Buttons::Database(_testDatabase._instance);
    HWAButtons            _hwa;
    ButtonsFilterStub     _filter;
    io::Buttons           _instance = io::Buttons(_hwa, _filter, _database);
};