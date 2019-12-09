#include "EEPROM.h"
#include "stm32f4xx.h"

bool EmuEEPROM::init()
{
    /* Get Page0 status */
    uint32_t page1Status = (*(volatile uint32_t*)page1.startAddress);

    /* Get Page1 status */
    uint32_t page2Status = (*(volatile uint32_t*)page2.startAddress);

    /* Check for invalid header states and repair if necessary */
    switch (page1Status)
    {
    case static_cast<uint16_t>(pageStatus_t::erased):
        if (page2Status == static_cast<uint32_t>(pageStatus_t::valid))
        {
            /* Page0 erased, Page1 valid */
            /* Erase Page0 */
            erasePageLL(page1.sector);
        }
        else if (page2Status == static_cast<uint32_t>(pageStatus_t::receiving))
        {
            /* Page0 erased, Page1 receive */
            /* Erase Page0 */
            erasePageLL(page1.sector);

            /* Mark Page1 as valid */
            if (!write32LL(page2.startAddress, static_cast<uint32_t>(pageStatus_t::valid)))
                return false;
        }
        else
        {
            /* First EEPROM access (Page0 and Page1 are erased) or invalid state -> format EEPROM */
            /* Erase both Page0 and Page1 and set Page0 as valid page */
            if (!format())
                return false;
        }
        break;

    case static_cast<uint32_t>(pageStatus_t::receiving):
        if (page2Status == static_cast<uint32_t>(pageStatus_t::valid))
        {
            /* Page0 receive, Page1 valid */
            //restart the transfer process by first erasing page0 and then performing page transfer
            erasePageLL(page1.sector);

            if (pageTransfer() != writeStatus_t::ok)
                return false;
        }
        else if (page2Status == static_cast<uint32_t>(pageStatus_t::erased))
        {
            /* Page0 receive, Page1 erased */
            /* Erase Page1 */
            erasePageLL(page2.sector);

            /* Mark Page0 as valid */
            if (!write32LL(page1.startAddress, static_cast<uint32_t>(pageStatus_t::valid)))
                return false;
        }
        else
        {
            /* Invalid state -> format eeprom */
            /* Erase both Page0 and Page1 and set Page0 as valid page */
            if (!format())
                return false;
        }
        break;

    case static_cast<uint32_t>(pageStatus_t::valid):
        if (page2Status == static_cast<uint32_t>(pageStatus_t::valid))
        {
            /* Invalid state -> format eeprom */
            /* Erase both Page0 and Page1 and set Page0 as valid page */
            if (!format())
                return false;
        }
        else if (page2Status == static_cast<uint32_t>(pageStatus_t::erased))
        {
            /* Page0 valid, Page1 erased */
            /* Erase Page1 */
            erasePageLL(page2.sector);
        }
        else
        {
            /* Page0 valid, Page1 receive */
            //restart the transfer process by first erasing page1 and then performing page transfer
            erasePageLL(page2.sector);

            if (pageTransfer() != writeStatus_t::ok)
                return false;
        }
        break;

    default:
        /* Any other state -> format eeprom */
        /* Erase both Page0 and Page1 and set Page0 as valid page */
        if (!format())
            return false;
        break;
    }

    return true;
}

bool EmuEEPROM::format()
{
    /* Erase Page0 */
    if (!erasePageLL(page1.sector))
        return false;

    if (!write32LL(page1.startAddress, static_cast<uint32_t>(pageStatus_t::valid)))
        return false;

    /* Erase Page1 */
    if (!erasePageLL(page2.sector))
        return false;

    /* Return Page1 erase operation status */
    return true;
}

EmuEEPROM::readStatus_t EmuEEPROM::read(uint16_t address, uint16_t& data)
{
    uint16_t validPage;

    /* Get active Page for read operation */
    /* Check if there is no valid page */
    if (!findValidPage(pageOp_t::read, validPage))
        return readStatus_t::noPage;

    readStatus_t status = readStatus_t::noVar;

    //take into account 4-byte page header
    uint32_t pageStartAddress = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(validPage * EEPROM_PAGE_SIZE)) + 4;
    uint32_t pageEndAddress   = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(validPage * EEPROM_PAGE_SIZE)) + EEPROM_PAGE_SIZE - 2;

    /* Check each active page address starting from end */
    while (pageEndAddress > pageStartAddress)
    {
        /* Get the current location content to be compared with virtual address */
        uint16_t addressValue = (*(volatile uint16_t*)pageEndAddress);

        /* Compare the read address with the virtual address */
        if (addressValue == address)
        {
            /* Get content of Address-2 which is variable value */
            data   = (*(volatile uint16_t*)(pageEndAddress - 2));
            status = readStatus_t::ok;
            break;
        }
        else
        {
            /* Next address location */
            pageEndAddress = pageEndAddress - 4;
        }
    }

    return status;
}

EmuEEPROM::writeStatus_t EmuEEPROM::write(uint16_t address, uint16_t data)
{
    writeStatus_t status;

    /* Write the variable virtual address and value in the EEPROM */
    status = writeInternal(address, data);

    if (status == writeStatus_t::pageFull)
    {
        /* Perform Page transfer */
        status = pageTransfer();

        //write the variable again to a new page
        if (status == writeStatus_t::ok)
            status = writeInternal(address, data);
    }

    return status;
}

