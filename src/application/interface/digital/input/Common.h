#pragma once

#include <inttypes.h>

///
/// \brief Used for buttonPCinc/buttonPCdec messages when each button press/encoder rotation sends incremented or decremented PC value.
/// 16 entries in array are used for 16 MIDI channels.
///
extern uint8_t     lastPCvalue[16];