#include "DB_ReadWrite.h"
#include <string.h>

uint32_t DBstorageMock::size()
{
#ifdef STM32_EMU_EEPROM
    // first 4 bytes are reserved for page status
    return EMU_EEPROM_PAGE_SIZE - 4;
#else
    return memoryArray.size();
#endif
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
        return 4;    // 2 bytes for address, 2 bytes for data
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
    {
        value = memoryArray.at(address);
    }
    break;

    case LESSDB::sectionParameterType_t::word:
    {
        value = memoryArray.at(address + 1);
        value <<= 8;
        value |= memoryArray.at(address + 0);
    }
    break;

    default:
    {
        // case LESSDB::sectionParameterType_t::dword:
        value = memoryArray.at(address + 3);
        value <<= 8;
        value |= memoryArray.at(address + 2);
        value <<= 8;
        value |= memoryArray.at(address + 1);
        value <<= 8;
        value |= memoryArray.at(address + 0);
    }
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
    {
        auto readStatus = emuEEPROM.read(address, tempData);

        if (readStatus == EmuEEPROM::readStatus_t::ok)
        {
            value = tempData;
        }
        else if (readStatus == EmuEEPROM::readStatus_t::noVar)
        {
            // variable with this address doesn't exist yet - set value to 0
            value = 0;
        }
        else
        {
            return false;
        }
    }
    break;

    default:
        return false;
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
    {
        memoryArray.at(address) = value;
    }
    break;

    case LESSDB::sectionParameterType_t::word:
    {
        memoryArray.at(address + 0) = (value >> 0) & (uint16_t)0xFF;
        memoryArray.at(address + 1) = (value >> 8) & (uint16_t)0xFF;
    }
    break;

    default:
    {
        // case LESSDB::sectionParameterType_t::dword:
        memoryArray.at(address + 0) = (value >> 0) & (uint32_t)0xFF;
        memoryArray.at(address + 1) = (value >> 8) & (uint32_t)0xFF;
        memoryArray.at(address + 2) = (value >> 16) & (uint32_t)0xFF;
        memoryArray.at(address + 3) = (value >> 24) & (uint32_t)0xFF;
    }
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
    {
        tempData = value;

        if (emuEEPROM.write(address, tempData) != EmuEEPROM::writeStatus_t::ok)
            return false;
    }
    break;

    default:
        return false;
    }

    return true;
#endif
}

bool DBstorageMock::clear()
{
#ifndef STM32_EMU_EEPROM
    std::fill(memoryArray.begin(), memoryArray.end(), 0x00);
    return true;
#else
    return emuEEPROM.format();
#endif
}

#ifdef STM32_EMU_EEPROM
DBstorageMock::EmuEEPROMStorageAccess::EmuEEPROMStorageAccess()
{
    pageArray.resize(EMU_EEPROM_PAGE_SIZE * 2, 0xFF);
}

bool DBstorageMock::EmuEEPROMStorageAccess::init()
{
    return true;
}

uint32_t DBstorageMock::EmuEEPROMStorageAccess::startAddress(EmuEEPROM::page_t page)
{
    if (page == EmuEEPROM::page_t::page1)
        return 0;
    else
        return EMU_EEPROM_PAGE_SIZE;
}

bool DBstorageMock::EmuEEPROMStorageAccess::erasePage(EmuEEPROM::page_t page)
{
    if (page == EmuEEPROM::page_t::page1)
        std::fill(pageArray.begin(), pageArray.end() - EMU_EEPROM_PAGE_SIZE, 0xFF);
    else
        std::fill(pageArray.begin() + EMU_EEPROM_PAGE_SIZE, pageArray.end(), 0xFF);

    return true;
}

bool DBstorageMock::EmuEEPROMStorageAccess::write16(uint32_t address, uint16_t data)
{
    // 0->1 transition is not allowed
    uint16_t currentData = 0;
    read16(address, currentData);

    if (data > currentData)
        return false;

    pageArray.at(address + 0) = (data >> 0) & (uint16_t)0xFF;
    pageArray.at(address + 1) = (data >> 8) & (uint16_t)0xFF;

    return true;
}

bool DBstorageMock::EmuEEPROMStorageAccess::write32(uint32_t address, uint32_t data)
{
    // 0->1 transition is not allowed
    uint32_t currentData = 0;
    read32(address, currentData);

    if (data > currentData)
        return false;

    pageArray.at(address + 0) = (data >> 0) & (uint32_t)0xFF;
    pageArray.at(address + 1) = (data >> 8) & (uint32_t)0xFF;
    pageArray.at(address + 2) = (data >> 16) & (uint32_t)0xFF;
    pageArray.at(address + 3) = (data >> 24) & (uint32_t)0xFF;

    return true;
}

bool DBstorageMock::EmuEEPROMStorageAccess::read16(uint32_t address, uint16_t& data)
{
    data = pageArray.at(address + 1);
    data <<= 8;
    data |= pageArray.at(address + 0);

    return true;
}

bool DBstorageMock::EmuEEPROMStorageAccess::read32(uint32_t address, uint32_t& data)
{
    data = pageArray.at(address + 3);
    data <<= 8;
    data |= pageArray.at(address + 2);
    data <<= 8;
    data |= pageArray.at(address + 1);
    data <<= 8;
    data |= pageArray.at(address + 0);

    return true;
}
#endif