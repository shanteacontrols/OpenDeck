#ifndef BOARDDEFSOPENDECK_H_
#define BOARDDEFSOPENDECK_H_

#include "PinManipulation.h"

#include <avr/cpufunc.h>

#define BOARD_OPENDECK_1

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

#define SR_DIN_DDR                  DDRD
#define SR_DIN_PIN_REGISTER         PIND
#define SR_DIN_PORT                 PORTD
#define SR_DIN_PIN                  5

#define SR_CLK_DDR                  DDRD
#define SR_CLK_PORT                 PORTD
#define SR_CLK_PIN                  0

#define SR_LATCH_DDR                DDRD
#define SR_LATCH_PORT               PORTD
#define SR_LATCH_PIN                1

#define DEC_DM_A0_DDR               DDRB
#define DEC_DM_A0_PORT              PORTB
#define DEC_DM_A0_PIN               4

#define DEC_DM_A1_DDR               DDRD
#define DEC_DM_A1_PORT              PORTD
#define DEC_DM_A1_PIN               6

#define DEC_DM_A2_DDR               DDRD
#define DEC_DM_A2_PORT              PORTD
#define DEC_DM_A2_PIN               4

#define DM_ROW_8_BIT                4
#define DM_ROW_7_BIT                5
#define DM_ROW_6_BIT                6
#define DM_ROW_5_BIT                7
#define DM_ROW_4_BIT                3
#define DM_ROW_3_BIT                2
#define DM_ROW_2_BIT                1
#define DM_ROW_1_BIT                0

#define DM_COLUMN_1                 0
#define DM_COLUMN_2                 7
#define DM_COLUMN_3                 2
#define DM_COLUMN_4                 1
#define DM_COLUMN_5                 4
#define DM_COLUMN_6                 3
#define DM_COLUMN_7                 5
#define DM_COLUMN_8                 6

//match pins on pcb
const uint8_t dmRowBitArray[] = { DM_ROW_1_BIT, DM_ROW_2_BIT, DM_ROW_3_BIT, DM_ROW_4_BIT, DM_ROW_5_BIT, DM_ROW_6_BIT, DM_ROW_7_BIT, DM_ROW_8_BIT };
const uint8_t dmColumnArray[] = { DM_COLUMN_1, DM_COLUMN_2, DM_COLUMN_3, DM_COLUMN_4, DM_COLUMN_5, DM_COLUMN_6, DM_COLUMN_7, DM_COLUMN_8 };

#define DEC_LM_A0_DDR               DDRB
#define DEC_LM_A0_PORT              PORTB
#define DEC_LM_A0_PIN               1

#define DEC_LM_A1_DDR               DDRB
#define DEC_LM_A1_PORT              PORTB
#define DEC_LM_A1_PIN               2

#define DEC_LM_A2_DDR               DDRB
#define DEC_LM_A2_PORT              PORTB
#define DEC_LM_A2_PIN               3

#define LED_ROW_1_DDR               DDRB
#define LED_ROW_1_PORT              PORTB
#define LED_ROW_1_PIN               7

#define LED_ROW_2_DDR               DDRD
#define LED_ROW_2_PORT              PORTD
#define LED_ROW_2_PIN               7

#define LED_ROW_3_DDR               DDRB
#define LED_ROW_3_PORT              PORTB
#define LED_ROW_3_PIN               5

#define LED_ROW_4_DDR               DDRC
#define LED_ROW_4_PORT              PORTC
#define LED_ROW_4_PIN               7

#define LED_ROW_5_DDR               DDRC
#define LED_ROW_5_PORT              PORTC
#define LED_ROW_5_PIN               6

#define LED_ROW_6_DDR               DDRB
#define LED_ROW_6_PORT              PORTB
#define LED_ROW_6_PIN               6

#define BTLDR_LED_DDR               DDRE
#define BTLDR_LED_PORT              PORTE
#define BTLDR_LED_PIN               6

#define MUX_S0_DDR                  DDRF
#define MUX_S0_PORT                 PORTF
#define MUX_S0_PIN                  5

#define MUX_S1_DDR                  DDRF
#define MUX_S1_PORT                 PORTF
#define MUX_S1_PIN                  7

#define MUX_S2_DDR                  DDRF
#define MUX_S2_PORT                 PORTF
#define MUX_S2_PIN                  4

#define MUX_S3_DDR                  DDRF
#define MUX_S3_PORT                 PORTF
#define MUX_S3_PIN                  6

#define MUX_PIN_1                   8
#define MUX_PIN_2                   9
#define MUX_PIN_3                   10
#define MUX_PIN_4                   11
#define MUX_PIN_5                   12
#define MUX_PIN_6                   13
#define MUX_PIN_7                   15
#define MUX_PIN_8                   15
#define MUX_PIN_9                   7
#define MUX_PIN_10                  6
#define MUX_PIN_11                  5
#define MUX_PIN_12                  4
#define MUX_PIN_13                  3
#define MUX_PIN_14                  2
#define MUX_PIN_15                  1
#define MUX_PIN_16                  0

const uint8_t muxPinOrderArray[] = {

    MUX_PIN_1,
    MUX_PIN_2,
    MUX_PIN_3,
    MUX_PIN_4,
    MUX_PIN_5,
    MUX_PIN_6,
    MUX_PIN_7,
    MUX_PIN_8,
    MUX_PIN_9,
    MUX_PIN_10,
    MUX_PIN_11,
    MUX_PIN_12,
    MUX_PIN_13,
    MUX_PIN_14,
    MUX_PIN_15,
    MUX_PIN_16

};

#define MUX_1_IN_DDR                DDRF
#define MUX_1_IN_PIN                0

#define MUX_2_IN_DDR                DDRF
#define MUX_2_IN_PIN                1

#define PIN_OUTPUT                  1
#define PIN_INPUT                   0

#define PIN_HIGH                    1
#define PIN_LOW                     0

#endif