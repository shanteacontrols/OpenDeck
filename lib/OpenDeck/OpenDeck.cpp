/*

OpenDECK library v1.92
Last revision date: 2014-08-10
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include "Ownduino.h"
#include <avr/io.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <util/crc16.h>
#include <avr/sfr_defs.h>

#define BUTTON_NOTE_CHANNEL_ADD				0
#define LONG_PRESS_BUTTON_NOTE_CHANNEL_ADD	1
#define POT_CC_CHANNEL_ADD					2
#define ENC_CC_CHANNEL_ADD					3
#define INPUT_CHANNEL_ADD					4

#define LONG_PRESS_TIME_ADD					5
#define BLINK_TIME_ADD						6
#define START_UP_SWITCH_TIME_ADD			7

#define SOFTWARE_FEATURES_ADD				8

#define START_UP_ROUTINE_EN_BIT				6
#define LED_BLINK_EN_BIT					5
#define LONG_PRESS_EN_BIT					4
#define POT_NOTES_EN_BIT					3
#define ENC_NOTES_EN_BIT					2
#define STANDARD_NOTE_OFF_EN_BIT			1
#define RUNNING_STATUS_EN_BIT				0

#define HARDWARE_FEATURES_ADD				9

#define BUTTONS_EN_BIT						3
#define LEDS_EN_BIT							2
#define POTS_EN_BIT							1
#define ENC_EN_BIT							0

#define POT_INVERSION_STATUS_ADD			10
#define POT_ENABLED_STATUS_ADD				26
#define POT_CC_NUMBER_ADD					42

#define BUTTON_NOTE_NUMBER_ADD				170
#define BUTTON_TYPE_ADD						298

#define LED_ID_ADD							314
#define TOTAL_LED_NUMBER_ADD				442

OpenDeck::OpenDeck()    {

  //initialization
  initVariables();
  
  //set all callbacks to NULL pointer
  
  sendButtonDataCallback		=	NULL;
  sendLEDrowOnCallback			=	NULL;
  sendLEDrowsOffCallback		=	NULL;
  sendButtonReadCallback		=	NULL;
  sendSwitchMuxOutCallback		=	NULL;
  sendInitPinsCallback			=	NULL;
  sendColumnSwitchCallback		=	NULL;
  sendPotCCDataCallback			=	NULL;
  sendPotNoteOnDataCallback		=	NULL;
  sendPotNoteOffDataCallback	=	NULL;
      
}


//init

void OpenDeck::setHandlePinInit(void (*fptr)())	{
	
	sendInitPinsCallback = fptr;
	
}

void OpenDeck::initVariables()  {

	//reset all variables
	
	//MIDI channels
	_buttonNoteChannel			= 0;
	_longPressButtonNoteChannel	= 0;
	_potCCchannel				= 0;
	_encCCchannel				= 0;
	_inputChannel				= 0;
	
	//hardware params
	_longPressTime				= 0;
	_blinkTime					= 0;
	_startUpLEDswitchTime		= 0;
	
	//software features
	softwareFeatures			= 0;
	
	//hardware features
	hardwareFeatures			= 0; 
	
	//buttons	 	  
	for (i=0; i<MAX_NUMBER_OF_BUTTONS; i++)  {
			
	buttonNote[i]				= 0;
	previousButtonState[i]		= 0;
	longPressState[i]			= 0;
	
	}
	
	for (i=0; i<MAX_NUMBER_OF_BUTTONS/8; i++)  {
		
	buttonType[i]				= 0;
	buttonPressed[i]			= 0;
	longPressSent[i]			= 0;
	
			
	}
		
	buttonDebounceCompare		= 0;
	  
	//pots
	for (i=0; i<MAX_NUMBER_OF_POTS; i++)  {

	ccNumber[i]					= 0;
	lastPotNoteValue[i]			= 0;
	lastAnalogueValue[i]		= 0;
	potTimer[i]					= 0;

	}
	
	for (i=0; i<MAX_NUMBER_OF_POTS/8; i++)  {
		
	potInverted[i]				= 0;
	potEnabled[i]				= 0;
		
	}
	
	_analogueIn					= 0;
	  
	potNumber					= 0;

	//LEDs
	for (i=0; i<MAX_NUMBER_OF_LEDS; i++)  {
		
		ledState[i]				= 0;
		ledID[i]			= 0;
		
	}
	 
	totalNumberOfLEDs			= 0;

	blinkState					= false;
	blinkEnabled				= false;
	blinkTimerCounter			= 0;
	  
	//input
	receivedNoteProcessed		= false;
	receivedNoteChannel			= 0;
	receivedNotePitch			= 0;
	receivedNoteVelocity		= 0;
	
	//column counter
	column						= 0;
	  
}

void OpenDeck::init()	{
	
	sendInitPinsCallback();
	
	setNumberOfColumnPasses();
	
	for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)		previousButtonState[i] = buttonDebounceCompare;
	
	//initialize lastPotNoteValue to 128, which is impossible value for MIDI,
	//to avoid sending note off for that value on first read
	for (int i=0; i<MAX_NUMBER_OF_POTS; i++)		lastPotNoteValue[i] = 128;
	
	blinkState				= true;
	receivedNoteProcessed	= true;

	//make initial pot reading to avoid sending all data on startup
	readPots();
	
	//get all values from EEPROM
	getConfiguration();
	
}


//configuration retrieve

//global configuration getter

void OpenDeck::getConfiguration()	{
	
	//get configuration from EEPROM
	getMIDIchannels();
	getHardwareParams();
	getSoftwareFeatures();
	getHardwareFeatures();
	getEnabledPots();
	getPotInvertStates();
	getCCnumbers();
	getButtonsType();
	getButtonNotes();
	getLEDIDs();
	getTotalLEDnumber();
	
}


//individual configuration getters

void OpenDeck::getMIDIchannels()	{
	
	_buttonNoteChannel			= eeprom_read_byte((uint8_t*)BUTTON_NOTE_CHANNEL_ADD);
	_longPressButtonNoteChannel	= eeprom_read_byte((uint8_t*)LONG_PRESS_BUTTON_NOTE_CHANNEL_ADD);
	_potCCchannel				= eeprom_read_byte((uint8_t*)POT_CC_CHANNEL_ADD);
	_encCCchannel				= eeprom_read_byte((uint8_t*)ENC_CC_CHANNEL_ADD);
	_inputChannel				= eeprom_read_byte((uint8_t*)INPUT_CHANNEL_ADD);
	
}

void OpenDeck::getHardwareParams()	{
	
	_longPressTime				= eeprom_read_byte((uint8_t*)LONG_PRESS_TIME_ADD) * 100;
	_blinkTime					= eeprom_read_byte((uint8_t*)BLINK_TIME_ADD) * 100;
	_startUpLEDswitchTime		= eeprom_read_byte((uint8_t*)START_UP_SWITCH_TIME_ADD) * 10;
	
}

void OpenDeck::getSoftwareFeatures()	{
	
	softwareFeatures = eeprom_read_byte((uint8_t*)SOFTWARE_FEATURES_ADD);
	
}

void OpenDeck::getHardwareFeatures()	{
	
	hardwareFeatures = eeprom_read_byte((uint8_t*)HARDWARE_FEATURES_ADD);
	
}

void OpenDeck::getEnabledPots()	{

	uint16_t eepromAddress = POT_ENABLED_STATUS_ADD;
	
	for (int i=0; i<(MAX_NUMBER_OF_POTS/8); i++)	{
		
		potEnabled[i] = eeprom_read_byte((uint8_t*)eepromAddress);
		eepromAddress++;
		
	}
	
}

void OpenDeck::getPotInvertStates()	{
	
	uint16_t eepromAddress = POT_INVERSION_STATUS_ADD;
	
	for (int i=0; i<(MAX_NUMBER_OF_POTS/8); i++)	{
		
		potInverted[i] = eeprom_read_byte((uint8_t*)eepromAddress);
		eepromAddress++;

	}
	
}

void OpenDeck::getCCnumbers()	{
	
	uint16_t eepromAddress = POT_CC_NUMBER_ADD;
	
	for (int i=0; i<MAX_NUMBER_OF_POTS; i++)	{
		
		ccNumber[i] = eeprom_read_byte((uint8_t*)eepromAddress);
		eepromAddress++;
		
	}
	
}

void OpenDeck::getButtonsType()	{
	
	uint16_t eepromAddress = BUTTON_TYPE_ADD;
	
	for (int i=0; i<MAX_NUMBER_OF_BUTTONS/8; i++)	{
		
		buttonType[i] = eeprom_read_byte((uint8_t*)eepromAddress);
		eepromAddress++;
		
	}
	
}

void OpenDeck::getButtonNotes()	{
	
	uint16_t eepromAddress = BUTTON_NOTE_NUMBER_ADD;
	
	for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)	{
		
		buttonNote[i] = eeprom_read_byte((uint8_t*)eepromAddress);
		eepromAddress++;
		
	}
	
}

void OpenDeck::getLEDIDs()	{
	
	uint16_t eepromAddress = LED_ID_ADD;
	
	for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)	{
		
		ledID[i] = eeprom_read_byte((uint8_t*)eepromAddress);
		eepromAddress++;
		
	}
	
}

void OpenDeck::getTotalLEDnumber()	{
	
	totalNumberOfLEDs = eeprom_read_byte((uint8_t*)TOTAL_LED_NUMBER_ADD);
	
}
  
  
bool OpenDeck::standardNoteOffEnabled()	{
	
	return getFeature(SOFTWARE_FEATURES_ADD, STANDARD_NOTE_OFF_EN_BIT); 
	 
}

bool OpenDeck::buttonsEnabled()	{
	
	return getFeature(HARDWARE_FEATURES_ADD, BUTTONS_EN_BIT);

}

bool OpenDeck::ledsEnabled()	{
	
	return getFeature(HARDWARE_FEATURES_ADD, LEDS_EN_BIT);

}

bool OpenDeck::potsEnabled()	{
	
	bool returnValue = getFeature(HARDWARE_FEATURES_ADD, POTS_EN_BIT);
	return returnValue;

}
  
//buttons

void OpenDeck::setNumberOfColumnPasses() {
	
	/*
		
		Algorithm calculates how many times does it need to read whole row
		before it can declare button reading stable.
	
	*/
	  
	uint8_t rowPassTime = getTimedLoopTime()*_numberOfColumns;
	uint8_t mod = 0;
	  
	if ((BUTTON_DEBOUNCE_TIME % rowPassTime) > 0)	mod = 1;

	uint8_t numberOfColumnPasses = ((BUTTON_DEBOUNCE_TIME / rowPassTime) + mod);
	
	setButtonDebounceCompare(numberOfColumnPasses);
	
}
 
