/*

OpenDECK library v1.90
Last revision date: 2014-07-30
Author: Igor Petrovic

*/

#ifndef OpenDeck_h
#define OpenDeck_h

#include <avr/io.h>
#include <avr/pgmspace.h>

#define BUTTON_DEBOUNCE_TIME 20

//potentiometer must exceed this value before sending new value
#define MIDI_CC_STEP 8

//potentiometer must exceed this value if it hasn't been moved for more than POTENTIOMETER_MOVE_TIMEOUT
#define MIDI_CC_STEP_TIMEOUT 9

//time in ms after which new value from pot must exceed MIDI_CC_STEP_TIMEOUT
#define POTENTIOMETER_MOVE_TIMEOUT 200

#define MAX_NUMBER_OF_POTS 32
#define MAX_NUMBER_OF_BUTTONS 64
#define MAX_NUMBER_OF_LEDS 64
  
//default controller settings
const uint8_t defConf[] PROGMEM = {
			
			//channels
			0x01, //button note channel 1			//000
			0x02, //long press note channel 2		//001
			0x01, //CC channel 1					//002
			0x02, //encoder channel 2				//003
			0x01, //MIDI input channel	1			//004
			
			//hardware parameters		
			0x05, //long press time (x100)			//005
			0x04, //blink duration time (x100)		//006
			0x07, //start up LED switch time (x10)	//007
			
			//features
			//bit meaning, MSB first (1/enabled, 0/disabled)
			
			//software
			
			//no function
			//start-up routine
			//LED blinking
			//long press
			//pot notes
			//encoder notes
			//standard note off
			//running status
			
			0b01111111,								//008
			
			//hardware
			
			//no function
			//no function
			//no function
			//no function
			//buttons
			//LEDs
			//pots
			//encoders
			0b00001111,								//009
			
			//invert CC data (1 enabled, 0 disabled)
			
			//7-0
			0x00,									//010
			//15-8
			0x00,									//011
			//23-16
			0x00,									//012
			//31-24
			0x00,									//013
			//39-32
			0x00,									//014
			//47-40
			0x00,									//015
			//55-48
			0x00,									//016
			//63-56
			0x00,									//017
			//71-64
			0x00,									//018
			//79-72
			0x00,									//019
			//87-80
			0x00,									//020
			//95-88
			0x00,									//021
			//103-96
			0x00,									//022
			//111-104
			0x00,									//023
			//119-112
			0x00,									//024
			//127-120
			0x00,									//025
			
			//enable pot (1 enabled, 0 disabled)
						
			//7-0
			0x00,									//026
			//15-8
			0x00,									//027
			//23-16
			0x00,									//028
			//31-24
			0x00,									//029
			//39-32
			0x00,									//030
			//47-40
			0x00,									//031
			//55-48
			0x00,									//032
			//63-56
			0x00,									//033
			//71-64
			0x00,									//034
			//79-72
			0x00,									//035
			//87-80
			0x00,									//036
			//95-88
			0x00,									//037
			//103-96
			0x00,									//038
			//111-104
			0x00,									//039
			//119-112
			0x00,									//040
			//127-120
			0x00,									//041
			
			//potentiometer numbers
			
			0x00,									//042
			0x01,									//043
			0x02,									//044
			0x03,									//045
			0x04,									//046
			0x05,									//047
			0x06,									//048
			0x07,									//049
			0x08,									//050
			0x09,									//051
			0x0A,									//052
			0x0B,									//053
			0x0C,									//054
			0x0D,									//055
			0x0E,									//056
			0x0F,									//057
			0x10,									//058
			0x11,									//059
			0x12,									//060
			0x13,									//061
			0x14,									//062
			0x15,									//063
			0x16,									//064
			0x17,									//065
			0x18,									//066
			0x19,									//067
			0x1A,									//068
			0x1B,									//069
			0x1C,									//070
			0x1D,									//071
			0x1E,									//072
			0x1F,									//073
			0x20,									//074
			0x21,									//075
			0x22,									//076
			0x23,									//077
			0x24,									//078
			0x25,									//079
			0x26,									//080
			0x27,									//081
			0x28,									//082
			0x29,									//083
			0x2A,									//084
			0x2B,									//085
			0x2C,									//086
			0x2D,									//087
			0x2E,									//088
			0x2F,									//089
			0x30,									//090
			0x31,									//091
			0x32,									//092
			0x33,									//093
			0x34,									//094
			0x35,									//095
			0x36,									//096
			0x37,									//097
			0x38,									//098
			0x39,									//099
			0x3A,									//100
			0x3B,									//101
			0x3C,									//102
			0x3D,									//103
			0x3E,									//104
			0x3F,									//105
			0x40,									//106
			0x41,									//107
			0x42,									//108
			0x43,									//109
			0x44,									//110
			0x45,									//111
			0x46,									//112
			0x47,									//113
			0x48,									//114
			0x49,									//115
			0x4A,									//116
			0x4B,									//117
			0x4C,									//118
			0x4D,									//119
			0x4E,									//120
			0x4F,									//121
			0x50,									//122
			0x51,									//123
			0x52,									//124
			0x53,									//125
			0x54,									//126
			0x55,									//127
			0x56,									//128
			0x57,									//129
			0x58,									//130
			0x59,									//131
			0x5A,									//132
			0x5B,									//133
			0x5C,									//134
			0x5D,									//135
			0x5E,									//136
			0x5F,									//137
			0x60,									//138
			0x61,									//139
			0x62,									//140
			0x63,									//141
			0x64,									//142
			0x65,									//143
			0x66,									//144
			0x67,									//145
			0x68,									//146
			0x69,									//147
			0x6A,									//148
			0x6B,									//149
			0x6C,									//150
			0x6D,									//151
			0x6E,									//152
			0x6F,									//153
			0x70,									//154
			0x71,									//155
			0x72,									//156
			0x73,									//157
			0x74,									//158
			0x75,									//159
			0x76,									//160
			0x77,									//161
			0x78,									//162
			0x79,									//163
			0x7A,									//164
			0x7B,									//165
			0x7C,									//166
			0x7D,									//167
			0x7E,									//168
			0x7F,									//169
			
			//button numbers
			
			0x00,									//170
			0x01,
			0x02,
			0x03,
			0x04,
			0x05,
			0x06,
			0x07,
			0x08,
			0x09,
			0x0A,
			0x0B,
			0x0C,
			0x0D,
			0x0E,
			0x0F,
			0x10,
			0x11,
			0x12,
			0x13,
			0x14,
			0x15,
			0x16,
			0x17,
			0x18,
			0x19,
			0x1A,
			0x1B,
			0x1C,
			0x1D,
			0x1E,
			0x1F,
			0x20,
			0x21,
			0x22,
			0x23,
			0x24,
			0x25,
			0x26,
			0x27,
			0x28,
			0x29,
			0x2A,
			0x2B,
			0x2C,
			0x2D,
			0x2E,
			0x2F,
			0x30,
			0x31,
			0x32,
			0x33,
			0x34,
			0x35,
			0x36,
			0x37,
			0x38,
			0x39,
			0x3A,
			0x3B,
			0x3C,
			0x3D,
			0x3E,
			0x3F,
			0x40,
			0x41,
			0x42,
			0x43,
			0x44,
			0x45,
			0x46,
			0x47,
			0x48,
			0x49,
			0x4A,
			0x4B,
			0x4C,
			0x4D,
			0x4E,
			0x4F,
			0x50,
			0x51,
			0x52,
			0x53,
			0x54,
			0x55,
			0x56,
			0x57,
			0x58,
			0x59,
			0x5A,
			0x5B,
			0x5C,
			0x5D,
			0x5E,
			0x5F,
			0x60,
			0x61,
			0x62,
			0x63,
			0x64,
			0x65,
			0x66,
			0x67,
			0x68,
			0x69,
			0x6A,
			0x6B,
			0x6C,
			0x6D,
			0x6E,
			0x6F,
			0x70,
			0x71,
			0x72,
			0x73,
			0x74,
			0x75,
			0x76,
			0x77,
			0x78,
			0x79,
			0x7A,
			0x7B,
			0x7C,
			0x7D,
			0x7E,
			0x7F,
			
			//button type 
			//0 - press/note on, release/note off
			//1 - press/note on, release/nothing, press again/note off
			
			//7-0										
			0x00,									//298
			//15-8
			0x00,
			//23-16
			0x00,
			//31-24
			0x00,
			//39-32
			0x00,
			//47-40
			0x00,
			//55-48
			0x00,
			//63-56
			0x00,
			//71-64
			0x00,
			//79-72
			0x00,
			//87-80
			0x00,
			//95-88
			0x00,
			//103-96
			0x00,
			//111-104
			0x00,
			//119-112
			0x00,
			//127-120
			0x00,						
			
			//LED notes
						
			0x00,								//314
			0x01,
			0x03,
			0x03,
			0x04,
			0x05,
			0x06,
			0x07,
			0x08,
			0x09,
			0x0A,
			0x0B,
			0x0C,
			0x0D,
			0x0E,
			0x0F,
			0x10,
			0x11,
			0x12,
			0x13,
			0x14,
			0x15,
			0x16,
			0x17,
			0x18,
			0x19,
			0x1A,
			0x1B,
			0x1C,
			0x1D,
			0x1E,
			0x1F,
			0x20,
			0x21,
			0x22,
			0x23,
			0x24,
			0x25,
			0x26,
			0x27,
			0x28,
			0x29,
			0x2A,
			0x2B,
			0x2C,
			0x2D,
			0x2E,
			0x2F,
			0x30,
			0x31,
			0x32,
			0x33,
			0x34,
			0x35,
			0x36,
			0x37,
			0x38,
			0x39,
			0x3A,
			0x3B,
			0x3C,
			0x3D,
			0x3E,
			0x3F,
			0x40,
			0x41,
			0x42,
			0x43,
			0x44,
			0x45,
			0x46,
			0x47,
			0x48,
			0x49,
			0x4A,
			0x4B,
			0x4C,
			0x4D,
			0x4E,
			0x4F,
			0x50,
			0x51,
			0x52,
			0x53,
			0x54,
			0x55,
			0x56,
			0x57,
			0x58,
			0x59,
			0x5A,
			0x5B,
			0x5C,
			0x5D,
			0x5E,
			0x5F,
			0x60,
			0x61,
			0x62,
			0x63,
			0x64,
			0x65,
			0x66,
			0x67,
			0x68,
			0x69,
			0x6A,
			0x6B,
			0x6C,
			0x6D,
			0x6E,
			0x6F,
			0x70,
			0x71,
			0x72,
			0x73,
			0x74,
			0x75,
			0x76,
			0x77,
			0x78,
			0x79,
			0x7A,
			0x7B,
			0x7C,
			0x7D,
			0x7E,
			0x7F,
			
			//total number of LEDs				//442
			0x00
					
		};

