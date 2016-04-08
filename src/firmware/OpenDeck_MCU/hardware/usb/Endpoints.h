#ifndef ENDPOINTS_H_
#define ENDPOINTS_H_

#include <avr/pgmspace.h>
#include "usb_common.h"

// 0: control
// 1: midi IN
// 2: midi OUT

#define ENDPOINT0_SIZE      64

#define MIDI_INTERFACE      0

#define MIDI_TX_ENDPOINT    1
#define MIDI_TX_SIZE        64
#define MIDI_TX_BUFFER      EP_DOUBLE_BUFFER

#define MIDI_RX_ENDPOINT    2
#define MIDI_RX_SIZE        64
#define MIDI_RX_BUFFER      EP_DOUBLE_BUFFER

#define NUM_ENDPOINTS       3

static const uint8_t PROGMEM endpoint_config_table[] = {

    1, EP_TYPE_BULK_IN, EP_SIZE(MIDI_TX_SIZE) | MIDI_TX_BUFFER,
    1, EP_TYPE_BULK_OUT, EP_SIZE(MIDI_RX_SIZE) | MIDI_RX_BUFFER

};

#endif