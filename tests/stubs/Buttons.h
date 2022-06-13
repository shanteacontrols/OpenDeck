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
    Database::Admin&      _databaseAdmin = _testDatabase._instance;
    IO::Buttons::Database _database      = IO::Buttons::Database(_testDatabase._instance);
    HWAButtons            _hwa;
    ButtonsFilterStub     _filter;
    IO::Buttons           _instance = IO::Buttons(_hwa, _filter, _database);
};