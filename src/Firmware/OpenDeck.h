#pragma once

#include "sysex/src/SysEx.h"

#define FIRMWARE_VERSION_STRING         0x56
#define HARDWARE_VERSION_STRING         0x42
#define REBOOT_APP_STRING               0x7F
#define REBOOT_BTLDR_STRING             0x55
#define FACTORY_RESET_STRING            0x44
#define COMPONENT_ID_STRING             0x49
#define MAX_COMPONENTS_STRING           0x4D

extern SysEx sysEx;