void OpenDeck::setButtonDebounceCompare(uint8_t numberOfColumnPasses)	{
	
	//depending on numberOfColumnPasses, button state gets shifted into
	//different buttonDebounceCompare variable
	  
	  switch(numberOfColumnPasses)	{
		  
		  case 1:
		  buttonDebounceCompare = 0b11111110;
		  break;
		  
		  case 2:
		  buttonDebounceCompare = 0b11111100;
		  break;
		  
		  case 3:
		  buttonDebounceCompare = 0b11111000;
		  break;
		  
		  case 4:
		  buttonDebounceCompare = 0b11110000;
		  break;
		  
		  default:
		  break;
		  
	  }
	  
}
 
void OpenDeck::readButtons()    {

	uint8_t buttonState = 0;
	uint8_t rowState = 0;
	
	sendButtonReadCallback(rowState);	
	
	//iterate over rows
	for (int i=0; i<_numberOfButtonRows; i++) {
		
		//extract current bit from rowState variable
		//invert extracted bit because of pull-up resistors
		uint8_t currentBit = !((rowState >> i) & 0x01);
		
		//calculate current button number
		uint8_t buttonNumber = getActiveColumn()+i*_numberOfColumns;
		
		//get button state
		buttonState = checkButton(currentBit, previousButtonState[buttonNumber]);
		
		//if current button status is different from previous
		if (buttonState != previousButtonState[buttonNumber]) {
			
			if (buttonState == 0xFF)	{	
				
				//button is pressed
				//if button is configured as toggle
				if (getButtonType(buttonNumber)) {
					
					//if a button has been already pressed
					if (getButtonPressed(buttonNumber))	{
						
						//if longPress is enabled and longPressNote has already been sent
						if (getFeature(SOFTWARE_FEATURES_ADD, LONG_PRESS_EN_BIT) && getButtonLongPressed(buttonNumber))	{
							
							//send both regular and long press note off
							sendButtonDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);
							sendButtonDataCallback(buttonNote[buttonNumber], false, _longPressButtonNoteChannel);
							
						}
						
						//else send regular note off only
						else sendButtonDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);
						
						//reset pressed state
						setButtonPressed(buttonNumber, false);
						
				}	else {
					
					//send note on on press
					sendButtonDataCallback(buttonNote[buttonNumber], true, _buttonNoteChannel);
					
					//toggle buttonPressed flag to true
					setButtonPressed(buttonNumber, true);
					
				}
					
			}
			
			//button has momentary operation
			//send note on
			else sendButtonDataCallback(buttonNote[buttonNumber], true, _buttonNoteChannel);
				
			//start long press timer
			if (getFeature(SOFTWARE_FEATURES_ADD, LONG_PRESS_EN_BIT))	longPressState[buttonNumber] = millis();
				
		}
				
				else if ((buttonState == buttonDebounceCompare) && (!getButtonType(buttonNumber)))	{
					
					//button is released
					//check button on release only if it's momentary
					
						if (getFeature(SOFTWARE_FEATURES_ADD, LONG_PRESS_EN_BIT))	{
												
							if (getButtonLongPressed(buttonNumber)) {
													
								//send both regular and long press note off
								sendButtonDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);
								sendButtonDataCallback(buttonNote[buttonNumber], false, _longPressButtonNoteChannel);
													
							}
							
								else sendButtonDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);
								
								longPressState[buttonNumber] = 0;
								setButtonLongPressed(buttonNumber, false);
												
						}
						
							else sendButtonDataCallback(buttonNote[buttonNumber], false, _buttonNoteChannel);
												
				}
					
				//update previous reading with current
				previousButtonState[buttonNumber] = buttonState;
			
		}	
			
				if (getFeature(SOFTWARE_FEATURES_ADD, LONG_PRESS_EN_BIT))	{
					
					//send long press note if button has been pressed for defined time and note hasn't already been sent
					if ((millis() - longPressState[buttonNumber] >= _longPressTime) && (!getButtonLongPressed(buttonNumber)) && (buttonState == 0xFF))	{
				
					sendButtonDataCallback(buttonNote[buttonNumber], true, _longPressButtonNoteChannel);
					setButtonLongPressed(buttonNumber, true);
					
			}
					
		}
			
	}
	
}

