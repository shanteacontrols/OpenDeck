/*

Hardware specific library for reading data from multiple sources
v1.0
Last revision date: 2014-05-17
Author: Igor Petrovic

*/

//Please note that this library works correctly only in matrix setups (buttons/LEDs)

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

#define NUMBER_OF_COLUMNS 5

#define NUMBER_OF_MUX 2

/*

	ADC pins defining guide:
	
	#define ADC_CHANNEL_NUMBER_TYPE
	
	Where number is ATmega328 analog input (0-7) and type is
	either MUX or POT.
	
	So, if there is only single pot connected to ATmega on channel 2, you only need
	to define that single pin:
	
	#define ADC_CHANNEL_2_POT


*/



#define ADC_CHANNEL_0_MUX
#define ADC_CHANNEL_1_MUX

//if uncommented, each pot will send 6 additional note events,
//depending on its position
#define ENABLE_POT_NOTE_EVENTS

//uncomment if pot gives you 127 on very left and 0 on right position
#define INVERT_ANALOGUE_VALUE

#ifdef LED_MATRIX
//define LED numbers
#define LED_1 3
#define LED_2 2
#define LED_3 1
#define LED_4 0

#define TOTAL_NUMBER_OF_LEDS 4

//all LED numbers need to be inside ledArray
const uint8_t ledArray[TOTAL_NUMBER_OF_LEDS] = {	LED_1, LED_2, LED_3, LED_4	};

//blink duration in milliseconds
#define BLINK_DURATION 400

//speed at which the LEDs are turning on one by one at startup (in ms)
#define START_UP_LED_SWITCH_TIME 100

#define NUMBER_OF_LED_ROWS 1
#define MAX_NUMBER_OF_LEDS NUMBER_OF_COLUMNS*NUMBER_OF_LED_ROWS

#endif

#ifdef BUTTON_MATRIX

#define NUMBER_OF_BUTTON_ROWS 4
#define BUTTON_DEBOUNCE_TIME 15
#define LONG_PRESS_TIME 500
#define MAX_NUMBER_OF_BUTTONS NUMBER_OF_COLUMNS*NUMBER_OF_BUTTON_ROWS

#endif

////////////////////////////////////////////////////////////////////////end config

#ifdef NUMBER_OF_AT_POTS
#define AT_POTS
#endif

#ifdef NUMBER_OF_MUX
#define MUX
//number of pots connected to 4051
#define NUMBER_OF_MUX_POTS 8*NUMBER_OF_MUX
#endif

#if defined (MUX) || defined (AT_POTS)
#define POTS
#endif

#ifdef POTS

//define number of components
#if defined(MUX) && defined(AT_POTS)
#define TOTAL_NUMBER_OF_POTS NUMBER_OF_AT_POTS+NUMBER_OF_MUX_POTS
#elif defined(MUX)
#define TOTAL_NUMBER_OF_POTS NUMBER_OF_MUX_POTS
#else
#define TOTAL_NUMBER_OF_POTS NUMBER_OF_AT_POTS
#endif

#endif

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
		
		#ifdef POTS
	
		static bool adcConnected(uint8_t adcChannel);
		
		#ifdef MUX
		static void setMuxOutput(uint8_t muxInput);
		static bool adcChannelMux(uint8_t adcChannel);
		#endif
		
		#endif
		
		static void initPins();
			    
};

#endif
