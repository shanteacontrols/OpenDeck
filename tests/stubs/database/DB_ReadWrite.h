#pragma once

#include <inttypes.h>
#include "dbms/src/LESSDB.h"

namespace DatabaseStub
{
    extern uint8_t memoryArray[EEPROM_SIZE - 3];

    bool read(uint32_t address, LESSDB::sectionParameterType_t type, int32_t& value);
    bool write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type);
}    // namespace DatabaseStub