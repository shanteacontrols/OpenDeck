#ifndef FIRMWARE_VERSION_H_
#define FIRMWARE_VERSION_H_

#include <inttypes.h>
#include "../Types.h"

bool checkNewRevision();
uint8_t getSWversion(swVersion_t point);

#endif