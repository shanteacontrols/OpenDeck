#pragma once

#include "EncodersFilter.h"
#include "HWAEncoders.h"
#include "stubs/Database.h"

class TestEncoders
{
    public:
    TestEncoders() = default;

    private:
    TestDatabase _testDatabase;

    public:
    Database::Admin&       _databaseAdmin = _testDatabase._instance;
    IO::Encoders::Database _database      = IO::Encoders::Database(_testDatabase._instance);
    HWAEncoders            _hwa;
    EncodersFilterStub     _filter;
    IO::Encoders           _instance = IO::Encoders(_hwa, _filter, _database);
};