class OpenDeck  {

    public:

        //constructor
        OpenDeck();
		
		//library initializer
		void init();
		
		//restores default controller functionality
		void setDefaultConf();
		
		//hardware configuration
		void setHandlePinInit(void (*fptr)());
		void setHandleColumnSwitch(void (*fptr)(uint8_t columnNumber));
		void setNumberOfColumns(uint8_t numberOfColumns);
		void setNumberOfButtonRows(uint8_t numberOfButtonRows);
		void setNumberOfLEDrows(uint8_t numberOfLEDrows);
		void setNumberOfMux(uint8_t numberOfMux);
		void enableAnalogueInput(uint8_t adcChannel);
		
        //buttons
		void readButtons();
		void setHandleButtonRead(void (*fptr)(uint8_t &buttonColumnState));
		void setHandleButtonSend(void (*fptr)(uint8_t buttonNumber, bool buttonState, uint8_t channel));
        
		//pot
		void readPots();
		void setHandleMuxOutput(void (*fptr)(uint8_t muxInput));
		void setHandlePotCC(void (*fptr)(uint8_t potNumber, uint8_t ccValue, uint8_t channel));
		void setHandlePotNoteOn(void (*fptr)(uint8_t note, uint8_t potNumber, uint8_t channel));
		void setHandlePotNoteOff(void (*fptr)(uint8_t note, uint8_t potNumber, uint8_t channel));
	
