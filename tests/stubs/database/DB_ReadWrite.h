#pragma once

#include <inttypes.h>
#include "dbms/src/LESSDB.h"

class DBstorageMock : public LESSDB::StorageAccess
{
    public:
    DBstorageMock() {}

    void init() override
    {
    }

    uint32_t size() override;
    size_t   paramUsage(LESSDB::sectionParameterType_t type) override;
    void     clear() override;
    bool     read(uint32_t address, int32_t& value, LESSDB::sectionParameterType_t type) override;
    bool     write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type) override;

    private:
    uint8_t memoryArray[DATABASE_SIZE] = {};
};