uint8_t OpenDeck::checkButton(uint8_t currentState, uint8_t previousState)  {
	
	uint8_t buttonState = previousState;
	
	buttonState = (buttonState << 1) | currentState | buttonDebounceCompare;
	
	return buttonState;

}

void OpenDeck::setHandleButtonSend(void (*fptr)(uint8_t buttonNumber, bool buttonState, uint8_t channel))	{
	
	sendButtonDataCallback = fptr;
	
}

void OpenDeck::setHandleButtonRead(void (*fptr)(uint8_t &buttonColumnState))	{

	sendButtonReadCallback = fptr;	
	
}

//pots
bool OpenDeck::adcConnected(uint8_t adcChannel)	{
	
	//_analogueIn stores 8 variables, for each analogue pin on ATmega328p
	//if variable is true, analogue input is enabled
	//else code doesn't check specified input
	return bitRead(_analogueIn, adcChannel);
	
}

void OpenDeck::readPots()	{
	
	//reset potNumber on each call
	potNumber = 0;
	
	//check 8 analogue inputs on ATmega328p
	for (int i=0; i<8; i++)	
		if (adcConnected(i))	readPotsMux(i);
				
}

void OpenDeck::readPotsMux(uint8_t adcChannel) {

	//iterate over 8 inputs on 4051 mux
	for (int j=0; j<8; j++)	{
		
		//enable selected input
		sendSwitchMuxOutCallback(j);
		
		//add small delay between setting select pins and reading the input
		NOP;

		//read analogue value from mux
		int16_t tempValue = analogRead(adcChannel);

		//if new reading is stable, send new MIDI message
		checkPotReading(tempValue, potNumber);
			
		//increment pot number
		potNumber++;

	}

}

