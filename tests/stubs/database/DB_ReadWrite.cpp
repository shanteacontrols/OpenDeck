#include "DB_ReadWrite.h"
#include <string.h>

uint32_t DBstorageMock::size()
{
    return DATABASE_SIZE;
}

size_t DBstorageMock::paramUsage(LESSDB::sectionParameterType_t type)
{
#ifndef STM32_EMU_EEPROM
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
#else
    switch (type)
    {
    case LESSDB::sectionParameterType_t::dword:
        return 8;

    case LESSDB::sectionParameterType_t::bit:
    case LESSDB::sectionParameterType_t::halfByte:
    case LESSDB::sectionParameterType_t::byte:
    case LESSDB::sectionParameterType_t::word:
    default:
        return 4;    //2 bytes for address, 2 bytes for data
    }
#endif
}

bool DBstorageMock::read(uint32_t address, int32_t& value, LESSDB::sectionParameterType_t type)
{
#ifndef STM32_EMU_EEPROM
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
#else
    uint16_t tempData;

    switch (type)
    {
    case LESSDB::sectionParameterType_t::bit:
    case LESSDB::sectionParameterType_t::byte:
    case LESSDB::sectionParameterType_t::halfByte:
    case LESSDB::sectionParameterType_t::word:
        if (eepromMemory[address] != 0xFFFF)
        {
            value = eepromMemory[address];
        }
        else
        {
            auto readStatus = emuEEPROM.read(address, tempData);

            if (readStatus == EmuEEPROM::readStatus_t::ok)
            {
                value                 = tempData;
                eepromMemory[address] = tempData;
            }
            else if (readStatus == EmuEEPROM::readStatus_t::noVar)
            {
                //variable with this address doesn't exist yet - set value to 0
                value                 = 0;
                eepromMemory[address] = tempData;
            }
            else
            {
                return false;
            }
        }
        break;

    default:
        return false;
        break;
    }

    return true;
#endif
}

bool DBstorageMock::write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type)
{
#ifndef STM32_EMU_EEPROM
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
#else
    uint16_t tempData;

    switch (type)
    {
    case LESSDB::sectionParameterType_t::bit:
    case LESSDB::sectionParameterType_t::byte:
    case LESSDB::sectionParameterType_t::halfByte:
    case LESSDB::sectionParameterType_t::word:
        tempData              = value;
        eepromMemory[address] = value;
        if (emuEEPROM.write(address, tempData) != EmuEEPROM::writeStatus_t::ok)
            return false;
        break;

    default:
        return false;
        break;
    }

    return true;
#endif
}

bool DBstorageMock::clear()
{
#ifndef STM32_EMU_EEPROM
    memset(memoryArray, 0x00, DATABASE_SIZE);
    return true;
#else
    return emuEEPROM.format();
#endif
}