        //LEDs
		void oneByOneLED(bool ledDirection, bool singleLED, bool turnOn);
		void checkLEDs();
		void allLEDsOn();
		void allLEDsOff();
		void turnOnLED(uint8_t ledNumber);
		void turnOffLED(uint8_t ledNumber);
		void storeReceivedNote(uint8_t channel, uint8_t pitch, uint8_t velocity);
		void checkReceivedNote();
		void setHandleLEDrowOn(void (*fptr)(uint8_t ledRow));
		void setHandleLEDrowsOff(void (*fptr)());
		
		//columns
		void nextColumn();
		
		//getters
		
		bool buttonsEnabled();
		bool ledsEnabled();
		bool potsEnabled();
		bool startUpRoutineEnabled();
		bool standardNoteOffEnabled();
		
		uint8_t getInputChannel();	
					
    private:
				
		//variables
		//MIDI channels
		uint8_t buttonNoteChannel,
				longPressButtonNoteChannel,
				ccChanelPot,
				ccChannelEnc,
				inputChannel;
		
        //hardware params
		uint16_t longPressTime,
				blinkTime,
				startUpLEDswitchTime;
				
		//software features
		bool _startUpRoutineEnabled,
			_ledBlinkEnabled,
			_longPressEnabled,
			_potNotesEnabled,
			_encoderNotesEnabled,
			_standardNoteOffEnabled,
			_runningStatusEnabled;
		