void OpenDeck::checkPotReading(int16_t currentValue, uint8_t potNumber)	{
	
	//calculate difference between current and previous reading
	int8_t analogueDiff = currentValue - lastAnalogueValue[potNumber];
	
	//get absolute difference
	if (analogueDiff < 0)	analogueDiff *= -1;
	
	uint32_t timeDifference = millis() - potTimer[potNumber];
	
		/*	
		
			When value from pot hasn't changed for more than POTENTIOMETER_MOVE_TIMEOUT value (time in ms), pot must 
			exceed MIDI_CC_STEP_TIMEOUT value. If the value has changed during POTENTIOMETER_MOVE_TIMEOUT, it must
			exceed MIDI_CC_STEP value.
			
		*/
		
		if (timeDifference < POTENTIOMETER_MOVE_TIMEOUT)	{
			
			if (analogueDiff >= MIDI_CC_STEP)	processPotReading(potNumber, currentValue);
			
		}

			else	{
								
				if (analogueDiff >= MIDI_CC_STEP_TIMEOUT)	processPotReading(potNumber, currentValue);	
					
		}
			
}

void OpenDeck::processPotReading(uint8_t potNumber, int16_t tempValue)	{
	
	uint8_t ccValue;
	uint8_t potNoteChannel = _longPressButtonNoteChannel+1;
				
	//invert CC data if potInverted is true
	if (getPotInvertState(potNumber))	ccValue = 127 - (tempValue >> 3);
	else	ccValue = tempValue >> 3;
		
	//only send data if pot is enabled and function isn't called in setup
	if ((sendPotCCDataCallback != NULL) && (getPotEnabled(potNumber)))
		sendPotCCDataCallback(ccNumber[potNumber], ccValue, _potCCchannel);
		
	if (getFeature(SOFTWARE_FEATURES_ADD, POT_NOTES_EN_BIT))	{
			
	uint8_t noteCurrent = getPotNoteValue(ccValue, ccNumber[potNumber]);
	
	//maximum number of notes per MIDI channel is 128, with 127 being final
	if (noteCurrent > 127)	{
		
		//if calculated note is bigger than 127, assign next midi channel
		potNoteChannel += noteCurrent/128;
		
		//substract 128*number of overflown channels from note
		noteCurrent -= 128*(noteCurrent/128);
		
	}
				
		if (checkPotNoteValue(potNumber, noteCurrent))	{
					
			//always send note off for previous value, except for the first read
			if ((lastPotNoteValue[potNumber] != 128) && (sendPotNoteOffDataCallback != NULL) && (getPotEnabled(potNumber)))
				sendPotNoteOffDataCallback(lastPotNoteValue[ccNumber[potNumber]], ccNumber[potNumber], (_longPressButtonNoteChannel+1));
			
			//send note on
			if ((sendPotNoteOnDataCallback != NULL) && (getPotEnabled(potNumber)))
				sendPotNoteOnDataCallback(noteCurrent, ccNumber[potNumber], (_longPressButtonNoteChannel+1));
				
			//update last value with current
			lastPotNoteValue[potNumber] = noteCurrent;;
					
		}
		
	}

	//update values
	lastAnalogueValue[potNumber] = tempValue;
	potTimer[potNumber] = millis();
	
}

uint8_t OpenDeck::getPotNoteValue(uint8_t analogueMIDIvalue, uint8_t potNumber)	{
	
	/*

    Each potentiometer alongside regular CC messages sends 6 different MIDI notes,
    depending on it's position. In the following table x represents the reading from
    pot and right column the MIDI note number:

    x=0:        0
    0<x<32:     1
    32<=x<64:   2
    64<=x<96:   3
    96<=x<127:  4
    x=127:      5

    */

    //variable to hold current modifier value
    uint8_t modifierValue = 6*potNumber;

    switch (analogueMIDIvalue)  {

        case 0:
        modifierValue += 0;
        break;

        case 127:
        modifierValue += 5;
        break;

        default:
        modifierValue += 1 + (analogueMIDIvalue >> 5);
        break;

    }

        return modifierValue;
		
}

bool OpenDeck::checkPotNoteValue(uint8_t potNumber, uint8_t noteCurrent)    {

	//make sure that modifier value is sent only once while the pot is in specified range
	if (lastPotNoteValue[potNumber] != noteCurrent) return true;
	return false;

}

void OpenDeck::setHandlePotCC(void (*fptr)(uint8_t potNumber, uint8_t ccValue, uint8_t channel))	{
	
	sendPotCCDataCallback = fptr;
	
}

void OpenDeck::setHandlePotNoteOn(void (*fptr)(uint8_t note, uint8_t potNumber, uint8_t channel))	{
	
	sendPotNoteOnDataCallback = fptr;
	
}

void OpenDeck::setHandlePotNoteOff(void (*fptr)(uint8_t note, uint8_t potNumber, uint8_t channel))	{
	
	sendPotNoteOffDataCallback = fptr;
	
}

void OpenDeck::setHandleMuxOutput(void (*fptr)(uint8_t muxInput))	{
	
	sendSwitchMuxOutCallback = fptr;
	
}


//LEDs
void OpenDeck::checkLEDs()  {
	
	//get currently active column
	uint8_t currentColumn = getActiveColumn();
	
	if (blinkEnabled)	switchBlinkState();
	
	//if there is an active LED in current column, turn on LED row
	for (int i=0; i<_numberOfLEDrows; i++)	
		if (ledOn(currentColumn+i*_numberOfColumns))	sendLEDrowOnCallback(i);

}

bool OpenDeck::checkLEDsOn()	{

	//return true if all LEDs are on
	for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)	if (ledState[i] != 0)	return false;
	return true;

}

bool OpenDeck::checkLEDsOff()	{

	//return true if all LEDs are off
	for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)	if (ledState[i] == 0)	return false;
	return true;

}

void OpenDeck::turnOnLED(uint8_t ledNumber)	{
	
	setConstantLEDstate(ledNumber);
	
}

void OpenDeck::turnOffLED(uint8_t ledNumber)	{
	
	ledState[ledNumber] = 0x00;
	
}

void OpenDeck::allLEDsOn()  {
	
	//turn on all LEDs
	for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)  setConstantLEDstate(i);
	
}

