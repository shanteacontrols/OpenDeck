#include "CInfo.h"

uint32_t lastCinfoMsgTime[DB_BLOCKS];

uint32_t getLastCinfoMsgTime(uint8_t block)
{
    return lastCinfoMsgTime[block];
}

void updateCinfoTime(uint8_t block)
{
    lastCinfoMsgTime[block] = rTimeMs();
}