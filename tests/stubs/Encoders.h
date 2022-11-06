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
    database::Admin&       _databaseAdmin = _testDatabase._instance;
    io::Encoders::Database _database      = io::Encoders::Database(_testDatabase._instance);
    HWAEncoders            _hwa;
    EncodersFilterStub     _filter;
    io::Encoders           _instance = io::Encoders(_hwa, _filter, _database);
};