void OpenDeck::allLEDsOff()  {
	
	//turn off all LEDs
	for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)  ledState[i] = 0x00;
	
}

void OpenDeck::checkBlinkLEDs() {
	
	//this function will disable blinking
	//if none of the LEDs is in blinking state
	
	//else it will enable it
	
	bool _blinkEnabled = false;
	
	//if any LED is blinking, set timerState to true and exit the loop
	for (int i=0; i<MAX_NUMBER_OF_LEDS; i++) if (checkBlinkState(i))  { _blinkEnabled = true; break; }
	
	if (_blinkEnabled) blinkEnabled = true;
	
	//don't bother reseting variables if blinking is already disabled
	else if (!_blinkEnabled && blinkEnabled)	{
		
		//reset blinkState to default value
		blinkState = true;
		blinkTimerCounter = 0;
		blinkEnabled = false;
		
	}
	
}

void OpenDeck::oneByOneLED(bool ledDirection, bool singleLED, bool turnOn)  {   
	
	/*
	
	Function accepts three boolean arguments.
	 
	ledDirection: true means that LEDs will go from left to right, false from right to left
	singleLED: true means that only one LED will be active at the time, false means that LEDs
			     will turn on one by one until they're all lighted up
	
	turnOn: true means that LEDs will be turned on, with all previous LED states being 0
			  false means that all LEDs are lighted up and they turn off one by one, depending
			  on second argument
	 
    */

    //remember last time column was switched
	static uint32_t columnTime = 0;
	
	//while loop counter
    uint8_t passCounter = 0;
      
    //reset the timer on each function call
    uint32_t startUpTimer = 0;  
      
    //index of LED to be processed next
    uint8_t ledNumber;
     
    //if second and third argument of function are set to false or
	//if second argument is set to false and all the LEDs are turned off
	//light up all LEDs
	if ((!singleLED && !turnOn) || (checkLEDsOff() && !turnOn))
	allLEDsOn();
     
    if (turnOn)  {
	
	//This part of code deals with situations when previous function call has been
	//left direction and current one is right and vice versa.
	
	//On first function call, let's assume the direction was left to right. That would mean
	//that LEDs had to be processed in this order:
	
	//LED 1
	//LED 2
	//LED 3
	//LED 4
	
	//Now, when function is finished, LEDs are not reset yet with allLEDsOff() function to keep
	//track of their previous states. Next function call is right to left. On first run with 
	//right to left direction, the LED order would be standard LED 4 to LED 1, however, LED 4 has
	//been already turned on by first function call, so we check if its state is already set, and if
	//it is we increment or decrement ledNumber by one, depending on previous and current direction.
	//When function is called second time with direction different than previous one, the number of 
	//times it needs to execute is reduced by one, therefore passCounter is incremented.
	
    //right-to-left direction
    if (!ledDirection)
    //if last LED is turned on
	if (ledOn(ledID[totalNumberOfLEDs-1]))	{

	    //LED index is penultimate LED number
	    ledNumber = ledID[totalNumberOfLEDs-2];
	    //increment counter since the loop has to run one cycle less
	    passCounter++;

    }
    
	//led index is last one if last one isn't already on
	else ledNumber = ledID[totalNumberOfLEDs-1];

	//left-to-right direction
	else
	//if first LED is already on
	if (ledOn(ledID[0]))	{

		//led index is 1
		ledNumber = ledID[1];
		//increment counter
		passCounter++;

	}

	else ledNumber = ledID[0];
      
	}
    
    else  {

	//This is situation when all LEDs are turned on and we're turning them off one by one. Same
	//logic applies in both cases (see above). In this case we're not checking for whether the LED
	//is already turned on, but whether it's already turned off.
	
    //right-to-left direction
    if (!ledDirection)	if (!(ledOn(ledID[totalNumberOfLEDs-1])))	{

		ledNumber = ledID[totalNumberOfLEDs-2];
		passCounter++;

	}
    
	else ledNumber = ledID[totalNumberOfLEDs-1];
         
    //left-to-right direction
    else  if (!(ledOn(ledID[0])))   {

		ledNumber = ledID[1];
		passCounter++;

	}

	else ledNumber = ledID[0];
      
    }

    //on first function call, the while loop is called TOTAL_NUMBER_OF_LEDS+1 times
	//to get empty cycle after processing last LED
	while (passCounter < totalNumberOfLEDs+1)  {	
      
		if ((millis() - columnTime) > getTimedLoopTime())  {   
    
			//activate next column
			nextColumn();

			//only process LED after defined time
			if ((millis() - startUpTimer) > _startUpLEDswitchTime)  {
  
				if (passCounter < totalNumberOfLEDs)  {
  
					//if we're turning LEDs on one by one, turn all the other LEDs off
					if (singleLED && turnOn)  allLEDsOff(); 
                                        
                    //if we're turning LEDs off one by one, turn all the other LEDs on
					else if (!turnOn && singleLED)	allLEDsOn();
                                        
                    //set LED state depending on turnOn parameter
					if (turnOn)  turnOnLED(ledNumber);
						else turnOffLED(ledNumber);

					//make sure out-of-bound index isn't requested from ledArray
				    if (passCounter < totalNumberOfLEDs-1)	{

						//right-to-left direction
						if (!ledDirection)	ledNumber = ledID[totalNumberOfLEDs - 2 - passCounter];

						//left-to-right direction
						else	if (passCounter < totalNumberOfLEDs-1)	ledNumber = ledID[passCounter+1];

					}
                                          
				} 		

            //always increment pass counter
		    passCounter++;

		    //update timer
		    startUpTimer = millis();
  
		}
    
		    //check if there is any LED to be turned on
		    checkLEDs();
			
			//update last time column was switched
			columnTime = millis();
      
		}

	}
  
}

