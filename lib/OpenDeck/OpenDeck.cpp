/*

OpenDECK library v1.90
Last revision date: 2014-07-30
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include "Ownduino.h"
#include <avr/io.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <util/crc16.h>

OpenDeck::OpenDeck()    {

  //initialization
  initVariables();
  
  //set all callback to NULL pointer
  
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

void OpenDeck::initVariables()  {

	//reset all variables
	
	//MIDI channels
	buttonNoteChannel			= 0;
	longPressButtonNoteChannel	= 0;
	ccChanelPot					= 0;
	ccChannelEnc				= 0;
	inputChannel				= 0;
	
	//hardware params
	longPressTime				= 0;
	blinkTime					= 0;
	startUpLEDswitchTime		= 0;
	
	//software features
	_startUpRoutineEnabled		= false;
	_ledBlinkEnabled			= false;
	_longPressEnabled			= false;
	_potNotesEnabled			= false;
	_encoderNotesEnabled		= false;
	_standardNoteOffEnabled		= false;
	_runningStatusEnabled		= false;
	
	//hardware features
	_buttonsEnabled				= false;
	_ledsEnabled				= false;
	_potsEnabled				= false;
	_encodersEnabled			= false;
	
	//buttons	 	  
	for (i=0; i<MAX_NUMBER_OF_BUTTONS; i++)  {
			
	buttonNote[i]				= 0;
	previousButtonState[i]		= 0;
	buttonType[i]				= 0;
	longPressSent[i]			= false;
	longPressState[i]			= 0;
			
	}
		
	buttonDebounceCompare		= 0;
	  
	//pots
	for (i=0; i<MAX_NUMBER_OF_POTS; i++)  {

	potInverted[i]				= false;
	potEnabled[i]				= false;
	ccNumber[i]					= 0;
	lastPotNoteValue[i]			= 0;
	lastAnalogueValue[i]		= 0;
	potTimer[i]					= 0;

	}
	
	for (i=0; i<8; i++)
		_analogueIn[i]			= false;
	  
	potNumber					= 0;

	//LEDs
	for (i=0; i<MAX_NUMBER_OF_LEDS; i++)  {
		
		ledState[i]				= 0;
		_ledNumber[i]			= 0;
		
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
	receivedNoteProcessed	= false;

	//make initial pot reading to avoid sending all data on startup
	readPots();
	
	//get all values from EEPROM
	getConfiguration();
	
}

void OpenDeck::setHandlePinInit(void (*fptr)())	{
	
	sendInitPinsCallback = fptr;
	
}


//configuration retrieve

void OpenDeck::getConfiguration()	{
	
	//get configuration from EEPROM
	getMIDIchannels();
	getHardwareParams();
	getSoftwareFeatures();
	getHardwareFeatures();
	getPotInvertStates();
	getEnabledPots();
	getCCnumbers();
	getButtonNumbers();
	getButtonType();
	getLEDnumbers();
	getTotalLEDnumber();
	
}

void OpenDeck::getMIDIchannels()	{
	
	buttonNoteChannel			= eeprom_read_byte((uint8_t*)0);
	longPressButtonNoteChannel	= eeprom_read_byte((uint8_t*)1);
	ccChanelPot					= eeprom_read_byte((uint8_t*)2);
	ccChannelEnc				= eeprom_read_byte((uint8_t*)3);
	inputChannel				= eeprom_read_byte((uint8_t*)4);
	
}

void OpenDeck::getHardwareParams()	{
	
	longPressTime				= eeprom_read_byte((uint8_t*)5) * 100;
	blinkTime					= eeprom_read_byte((uint8_t*)6) * 100;
	startUpLEDswitchTime		= eeprom_read_byte((uint8_t*)7) * 10;
	
}

void OpenDeck::getSoftwareFeatures()	{
	
	uint8_t features = eeprom_read_byte((uint8_t*)8);
	
	_startUpRoutineEnabled	= features >> 6;
	_ledBlinkEnabled		= (features >> 5) & 0x01;
	_longPressEnabled		= (features >> 4) & 0x01;
	_potNotesEnabled		= (features >> 3) & 0x01;
	_encoderNotesEnabled	= (features >> 2) & 0x01;
	_standardNoteOffEnabled = (features >> 1) & 0x01;
	_runningStatusEnabled	= features & 0x01;
	
}

void OpenDeck::getHardwareFeatures()	{
	
	uint8_t features = eeprom_read_byte((uint8_t*)9);
	
	_buttonsEnabled		= features >> 3;
	_ledsEnabled		= ((features >> 2) & 0x01);
	_potsEnabled		= ((features >> 1) & 0x01);
	_encodersEnabled	= (features & 0x01);
	
}

void OpenDeck::getPotInvertStates()	{
	
	uint8_t inversionEnabled;
	uint16_t eepromAddress = 10;
	
	for (int i=0; i<(MAX_NUMBER_OF_POTS/8); i++)	{
		
		inversionEnabled = eeprom_read_byte((uint8_t*)eepromAddress);
		eepromAddress++;
		
		for (int j=0; j<8; j++)	potInverted[i*8+j] = (inversionEnabled >> j) & 0x01;;

	}
	
}

void OpenDeck::getEnabledPots()	{
	
	uint8_t _potEnabled;
	uint16_t eepromAddress = 26;
	
	for (int i=0; i<(MAX_NUMBER_OF_POTS/8); i++)	{
		
		_potEnabled = eeprom_read_byte((uint8_t*)eepromAddress);
		eepromAddress++;
		
		for (int j=0; j<8; j++)	potEnabled[i*8+j] = (_potEnabled >> j) & 0x01;
		
	}
	
}

void OpenDeck::getCCnumbers()	{
	
	uint16_t eepromAddress = 42;
	
	for (int i=0; i<MAX_NUMBER_OF_POTS; i++)	{
		
		ccNumber[i] = eeprom_read_byte((uint8_t*)eepromAddress);
		eepromAddress++;
		
	}
	
}

void OpenDeck::getButtonNumbers()	{
	
	uint16_t eepromAddress = 170;
	
	for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)	{
		
		buttonNote[i] = eeprom_read_byte((uint8_t*)eepromAddress);
		eepromAddress++;
		
	}
	
}

void OpenDeck::getButtonType()	{
	
	uint8_t _buttonType;
	uint16_t eepromAddress = 298;
	
	for (int i=0; i<MAX_NUMBER_OF_BUTTONS/8; i++)	{
		
		_buttonType = eeprom_read_byte((uint8_t*)eepromAddress);
		eepromAddress++;
		
		for (int j=0; j<8; j++)	buttonType[i*8+j] = (_buttonType >> j) & 0x01;
		
	}
	
}

void OpenDeck::getLEDnumbers()	{
	
	uint16_t eepromAddress = 314;
	
	for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)	{
		
		_ledNumber[i] = eeprom_read_byte((uint8_t*)eepromAddress);
		eepromAddress++;
		
	}
	
}

void OpenDeck::getTotalLEDnumber()	{
	
	totalNumberOfLEDs = eeprom_read_byte((uint8_t*)442);
	
}


//restore default configuration

void OpenDeck::setDefaultConf()	{

	//write default configuration stored in PROGMEM to EEPROM
	for (int i=0; i<(int16_t)sizeof(defConf); i++)	
		eeprom_update_byte((uint8_t*)i, pgm_read_byte(&(defConf[i])));
			
}
  
uint8_t OpenDeck::getInputChannel()	{
	
	//return listening MIDI channel
	return inputChannel;
	
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
				if (buttonType[buttonNumber]) {
					
					//if a button has been already pressed
					if (buttonPressed[buttonNumber])	{
						
						//if longPress is enabled and longPressNote has already been sent
						if (_longPressEnabled && longPressSent[buttonNumber])	{
							
							//send both regular and long press note off
							sendButtonDataCallback(buttonNote[buttonNumber], false, buttonNoteChannel);
							sendButtonDataCallback(buttonNote[buttonNumber], false, longPressButtonNoteChannel);
							
						}
						
						//else send regular note off only
						else sendButtonDataCallback(buttonNote[buttonNumber], false, buttonNoteChannel);
						
						//reset pressed state
						buttonPressed[buttonNumber] = false;
						
				}	else {
					
					//send note on on press
					sendButtonDataCallback(buttonNote[buttonNumber], true, buttonNoteChannel);
					
					//toggle buttonPressed flag to true
					buttonPressed[buttonNumber] = true;
					
				}
					
			}
			
			//button has momentary operation
			//send note on
			else sendButtonDataCallback(buttonNote[buttonNumber], true, buttonNoteChannel);
				
			//start long press timer
			if (_longPressEnabled)	longPressState[buttonNumber] = millis();
				
		}
				
				else if ((buttonState == buttonDebounceCompare) && (!buttonType[buttonNumber]))	{
					
					//button is released
					//check button on release only if it's momentary
					
						if (_longPressEnabled)	{
												
							if (longPressSent[buttonNumber]) {
													
								//send both regular and long press note off
								sendButtonDataCallback(buttonNote[buttonNumber], false, buttonNoteChannel);
								sendButtonDataCallback(buttonNote[buttonNumber], false, longPressButtonNoteChannel);
													
							}
							
								else sendButtonDataCallback(buttonNote[buttonNumber], false, buttonNoteChannel);
								
								longPressState[buttonNumber] = 0;
								longPressSent[buttonNumber] = false;
												
						}
						
							else sendButtonDataCallback(buttonNote[buttonNumber], false, buttonNoteChannel);
												
				}
					
				//update previous reading with current
				previousButtonState[buttonNumber] = buttonState;
			
		}	
			
				if (_longPressEnabled)	{
					
					//send long press note if button has been pressed for defined time and note hasn't already been sent
					if ((millis() - longPressState[buttonNumber] >= longPressTime) && (!longPressSent[buttonNumber]) && (buttonState == 0xFF))	{
				
					sendButtonDataCallback(buttonNote[buttonNumber], true, longPressButtonNoteChannel);
					longPressSent[buttonNumber] = true;
					
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
	return _analogueIn[adcChannel];
	
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
	uint8_t potNoteChannel = longPressButtonNoteChannel+1;
				
	//invert CC data if potInverted is true
	if (potInverted[potNumber])	ccValue = 127 - (tempValue >> 3);
	else	ccValue = tempValue >> 3;
		
	//only send data if pot is enabled and function isn't called in setup
	if ((sendPotCCDataCallback != NULL) && (potEnabled[potNumber]))
		sendPotCCDataCallback(ccNumber[potNumber], ccValue, ccChanelPot);
		
	if (_potNotesEnabled)	{
			
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
			if ((lastPotNoteValue[potNumber] != 128) && (sendPotNoteOffDataCallback != NULL) && (potEnabled[potNumber]))
				sendPotNoteOffDataCallback(lastPotNoteValue[ccNumber[potNumber]], ccNumber[potNumber], (longPressButtonNoteChannel+1));
			
			//send note on
			if ((sendPotNoteOnDataCallback != NULL) && (potEnabled[potNumber]))
				sendPotNoteOnDataCallback(noteCurrent, ccNumber[potNumber], (longPressButtonNoteChannel+1));
				
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
	if (ledOn(_ledNumber[totalNumberOfLEDs-1]))	{

	    //LED index is penultimate LED number
	    ledNumber = _ledNumber[totalNumberOfLEDs-2];
	    //increment counter since the loop has to run one cycle less
	    passCounter++;

    }
    
	//led index is last one if last one isn't already on
	else ledNumber = _ledNumber[totalNumberOfLEDs-1];

	//left-to-right direction
	else
	//if first LED is already on
	if (ledOn(_ledNumber[0]))	{

		//led index is 1
		ledNumber = _ledNumber[1];
		//increment counter
		passCounter++;

	}

	else ledNumber = _ledNumber[0];
      
	}
    
    else  {

	//This is situation when all LEDs are turned on and we're turning them off one by one. Same
	//logic applies in both cases (see above). In this case we're not checking for whether the LED
	//is already turned on, but whether it's already turned off.
	
    //right-to-left direction
    if (!ledDirection)	if (!(ledOn(_ledNumber[totalNumberOfLEDs-1])))	{

		ledNumber = _ledNumber[totalNumberOfLEDs-2];
		passCounter++;

	}
    
	else ledNumber = _ledNumber[totalNumberOfLEDs-1];
         
    //left-to-right direction
    else  if (!(ledOn(_ledNumber[0])))   {

		ledNumber = _ledNumber[1];
		passCounter++;

	}

	else ledNumber = _ledNumber[0];
      
    }

    //on first function call, the while loop is called TOTAL_NUMBER_OF_LEDS+1 times
	//to get empty cycle after processing last LED
	while (passCounter < totalNumberOfLEDs+1)  {	
      
		if ((millis() - columnTime) > getTimedLoopTime())  {   
    
			//activate next column
			nextColumn();

			//only process LED after defined time
			if ((millis() - startUpTimer) > startUpLEDswitchTime)  {
  
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
						if (!ledDirection)	ledNumber = _ledNumber[totalNumberOfLEDs - 2 - passCounter];

						//left-to-right direction
						else	if (passCounter < totalNumberOfLEDs-1)	ledNumber = _ledNumber[passCounter+1];

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
	
	if ((millis() - blinkTimerCounter) >= blinkTime)	{
		
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
  
  uint8_t ledNumber = _ledNumber[ledNote];

       switch (currentLEDstate) {

        case false:
        //note off event

        //if remember bit is set
        if ((ledState[ledNumber] >> 3) & (0x01))  {

          //if note off for blink state is received
          //clear remember bit and blink bits
          //set constant state bit
          if (blinkMode)  ledState[ledNumber] = 0x05;
            //else clear constant state bit and remember bit
            //set blink bits
            else ledState[ledNumber] = 0x16;

        }

        else {

        if (blinkMode)  /*clear blink bit */ ledState[ledNumber] &= 0x15;
          else /* clear constant state bit */ ledState[ledNumber] &= 0x16;

        }

        //if bits 0 and 1 are 0, LED is off so we set ledState to zero
        if (!(ledState[ledNumber] & 3))  ledState[ledNumber] = 0x00;

        break;

        case true:
        //note on event

        //if constant note on is received and LED is already blinking
        //clear blinking bits and set remember bit and constant bit
        if ((!blinkMode) && checkBlinkState(ledNumber))  ledState[ledNumber] = 0x0D;

        //set bit 2 to 1 in any case (constant/blink state)
        else ledState[ledNumber] |= (0x01 << blinkMode) | 0x04 | (blinkMode << 4);

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


//getters

bool OpenDeck::buttonsEnabled()	{
	
	return _buttonsEnabled;
	
}

bool OpenDeck::ledsEnabled()	{
	
	return _ledsEnabled;
	
}

bool OpenDeck::potsEnabled()	{
	
	return _potsEnabled;
	
}

bool OpenDeck::startUpRoutineEnabled()	{
	
	return _startUpRoutineEnabled;
	
}

bool OpenDeck::standardNoteOffEnabled()	{
	
	return _standardNoteOffEnabled;
	
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
	
	_analogueIn[adcChannel] = true;
	
}

//create instance of library automatically
OpenDeck openDeck;