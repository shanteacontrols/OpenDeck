#ifndef BOARDDEFSOPENDECK_H_
#define BOARDDEFSOPENDECK_H_

#define BOARD
#define BOARD_OPENDECK_1
//#define TANNIN_2_PROTOTYPE

#define NUMBER_OF_MUX               2
#define NUMBER_OF_LED_COLUMNS       8
#define NUMBER_OF_BUTTON_COLUMNS    8
#define NUMBER_OF_BUTTON_ROWS       4
#define NUMBER_OF_LED_ROWS          4

#ifdef TANNIN_2_PROTOTYPE
    #define NUMBER_OF_ENCODERS          4
#else
    #define NUMBER_OF_ENCODERS          1
#endif



#endif