void OpenDeck::switchBlinkState()  {
	
	if ((millis() - blinkTimerCounter) >= _blinkTime)	{
		
		//change blinkBit state and write it into ledState variable if LED is in blink state
		for (int i = 0; i<MAX_NUMBER_OF_LEDS; i++)
		if (checkBlinkState(i))	setBlinkState(i, blinkState);
		
		//invert blink state
		blinkState = !blinkState;
		
		//update blink timer
		blinkTimerCounter = millis();
		
	}

}

bool OpenDeck::ledOn(uint8_t ledNumber)   {

	if  (

	ledState[ledNumber] == 0x05 ||
	ledState[ledNumber] == 0x15 ||
	ledState[ledNumber] == 0x16 ||
	ledState[ledNumber] == 0x1D ||
	ledState[ledNumber] == 0x0D ||
	ledState[ledNumber] == 0x17

	)  return true;
		
		return false;

}

void OpenDeck::setConstantLEDstate(uint8_t ledNumber)   {

	ledState[ledNumber] = 0x05;

}

void OpenDeck::setBlinkState(uint8_t ledNumber, bool blinkState)    {

	switch (blinkState) {

		case true:
		ledState[ledNumber] |= 0x10;
		break;

		case false:
		ledState[ledNumber] &= 0xEF;
		break;

	}

}

bool OpenDeck::checkBlinkState(uint8_t ledNumber)   {

	//function returns true if blinking bit in ledState is set
	return ((ledState[ledNumber] >> 1) & (0x01));

}

void OpenDeck::handleLED(uint8_t ledNote, bool currentLEDstate, bool blinkMode)   {

  /*

  LED state is stored into one byte (ledState). The bits have following meaning (7 being the MSB bit):

  7: x
  6: x
  5: x
  4: Blink bit (timer changes this bit)
  3: "Remember" bit, used to restore previous LED state
  2: LED is active (either it blinks or it's constantly on), this bit is OR function between bit 0 and 1
  1: LED blinks
  0: LED is constantly turned on

  */
  
  uint8_t _ledID = ledID[ledNote];

       switch (currentLEDstate) {

        case false:
        //note off event

        //if remember bit is set
        if ((ledState[_ledID] >> 3) & (0x01))  {

          //if note off for blink state is received
          //clear remember bit and blink bits
          //set constant state bit
          if (blinkMode)  ledState[_ledID] = 0x05;
            //else clear constant state bit and remember bit
            //set blink bits
            else ledState[_ledID] = 0x16;

        }

        else {

        if (blinkMode)  /*clear blink bit */ ledState[_ledID] &= 0x15;
          else /* clear constant state bit */ ledState[_ledID] &= 0x16;

        }

        //if bits 0 and 1 are 0, LED is off so we set ledState to zero
        if (!(ledState[_ledID] & 3))  ledState[_ledID] = 0x00;

        break;

        case true:
        //note on event

        //if constant note on is received and LED is already blinking
        //clear blinking bits and set remember bit and constant bit
        if ((!blinkMode) && checkBlinkState(_ledID))  ledState[_ledID] = 0x0D;

        //set bit 2 to 1 in any case (constant/blink state)
        else ledState[_ledID] |= (0x01 << blinkMode) | 0x04 | (blinkMode << 4);

  }	

}

void OpenDeck::setLEDState()	{
	
	//ignore every note bigger than totalNumberOfLEDs*2 to avoid out-of-bound request
	if (receivedNotePitch < totalNumberOfLEDs*2)	{
		
		bool currentLEDstate;
		uint8_t ledNote;
		
		//if blinkMode is 1, the LED is blinking
		uint8_t blinkMode = 0;
		
		//if velocity is 0, turn off LED
		//else turn it on
		if (receivedNoteVelocity != 0)  currentLEDstate = true;
		else currentLEDstate = false;
		
		if (receivedNotePitch >= totalNumberOfLEDs)  blinkMode = 1;
		
		//it's important to get the same led number in either case
		ledNote = receivedNotePitch - (totalNumberOfLEDs*blinkMode);
		
		handleLED(ledNote, currentLEDstate, blinkMode);
		
		if (blinkMode && currentLEDstate)	blinkEnabled = true;
		else	checkBlinkLEDs();
		
	}
	
		receivedNoteProcessed = true;

}

void OpenDeck::checkReceivedNote()	{
		
		if (!receivedNoteProcessed)	setLEDState();
	
}

void OpenDeck::storeReceivedNote(uint8_t channel, uint8_t pitch, uint8_t velocity)	{
	
	receivedNoteChannel = channel;
	receivedNotePitch = pitch;
	receivedNoteVelocity = velocity;
	
	receivedNoteProcessed = false;
	
}

void OpenDeck::setHandleLEDrowOn(void (*fptr)(uint8_t ledRow))	{
	
	sendLEDrowOnCallback = fptr;
	
}

void OpenDeck::setHandleLEDrowsOff(void (*fptr)())	{
	
	sendLEDrowsOffCallback = fptr;
	
}


//columns

void OpenDeck::nextColumn()   {
	
	sendLEDrowsOffCallback();
	
	if (column == _numberOfColumns)	column = 0;
	
	sendColumnSwitchCallback(column);
	
	//increment column
	column++;

}

uint8_t OpenDeck::getActiveColumn() {

	//return currently active column
	return (column - 1);

}

void OpenDeck::setHandleColumnSwitch(void (*fptr)(uint8_t columnNumber))	{
	
	sendColumnSwitchCallback = fptr;
	
}


//setters

void OpenDeck::setNumberOfColumns(uint8_t numberOfColumns)	{
	
	_numberOfColumns = numberOfColumns;
	
}

