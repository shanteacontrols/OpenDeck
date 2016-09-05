#ifndef MIDI_CONFIG_H_
#define MIDI_CONFIG_H_

#include "../../hardware/uart/UART.h"

#define USE_SERIAL_PORT         uart

#define MIDI_CHANNEL_OMNI       0
#define MIDI_CHANNEL_OFF        17 // and over

#define MIDI_PITCHBEND_MIN      -8192
#define MIDI_PITCHBEND_MAX      8191

//maximum sysex length we can receive
#define MIDI_SYSEX_ARRAY_SIZE   45

#endif