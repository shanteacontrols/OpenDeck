/*

Hardware specific library for reading data from multiple sources
v1.0
Last revision date: 2014-05-17
Author: Igor Petrovic

*/

#ifndef HardwareReadSpecific_h
#define HardwareReadSpecific_h

#include <avr/io.h>


/*
    ###############################################################
    #                                                             #
    #					CONFIGURATION AREA                        #
    #                                                             #
    ###############################################################
*/

#define BUTTON_MATRIX
#define LED_MATRIX

#define NUMBER_OF_COLUMNS

/*

	ADC pins defining guide:
	
	#define ADC_CHANNEL_NUMBER_TYPE
	
	Where number is ATmega328 analog input (0-7) and type is
	either MUX or POT.
	
	So, if there is only single pot connected to ATmega on channel 2, you only need
	to define that single pin:
	
	#define ADC_CHANNEL_2_POT
	
	Number of both multiplexers and pots directly connected must be defined (if they're connected that is)
	
	Example 1:
	
	#define ADC_CHANNEL_0_MUX
	#define ADC_CHANNEL_1_MUX
	
	#define NUMBER_OF_MUX 2
	
	
	Example 2:
	
	#define ADC_CHANNEL_5_POT
	
	#define NUMBER_OF_AT_POTS 1
	
	
	Example 3:
	
	#define ADC_CHANNEL_0_MUX
	#define ADC_CHANNEL_5_POT
	
	#define NUMBER_OF_AT_POTS 1
	#define NUMBER_OF_MUX 1


*/


//if uncommented, each pot will send 6 additional note events,
//depending on its position
#define ENABLE_POT_NOTE_EVENTS

//uncomment if pot gives you 127 on very left and 0 on right position
#define INVERT_ANALOGUE_VALUE

#ifdef LED_MATRIX
//define LED numbers
#define LED_1
#define LED_2
#define LED_3
#define LED_4

#define TOTAL_NUMBER_OF_LEDS

//all LED numbers need to be inside ledArray
const uint8_t ledArray[TOTAL_NUMBER_OF_LEDS] = {	LED_1, LED_2, LED_3, LED_4	};

//blink duration in milliseconds
#define BLINK_DURATION

//speed at which the LEDs are turning on one by one at startup (in ms)
#define START_UP_LED_SWITCH_TIME

#define NUMBER_OF_LED_ROWS
#define MAX_NUMBER_OF_LEDS NUMBER_OF_COLUMNS*NUMBER_OF_LED_ROWS

#endif

#ifdef BUTTON_MATRIX

#define NUMBER_OF_BUTTON_ROWS
#define BUTTON_DEBOUNCE_TIME
#define LONG_PRESS_TIME
#define MAX_NUMBER_OF_BUTTONS NUMBER_OF_COLUMNS*NUMBER_OF_BUTTON_ROWS

#endif

////////////////////////////////////////////////////////////////////////end config

class HardwareReadSpecific  {

    public:

        //constructor
        HardwareReadSpecific();
        
        //LEDs
        #ifdef LED_MATRIX
			static void ledRowOn(uint8_t rowNummber);
			static void ledRowOff(uint8_t rowNummber);
			static void ledRowsOn();
			static void ledRowsOff();
		#endif

		static void activateColumn(uint8_t column);
		
		#ifdef BUTTON_MATRIX
			static uint8_t readButtons();
		#endif
		
		#ifdef NUMBER_OF_MUX
		static void setMuxOutput(uint8_t muxInput);
		#endif

		static void initPins();

};

#endif