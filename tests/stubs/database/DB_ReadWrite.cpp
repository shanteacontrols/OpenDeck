#include "DB_ReadWrite.h"
#include <string.h>

#define DB_SIZE (EEPROM_SIZE - 3)

uint32_t DBstorageMock::size()
{
    return DB_SIZE;
}

size_t DBstorageMock::paramUsage(LESSDB::sectionParameterType_t type)
{
    switch (type)
    {
    case LESSDB::sectionParameterType_t::word:
        return 2;

    case LESSDB::sectionParameterType_t::dword:
        return 4;

    case LESSDB::sectionParameterType_t::bit:
    case LESSDB::sectionParameterType_t::halfByte:
    case LESSDB::sectionParameterType_t::byte:
    default:
        return 1;
    }
}

bool DBstorageMock::read(uint32_t address, int32_t& value, LESSDB::sectionParameterType_t type)
{
    switch (type)
    {
    case LESSDB::sectionParameterType_t::bit:
    case LESSDB::sectionParameterType_t::byte:
    case LESSDB::sectionParameterType_t::halfByte:
        value = memoryArray[address];
        break;

    case LESSDB::sectionParameterType_t::word:
        value = memoryArray[address + 1];
        value <<= 8;
        value |= memoryArray[address + 0];
        break;

    default:
        // case LESSDB::sectionParameterType_t::dword:
        value = memoryArray[address + 3];
        value <<= 8;
        value |= memoryArray[address + 2];
        value <<= 8;
        value |= memoryArray[address + 1];
        value <<= 8;
        value |= memoryArray[address + 0];
        break;
    }

    return true;
}

bool DBstorageMock::write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type)
{
    switch (type)
    {
    case LESSDB::sectionParameterType_t::bit:
    case LESSDB::sectionParameterType_t::byte:
    case LESSDB::sectionParameterType_t::halfByte:
        memoryArray[address] = value;
        break;

    case LESSDB::sectionParameterType_t::word:
        memoryArray[address + 0] = (value >> 0) & (uint16_t)0xFF;
        memoryArray[address + 1] = (value >> 8) & (uint16_t)0xFF;
        break;

    default:
        // case LESSDB::sectionParameterType_t::dword:
        memoryArray[address + 0] = (value >> 0) & (uint32_t)0xFF;
        memoryArray[address + 1] = (value >> 8) & (uint32_t)0xFF;
        memoryArray[address + 2] = (value >> 16) & (uint32_t)0xFF;
        memoryArray[address + 3] = (value >> 24) & (uint32_t)0xFF;
        break;
    }

    return true;
}

void DBstorageMock::clear()
{
    memset(memoryArray, 0x00, DB_SIZE);
}