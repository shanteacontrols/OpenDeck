#pragma once

//uncomment if leds use reverse logic for setting on/off state
#define LED_INVERT

#define NUMBER_OF_MUX               2
#define NUMBER_OF_MUX_INPUTS        16

#define NUMBER_OF_LED_COLUMNS       8
#define NUMBER_OF_LED_ROWS          6

#define NUMBER_OF_BUTTON_COLUMNS    8
#define NUMBER_OF_BUTTON_ROWS       8

#define MAX_NUMBER_OF_ANALOG        (NUMBER_OF_MUX*NUMBER_OF_MUX_INPUTS)
#define MAX_NUMBER_OF_BUTTONS       (NUMBER_OF_BUTTON_COLUMNS*NUMBER_OF_BUTTON_ROWS)
#define MAX_NUMBER_OF_LEDS          (NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS)
#define MAX_NUMBER_OF_RGB_LEDS      (MAX_NUMBER_OF_LEDS/3)
#define MAX_NUMBER_OF_ENCODERS      (MAX_NUMBER_OF_BUTTONS/2)