void OpenDeck::setNumberOfButtonRows(uint8_t numberOfButtonRows)	{
	
	_numberOfButtonRows = numberOfButtonRows;
	
}

void OpenDeck::setNumberOfLEDrows(uint8_t numberOfLEDrows)	{
	
	_numberOfLEDrows = numberOfLEDrows;
		
}

void OpenDeck::setNumberOfMux(uint8_t numberOfMux)	{
	
	_numberOfMux = numberOfMux;
	
}

void OpenDeck::enableAnalogueInput(uint8_t adcChannel)	{
	
	if ((adcChannel >=0) && (adcChannel < 8))
		bitWrite(_analogueIn, adcChannel, 1);
	
}


//SysEx functions

//restore default configuration

void OpenDeck::setDefaultConf()	{

	//write default configuration stored in PROGMEM to EEPROM
	for (int i=0; i<(int16_t)sizeof(defConf); i++)
	eeprom_update_byte((uint8_t*)i, pgm_read_byte(&(defConf[i])));
	
}

bool OpenDeck::getFeature(uint8_t featureType, uint8_t feature)	{
	
	switch (featureType)	{
		
		case SOFTWARE_FEATURES_ADD:
		//software feature
		return bitRead(softwareFeatures, feature);
		break;
		
		case HARDWARE_FEATURES_ADD:
		//hardware feature
		return bitRead(hardwareFeatures, feature);
		break;
		
		default:
		break;
		
		
	}	return false;
	
}

bool OpenDeck::setFeature(uint8_t featureType, uint8_t feature, bool state)	{
	
	switch (featureType)	{
		
		case SOFTWARE_FEATURES_ADD:
		//software feature
		bitWrite(softwareFeatures, feature, state);
		eeprom_update_byte((uint8_t*) SOFTWARE_FEATURES_ADD, softwareFeatures);
		return (eeprom_read_byte((uint8_t*)SOFTWARE_FEATURES_ADD) == softwareFeatures);
		break;
		
		case HARDWARE_FEATURES_ADD:
		//hardware feature
		bitWrite(hardwareFeatures, feature, state);
		eeprom_update_byte((uint8_t*) HARDWARE_FEATURES_ADD, hardwareFeatures);
		return (eeprom_read_byte((uint8_t*)HARDWARE_FEATURES_ADD) == hardwareFeatures);
		break;
		
		default:
		break;
		
		
	}	return false;
	
}

uint8_t OpenDeck::getHardwareParameter(uint8_t parameter)	{
	
	switch (parameter)	{
		
		case 0:
		//long press time
		return _longPressTime;
		break;
		
		case 1:
		//blink time
		return _blinkTime;
		break;
		
		case 2:
		//start-up led switch time
		return _startUpLEDswitchTime;
		break;
		
		default:
		break;
		
		
	}	return 0;
	
}

bool OpenDeck::setHardwareParameter(uint8_t parameter, uint8_t value)	{
	
	switch (parameter)	{
		
		case 0:
		//long press time
		_longPressTime = value*100;
		eeprom_update_byte((uint8_t*)LONG_PRESS_TIME_ADD, value);
		return (eeprom_read_byte((uint8_t*)LONG_PRESS_TIME_ADD) == value);
		break;
		
		case 1:
		//blink time
		_blinkTime = value*100;
		eeprom_update_byte((uint8_t*)BLINK_TIME_ADD, value);
		return (eeprom_read_byte((uint8_t*)BLINK_TIME_ADD) == value);
		break;
		
		case 2:
		//start-up led switch time
		_startUpLEDswitchTime = value*10;
		eeprom_update_byte((uint8_t*)START_UP_SWITCH_TIME_ADD, value);
		return (eeprom_read_byte((uint8_t*)START_UP_SWITCH_TIME_ADD) == value);
		break;
		
		default:
		break;
		
		
	}	return false;
	
}

uint8_t OpenDeck::getInputMIDIchannel()	{
	
	return _inputChannel;
	
}

bool OpenDeck::getButtonType(uint8_t buttonNumber)	{
	
	uint8_t arrayIndex = buttonNumber/8;
	uint8_t buttonIndex = buttonNumber - 8*arrayIndex;
	
	return bitRead(buttonType[arrayIndex], buttonIndex);
	
}

bool OpenDeck::setButtonType(uint8_t buttonNumber, bool type)	{
	
	uint8_t arrayIndex = buttonNumber/8;
	uint8_t buttonIndex = buttonNumber - 8*arrayIndex;
	uint16_t eepromAddress = BUTTON_TYPE_ADD+arrayIndex;
	
	bitWrite(buttonType[arrayIndex], buttonIndex, type);
	eeprom_update_byte((uint8_t*)eepromAddress, buttonType[arrayIndex]);
	
	return (buttonType[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));
	
}

bool OpenDeck::getButtonPressed(uint8_t buttonNumber)	{
	
	uint8_t arrayIndex = buttonNumber/8;
	uint8_t buttonIndex = buttonNumber - 8*arrayIndex;
	
	return bitRead(buttonPressed[arrayIndex], buttonIndex);
	
}

void OpenDeck::setButtonPressed(uint8_t buttonNumber, bool state)	{
	
	uint8_t arrayIndex = buttonNumber/8;
	uint8_t buttonIndex = buttonNumber - 8*arrayIndex;
	
	bitWrite(buttonPressed[arrayIndex], buttonIndex, state);
	
}

bool OpenDeck::getButtonLongPressed(uint8_t buttonNumber)	{
	
	uint8_t arrayIndex = buttonNumber/8;
	uint8_t buttonIndex = buttonNumber - 8*arrayIndex;
	
	return bitRead(longPressSent[arrayIndex], buttonIndex);
	
}

