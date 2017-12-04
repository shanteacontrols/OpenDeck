#pragma once

#include "midi/src/MIDI.h"

extern MIDI             midi;
extern volatile bool    MIDImessageReceived;
extern volatile bool    MIDImessageSent;