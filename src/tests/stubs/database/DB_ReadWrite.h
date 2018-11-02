#pragma once

#include <inttypes.h>
#include "dbms/src/DataTypes.h"

namespace DatabaseStub
{
    extern uint8_t memoryArray[LESSDB_SIZE];

    bool memoryRead(uint32_t address, sectionParameterType_t type, int32_t &value);
    bool memoryWrite(uint32_t address, int32_t value, sectionParameterType_t type);
}