		//hardware features
		bool _buttonsEnabled,
			_ledsEnabled,
			_potsEnabled,
			_encodersEnabled;

		//buttons
		uint8_t buttonNote[MAX_NUMBER_OF_BUTTONS],
				previousButtonState[MAX_NUMBER_OF_BUTTONS],
				buttonDebounceCompare;
				
		bool buttonType[MAX_NUMBER_OF_BUTTONS],
			buttonPressed[MAX_NUMBER_OF_BUTTONS],
			longPressSent[MAX_NUMBER_OF_BUTTONS];
		
		uint32_t longPressState[MAX_NUMBER_OF_BUTTONS];
		
		//pots
		bool potInverted[MAX_NUMBER_OF_POTS],
			potEnabled[MAX_NUMBER_OF_POTS];
		
		uint8_t ccNumber[MAX_NUMBER_OF_POTS],
				lastPotNoteValue[MAX_NUMBER_OF_POTS],
				potNumber;
						
		uint16_t lastAnalogueValue[MAX_NUMBER_OF_POTS];
		uint32_t potTimer[MAX_NUMBER_OF_POTS];
				
		//LEDs
		uint8_t ledState[MAX_NUMBER_OF_LEDS],
				_ledNumber[MAX_NUMBER_OF_LEDS],
				totalNumberOfLEDs;
		
		bool blinkState,
			blinkEnabled;
			
		uint32_t blinkTimerCounter;
		
		//input
		bool receivedNoteProcessed;
		
		uint8_t receivedNoteChannel,
				receivedNotePitch,
				receivedNoteVelocity;
				
		//hardware
		uint8_t column,
				_numberOfColumns,
				_numberOfButtonRows,
				_numberOfLEDrows,
				_numberOfMux;
		
		bool _analogueIn[8];
				
		//general
		uint8_t i;

        
		//functions
		
		//init
		void initVariables();
		void (*sendInitPinsCallback)();
		
        //read configuration
		void getConfiguration();
		void getMIDIchannels();
		void getHardwareParams();
		void getSoftwareFeatures();
		void getHardwareFeatures();
		void getPotInvertStates();
		void getEnabledPots();
		void getCCnumbers();
		void getButtonNumbers();
		void getButtonType();
		void getLEDnumbers();
		void getTotalLEDnumber();
		
		//buttons
		uint8_t checkButton(uint8_t currentState, uint8_t previousState);
		void setNumberOfColumnPasses();
		void setButtonDebounceCompare(uint8_t numberOfColumnPasses);
		void (*sendButtonReadCallback)(uint8_t &buttonColumnState);
		void (*sendButtonDataCallback)(uint8_t buttonNumber, bool buttonState, uint8_t channel);
				
		//LEDs
		void handleLED(uint8_t ledNote, bool currentLEDstate, bool blinkMode);
		void setLEDState();
		void checkBlinkLEDs();
		bool checkBlinkState(uint8_t ledNumber);
		void setBlinkState(uint8_t ledNumber, bool blinkState);
		void switchBlinkState();
		void setConstantLEDstate(uint8_t ledNumber);
		bool ledOn(uint8_t ledNumber);
		bool checkLEDsOn();
		bool checkLEDsOff();
		void (*sendLEDrowOnCallback)(uint8_t ledRow);
		void (*sendLEDrowsOffCallback)();
	
		//pots
		bool adcConnected(uint8_t adcChannel);
		bool adcChannelMux(uint8_t adcChannel);
		void readPotsMux(uint8_t adcChannel);
		void processPotReading(uint8_t potNumber, int16_t tempValue);
		uint8_t getPotNoteValue(uint8_t analogueMIDIvalue, uint8_t potNumber);
		void checkPotReading(int16_t currentValue, uint8_t potNumber);
		bool checkPotNoteValue(uint8_t potNumber, uint8_t ccValue);
		void (*sendSwitchMuxOutCallback)(uint8_t muxInput);
		void (*sendPotCCDataCallback)(uint8_t potNumber, uint8_t ccValue, uint8_t channel);
		void (*sendPotNoteOnDataCallback)(uint8_t note, uint8_t potNumber, uint8_t channel);
		void (*sendPotNoteOffDataCallback)(uint8_t note, uint8_t potNumber, uint8_t channel);		
			
		//columns
		uint8_t getActiveColumn();
		void (*sendColumnSwitchCallback)(uint8_t columnNumber);
			
};

extern OpenDeck openDeck;

#endif