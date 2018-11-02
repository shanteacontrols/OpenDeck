#include "DB_ReadWrite.h"

namespace DatabaseStub
{
    uint8_t memoryArray[LESSDB_SIZE] = {};

    bool memoryRead(uint32_t address, sectionParameterType_t type, int32_t &value)
    {
        switch(type)
        {
            case BIT_PARAMETER:
            case BYTE_PARAMETER:
            case HALFBYTE_PARAMETER:
            value = memoryArray[address];
            break;

            case WORD_PARAMETER:
            value = memoryArray[address+1];
            value <<= 8;
            value |= memoryArray[address+0];
            break;

            default:
            // case DWORD_PARAMETER:
            value = memoryArray[address+3];
            value <<= 8;
            value |= memoryArray[address+2];
            value <<= 8;
            value |= memoryArray[address+1];
            value <<= 8;
            value |= memoryArray[address+0];
            break;
        }

        return true;
    }

    bool memoryWrite(uint32_t address, int32_t value, sectionParameterType_t type)
    {
        switch(type)
        {
            case BIT_PARAMETER:
            case BYTE_PARAMETER:
            case HALFBYTE_PARAMETER:
            memoryArray[address] = value;
            break;

            case WORD_PARAMETER:
            memoryArray[address+0] = (value >> 0) & (uint16_t)0xFF;
            memoryArray[address+1] = (value >> 8) & (uint16_t)0xFF;
            break;

            default:
            // case DWORD_PARAMETER:
            memoryArray[address+0] = (value >> 0) & (uint32_t)0xFF;
            memoryArray[address+1] = (value >> 8) & (uint32_t)0xFF;
            memoryArray[address+2] = (value >> 16) & (uint32_t)0xFF;
            memoryArray[address+3] = (value >> 24) & (uint32_t)0xFF;
            break;
        }

        return true;
    }
}