/*

OpenDECK library v1.0
Last revision date: 2014-05-23
Author: Igor Petrovic

*/

#ifndef OpenDeck_h
#define OpenDeck_h

#include <avr/io.h>
#include "HardwareReadSpecific.h"

//potentiometer must exceed this value before sending new value
#define MIDI_CC_STEP 8

//potentiometer must exceed this value if it hasn't been moved for more than POTENTIOMETER_MOVE_TIMEOUT
#define MIDI_CC_STEP_TIMEOUT 8

//time in ms after which new value from pot must exceed MIDI_CC_STEP_TIMEOUT
#define POTENTIOMETER_MOVE_TIMEOUT 200

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

#ifdef BUTTON_MATRIX
#define MAX_NUMBER_OF_BUTTONS NUMBER_OF_COLUMNS*NUMBER_OF_BUTTON_ROWS
#endif

#ifdef LED_MATRIX
#define MAX_NUMBER_OF_LEDS NUMBER_OF_COLUMNS*NUMBER_OF_LED_ROWS
#endif

class OpenDeck  {

    public:

        //constructor
        OpenDeck();
		void init();

        //buttons
		#ifdef BUTTON_MATRIX
			void setHandleButton(void (*fptr)(uint8_t buttonNumber, uint8_t buttonState));
			void readButtons();
		#endif
        
		//pots
		#ifdef POTS
			void readPots();
			void setHandlePotCC(void (*fptr)(uint8_t potNumber, uint8_t ccValue));
			void setHandlePotNoteOn(void (*fptr)(uint8_t note));
			void setHandlePotNoteOff(void (*fptr)(uint8_t note));
		#endif

        //LEDs
        #ifdef LED_MATRIX
			void oneByOneLED(bool ledDirection, bool singleLED, bool turnOn);
			void checkLEDs();
			void allLEDsOn();
			void allLEDsOff();
			void storeReceivedNote(uint8_t channel, uint8_t pitch, uint8_t velocity);
			void checkReceivedNote();
			void turnOnLED(uint8_t ledNumber);
			void turnOffLED(uint8_t ledNumber);
		#endif
		
		//columns
		void activateColumn();
		void nextColumn();
				
    private:

        //variables
        bool receivedNoteProcessed;
			
		#ifdef LED_MATRIX
		bool blinkState,
			blinkEnabled;
		#endif
			
        #ifdef POTS
			uint8_t lastPotNoteValue[TOTAL_NUMBER_OF_POTS],
					potNumber;
			
			#ifdef MUX
				uint8_t muxInput;
			#endif
        #endif
		
		uint8_t i,
                column,
				receivedNoteChannel,
				receivedNotePitch,
				receivedNoteVelocity;
				
		#ifdef LED_MATRIX
			uint8_t ledState[MAX_NUMBER_OF_LEDS];
		#endif
				
		#ifdef BUTTON_MATRIX
		
		#ifdef LONG_PRESS_TIME
			bool longPressSent[MAX_NUMBER_OF_BUTTONS];
		#endif
		
		uint8_t previousButtonState[MAX_NUMBER_OF_BUTTONS],
				buttonDebounceCompare,
				numberOfColumnPasses;
				
		#ifdef LONG_PRESS_TIME
			uint32_t longPressState[MAX_NUMBER_OF_BUTTONS];
		#endif
		
		#endif

        #ifdef POTS
			uint16_t lastAnalogueValue[TOTAL_NUMBER_OF_POTS];
        #endif
		
		#ifdef POTS
			uint32_t potTimer[TOTAL_NUMBER_OF_POTS];
		#endif
		
		#ifdef LED_MATRIX
			uint32_t blinkTimerCounter;
		#endif

        //functions
		
        //init
        void initVariables();
		
		
		//LEDs
		#ifdef LED_MATRIX
			bool checkBlinkState(uint8_t ledNumber);
			void switchBlinkState();
			void checkBlinkLEDs();
			void setBlinkState(uint8_t ledNumber, bool blinkState);
			void setConstantLEDstate(uint8_t ledNumber);
			bool ledOn(uint8_t ledNumber);
			void handleLED(uint8_t ledNumber, bool currentLEDstate, bool blinkMode);
			bool checkLEDsOn();
			bool checkLEDsOff();
			void setLEDState();
		#endif
		
		
		//buttons
		#ifdef BUTTON_MATRIX
			void (*sendButtonDataCallback)(uint8_t buttonNumber, uint8_t buttonStatus);
			uint8_t checkButton(uint8_t currentState, uint8_t previousState);
			void setNumberOfColumnPasses();
			void setButtonDebounceCompare();
		#endif
		
		
		//pots
		#ifdef POTS
			void (*sendPotCCDataCallback)(uint8_t potNumber, uint8_t ccValue);
			void (*sendPotNoteOnDataCallback)(uint8_t note);
			void (*sendPotNoteOffDataCallback)(uint8_t note);	
			uint8_t getPotNoteValue(uint8_t analogueMIDIvalue, uint8_t potNumber);
			bool checkPotNoteValue(uint8_t potNumber, uint8_t ccValue);
			void processPotReading(uint8_t potNumber, int16_t tempValue);
			bool adcConnected(uint8_t adcChannel);
			void checkPotReading(int16_t currentValue, uint8_t potNumber);
			
			#ifdef MUX
				void readPotsMux(uint8_t adcChannel);
				bool adcChannelMux(uint8_t adcChannel);
			#endif
			
			#ifdef AT_POTS
				void readPotsATmega(uint8_t adcChannel);
			#endif
		#endif
		
		//columns
		uint8_t getActiveColumn();
			
};

extern OpenDeck openDeck;

#endif