#pragma once

#include "../../database/blocks/Blocks.h"

#define COMPONENT_INFO_TIMEOUT          500 //ms

extern uint32_t lastCinfoMsgTime[DB_BLOCKS];

uint32_t getLastCinfoMsgTime(uint8_t block);
void updateCinfoTime(uint8_t block);