bool EmuEEPROM::findValidPage(pageOp_t operation, uint16_t& page)
{
    uint32_t page1Status = (*(volatile uint32_t*)page1.startAddress);
    uint32_t page2Status = (*(volatile uint32_t*)page2.startAddress);

    /* Write or read operation */
    switch (operation)
    {
    case pageOp_t::write:
        if (page2Status == static_cast<uint32_t>(pageStatus_t::valid))
        {
            /* Page0 receiving data */
            if (page1Status == static_cast<uint32_t>(pageStatus_t::receiving))
                page = 0;
            else
                page = 1;
        }
        else if (page1Status == static_cast<uint32_t>(pageStatus_t::valid))
        {
            /* Page1 receiving data */
            if (page2Status == static_cast<uint32_t>(pageStatus_t::receiving))
                page = 1;
            else
                page = 0;
        }
        else
        {
            /* No valid Page */
            return false;
        }
        break;

    case pageOp_t::read:
        if (page1Status == static_cast<uint32_t>(pageStatus_t::valid))
        {
            page = 0;
        }
        else if (page2Status == static_cast<uint32_t>(pageStatus_t::valid))
        {
            /* Page1 valid */
            page = 1;
        }
        else
        {
            /* No valid Page */
            return false;
        }
        break;

    default:
        /* Page0 valid */
        page = 0;
    }

    return true;
}

EmuEEPROM::writeStatus_t EmuEEPROM::writeInternal(uint16_t address, uint16_t data)
{
    if (address == 0xFFFF)
        return writeStatus_t::writeError;

    uint16_t validPage;

    if (!findValidPage(pageOp_t::write, validPage))
        return writeStatus_t::noPage;

    //take into account 4-byte page header
    uint32_t pageStartAddress = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(validPage * EEPROM_PAGE_SIZE)) + 4;
    uint32_t pageEndAddress   = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(validPage * EEPROM_PAGE_SIZE)) + EEPROM_PAGE_SIZE - 2;

    /* Check each active page address starting from begining */
    while (pageStartAddress < pageEndAddress)
    {
        if ((*(volatile uint32_t*)pageStartAddress) == 0xFFFFFFFF)
        {
            /* Set variable data */
            if (!write16LL(pageStartAddress, data))
                return writeStatus_t::writeError;

            /* Set variable virtual address */
            if (!write16LL(pageStartAddress + 2, address))
                return writeStatus_t::writeError;

            return writeStatus_t::ok;
        }
        else
        {
            /* Next address location */
            pageStartAddress = pageStartAddress + 4;
        }
    }

    return writeStatus_t::pageFull;
}

EmuEEPROM::writeStatus_t EmuEEPROM::pageTransfer()
{
    uint16_t validPage;

    /* Get active Page for read operation */
    if (!findValidPage(pageOp_t::read, validPage))
        return writeStatus_t::noPage;

    uint32_t newPageAddress = EEPROM_START_ADDRESS;
    uint16_t oldPageSector  = 0;

    if (validPage == 1)
    {
        /* New page address where variable will be moved to */
        newPageAddress = page1.startAddress;

        /* Old page ID where variable will be taken from */
        oldPageSector = page2.sector;
    }
    else if (validPage == 0)
    {
        /* New page address  where variable will be moved to */
        newPageAddress = page2.startAddress;

        /* Old page ID where variable will be taken from */
        oldPageSector = page1.sector;
    }

    if (!write32LL(newPageAddress, static_cast<uint32_t>(pageStatus_t::receiving)))
        return writeStatus_t::writeError;

    writeStatus_t eepromStatus;
    uint16_t      data;

    /* Transfer process: transfer variables from old to the new active page */
    for (uint32_t i = 0; i < EEPROM_SIZE; i++)
    {
        /* Read the other last variable updates */
        /* In case variable corresponding to the virtual address was found */
        if (read(i, data) == readStatus_t::ok)
        {
            /* Transfer the variable to the new active page */
            eepromStatus = writeInternal(i, data);

            if (eepromStatus != writeStatus_t::ok)
                return eepromStatus;
        }
    }

    /* Erase the old Page: Set old Page status to ERASED status */
    erasePageLL(oldPageSector);

    /* Set new Page status to VALID_PAGE status */
    if (!write32LL(newPageAddress, static_cast<uint32_t>(pageStatus_t::valid)))
        return writeStatus_t::writeError;

    return writeStatus_t::ok;
}

bool EmuEEPROM::erasePageLL(uint16_t page)
{
    FLASH_EraseInitTypeDef pEraseInit = {};

    pEraseInit.Banks        = FLASH_BANK_1;
    pEraseInit.NbSectors    = 1;
    pEraseInit.Sector       = page;
    pEraseInit.VoltageRange = EEPROM_VOLTAGE_RANGE;
    pEraseInit.TypeErase    = FLASH_TYPEERASE_SECTORS;

    uint32_t          eraseStatus;
    HAL_StatusTypeDef halStatus = HAL_FLASH_Unlock();

    if (halStatus == HAL_OK)
    {
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
        halStatus = HAL_FLASHEx_Erase(&pEraseInit, &eraseStatus);
    }

    HAL_FLASH_Lock();
    return (halStatus == HAL_OK) && (eraseStatus == 0xFFFFFFFFU);
}

bool EmuEEPROM::write16LL(uint32_t address, uint16_t data)
{
    HAL_StatusTypeDef halStatus = HAL_FLASH_Unlock();

    if (halStatus == HAL_OK)
    {
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
        halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, data);
    }

    HAL_FLASH_Lock();
    return (halStatus == HAL_OK);
}

bool EmuEEPROM::write32LL(uint32_t address, uint32_t data)
{
    HAL_StatusTypeDef halStatus = HAL_FLASH_Unlock();

    if (halStatus == HAL_OK)
    {
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
        halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
    }

    HAL_FLASH_Lock();
    return (halStatus == HAL_OK);
}