void OpenDeck::setButtonLongPressed(uint8_t buttonNumber, bool state)	{
	
	uint8_t arrayIndex = buttonNumber/8;
	uint8_t buttonIndex = buttonNumber - 8*arrayIndex;
	
	bitWrite(longPressSent[arrayIndex], buttonIndex, state);
	
}

bool OpenDeck::getPotInvertState(uint8_t potNumber)	{
	
	uint8_t arrayIndex = potNumber/8;
	uint8_t potIndex = potNumber - 8*arrayIndex;
	
	return bitRead(potInverted[arrayIndex], potIndex);
	
}

bool OpenDeck::setPotInvertState(uint8_t potNumber, bool state)	{
	
	uint8_t arrayIndex = potNumber/8;
	uint8_t potIndex = potNumber - 8*arrayIndex;
	uint16_t eepromAddress = POT_INVERSION_STATUS_ADD+arrayIndex;
	
	bitWrite(potInverted[arrayIndex], potIndex, state);
	eeprom_update_byte((uint8_t*)eepromAddress, potInverted[arrayIndex]);
	
	return (potInverted[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));
	
}

bool OpenDeck::getPotEnabled(uint8_t potNumber)	{
	
	uint8_t arrayIndex = potNumber/8;
	uint8_t potIndex = potNumber - 8*arrayIndex;
	
	return bitRead(potEnabled[arrayIndex], potIndex);
	
}

bool OpenDeck::setPotEnabled(uint8_t potNumber, bool state)	{
	
	uint8_t arrayIndex = potNumber/8;
	uint8_t potIndex = potNumber - 8*arrayIndex;
	uint16_t eepromAddress = POT_ENABLED_STATUS_ADD+arrayIndex;
	
	bitWrite(potEnabled[arrayIndex], potIndex, state);
	eeprom_update_byte((uint8_t*)eepromAddress, potEnabled[arrayIndex]);
	
	return (potEnabled[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));
	
}

uint8_t OpenDeck::getMIDIchannel(uint8_t channel)	{
	
	switch (channel)	{
		
		case 0:
		return _buttonNoteChannel;
		break;
		
		case 1:
		return _longPressButtonNoteChannel;
		break;
		
		case 2:
		return _potCCchannel;
		break;
		
		case 3:
		return _encCCchannel;
		break;
		
		case 4:
		return _inputChannel;
		break;
		
		default:
		return 0;
		
	}
	
}

bool OpenDeck::setMIDIchannel(uint8_t channel, uint8_t channelNumber)	{
	
	switch (channel)	{
		
		case 0:
		_buttonNoteChannel = channelNumber;
		eeprom_update_byte((uint8_t*)BUTTON_NOTE_CHANNEL_ADD, channelNumber);
		return (channelNumber == eeprom_read_byte((uint8_t*)BUTTON_NOTE_CHANNEL_ADD));
		break;
		
		case 1:
		_longPressButtonNoteChannel = channelNumber;
		eeprom_update_byte((uint8_t*)LONG_PRESS_BUTTON_NOTE_CHANNEL_ADD, channelNumber);
		return (channelNumber == eeprom_read_byte((uint8_t*)LONG_PRESS_BUTTON_NOTE_CHANNEL_ADD));
		break;
		
		case 2:
		_potCCchannel = channelNumber;
		eeprom_update_byte((uint8_t*)POT_CC_CHANNEL_ADD, channelNumber);
		return (channelNumber == eeprom_read_byte((uint8_t*)POT_CC_CHANNEL_ADD));
		break;
		
		case 3:
		_encCCchannel = channelNumber;
		eeprom_update_byte((uint8_t*)ENC_CC_CHANNEL_ADD, channelNumber);
		return (channelNumber == eeprom_read_byte((uint8_t*)ENC_CC_CHANNEL_ADD));
		break;
		
		case 4:
		_inputChannel = channelNumber;
		eeprom_update_byte((uint8_t*)INPUT_CHANNEL_ADD, channelNumber);
		return (channelNumber == eeprom_read_byte((uint8_t*)INPUT_CHANNEL_ADD));
		break;
		
		default:
		return false;
		
	}
	
}

uint8_t OpenDeck::getCCnumber(uint8_t potNumber)	{
	
	return ccNumber[potNumber];
	
}

bool OpenDeck::setCCnumber(uint8_t potNumber, uint8_t _ccNumber)	{
	
	uint16_t eepromAddress = POT_CC_NUMBER_ADD+potNumber;
	
	ccNumber[potNumber] = _ccNumber;
	eeprom_update_byte((uint8_t*)eepromAddress, _ccNumber);
	return (_ccNumber == eeprom_read_byte((uint8_t*)eepromAddress));
	
}

uint8_t OpenDeck::getButtonNote(uint8_t buttonNumber)	{
	
	return buttonNote[buttonNumber];
	
}

bool OpenDeck::setButtonNote(uint8_t buttonNumber, uint8_t _buttonNote)	{
	
	uint16_t eepromAddress = BUTTON_NOTE_NUMBER_ADD+potNumber;
	
	buttonNote[buttonNumber] = _buttonNote;
	eeprom_update_byte((uint8_t*)eepromAddress, _buttonNote);
	return (_buttonNote == eeprom_read_byte((uint8_t*)eepromAddress));
	
}

uint8_t OpenDeck::getLEDID(uint8_t ledNumber)	{
	
	return ledID[ledNumber];
	
}

bool OpenDeck::setLEDID(uint8_t ledNumber, uint8_t _ledID)	{
	
	uint16_t eepromAddress = LED_ID_ADD+ledNumber;
	
	ledID[ledNumber] = _ledID;
	eeprom_update_byte((uint8_t*)eepromAddress, _ledID);
	return _ledID == eeprom_read_byte((uint8_t*)eepromAddress);
	
}

//create instance of library automatically
OpenDeck openDeck;