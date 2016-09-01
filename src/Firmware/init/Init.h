#ifndef INIT_H_
#define INIT_H_

#include "../hardware/core/Core.h"
#include "../interface/analog/Analog.h"
#include "../interface/buttons/Buttons.h"
#include "../interface/encoders/Encoders.h"
#include "../interface/leds/LEDs.h"
#include "../eeprom/Configuration.h"
#include "../sysex/SysEx.h"
#include "../hardware/reset/Reset.h"
#include "../interface/settings/Settings.h"
#include "../version/Firmware.h"
#include "../version/Hardware.h"

void globalInit();

#endif