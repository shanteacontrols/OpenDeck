/*

OpenDECK library v1.0
Last revision date: 2014-05-19
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include "Ownduino.h"
#include <avr/io.h>
#include <stdlib.h>

OpenDeck::OpenDeck()    {

  //initialization
  
  HardwareReadSpecific::initPins();
  initVariables();
  
  #ifdef BUTTON_MATRIX
	sendButtonDataCallback = NULL;
  #endif
  
  #ifdef POTS
	sendPotCCDataCallback = NULL;
	sendPotNoteOnDataCallback = NULL;
	sendPotNoteOffDataCallback = NULL;
  #endif  
      
}


//init

void OpenDeck::initVariables()  {
	  
	  //set default variable states
	  
	  #ifdef BUTTON_MATRIX
		for (i=0; i<MAX_NUMBER_OF_BUTTONS; i++)  {
			
			previousButtonState[i] = 0;
			
			#ifdef LONG_PRESS_TIME
				longPressState[i] = 0;
				longPressSent[i] = false;
			#endif
			
		}
		
		buttonDebounceCompare = 0;
		numberOfColumnPasses = 0;
	  #endif
	  
	  #ifdef POTS
	  
	  for (i=0; i<TOTAL_NUMBER_OF_POTS; i++)  {

		  lastAnalogueValue[i] = 0;
		  lastPotNoteValue[i] = 0;
		  potTimer[i] = 0;

	  }
	  
	  potNumber = 0;
	  
	  #ifdef MUX
		muxInput = 0;
	  #endif
	  
	  #endif

	  #ifdef LED_MATRIX
	  for (i=0; i<MAX_NUMBER_OF_LEDS; i++)  ledState[i] = 0;
	 
	  //global blink state
	  blinkState = true;
	  
	  //enable/disable blinking
	  blinkEnabled = false;
	  
	  //blink timer
	  blinkTimerCounter = 0;
	  #endif
	  
	  //column counter
	  column = 0;
	  
	  receivedNoteProcessed = true;
	  
	  receivedNoteChannel = 0;
	  receivedNotePitch = 0;
	  receivedNoteVelocity = 0;
	  
}

void OpenDeck::init()	{
	
	#ifdef BUTTON_MATRIX
		setNumberOfColumnPasses();
		for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)	previousButtonState[i] = buttonDebounceCompare;
	#endif
	
	#ifdef POTS
		//read pots on startup to avoid sending all pot data
		//when controller is turned on
		for (int i=0; i<TOTAL_NUMBER_OF_POTS; i++)	lastPotNoteValue[i] = 128;
		
		readPots();
	#endif
}
  
//buttons

#ifdef BUTTON_MATRIX

void OpenDeck::setNumberOfColumnPasses() {
	
	/*
		
		Algorithm calculates how many times does it need to read whole row
		before it can declare button reading stable.
	
	*/
	  
	uint8_t rowPassTime = getTimedLoopTime()*NUMBER_OF_COLUMNS;
	uint8_t mod = 0;
	  
	if ((BUTTON_DEBOUNCE_TIME % rowPassTime) > 0)	mod = 1;

	numberOfColumnPasses = ((BUTTON_DEBOUNCE_TIME / rowPassTime) + mod);
	
	setButtonDebounceCompare();
	

}
 
void OpenDeck::setButtonDebounceCompare()	{
	
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
	uint8_t rowState = HardwareReadSpecific::readButtons();
	
	//iterate over rows
	for (int i=0; i<NUMBER_OF_BUTTON_ROWS; i++) {
		
		//extract current bit from rowState variable
		//invert extracted bit because of pull-up resistors
		uint8_t currentBit = !((rowState >> i) & 0x01);
		
		//calculate current button number
		uint8_t buttonNumber = getActiveColumn()+i*NUMBER_OF_COLUMNS;
		
		//get button state
		buttonState = checkButton(currentBit, previousButtonState[buttonNumber]);
		
		//if current button status is different from previous
		if (buttonState != previousButtonState[buttonNumber]) {
			
			if (buttonState == 0xFF)	{	
				
				//button is pressed
				sendButtonDataCallback(buttonNumber, 1);
				
				#ifdef LONG_PRESS_TIME
					//start long press timer
					longPressState[buttonNumber] = millis();
				#endif
				
			}
				
				else if (buttonState == buttonDebounceCompare)	{
					
					//button is released
					
					#ifdef LONG_PRESS_TIME
						if (longPressSent[buttonNumber]) sendButtonDataCallback(buttonNumber, 3);
							else sendButtonDataCallback(buttonNumber, 0);
					#else
						sendButtonDataCallback(buttonNumber, 0);
					#endif
					
					#ifdef LONG_PRESS_TIME 
						longPressState[buttonNumber] = 0;
						longPressSent[buttonNumber] = false;
					#endif
					
				}
					
				//update previous reading with current
				previousButtonState[buttonNumber] = buttonState;
			
		}	
			
			#ifdef LONG_PRESS_TIME 
				if ((millis() - longPressState[buttonNumber] >= LONG_PRESS_TIME) && (!longPressSent[buttonNumber]) && (buttonState == 0xFF))	{
				
					sendButtonDataCallback(buttonNumber, 2);
					longPressSent[buttonNumber] = true;
					
			}
			#endif
		
	}

}

uint8_t OpenDeck::checkButton(uint8_t currentState, uint8_t previousState)  {
	
	uint8_t buttonState = previousState;
	
	buttonState = (buttonState << 1) | currentState | buttonDebounceCompare;
	
	return buttonState;

}

void OpenDeck::setHandleButton(void (*fptr)(uint8_t buttonNumber, uint8_t buttonState))	{
	
	sendButtonDataCallback = fptr;
	
}

#endif

//pots
#ifdef POTS

#ifdef MUX
bool OpenDeck::adcChannelMux(uint8_t adcChannel)	{
	
	switch (adcChannel)	{
		
		case 0:
		#ifdef ADC_CHANNEL_0_MUX
		return true;
		#else
		return false;
		#endif
		break;
		
		case 1:
		#ifdef ADC_CHANNEL_1_MUX
		return true;
		#else
		return false;
		#endif
		break;
		
		case 2:
		#ifdef ADC_CHANNEL_2_MUX
		return true;
		#else
		return false;
		#endif
		break;
		
		case 3:
		#ifdef ADC_CHANNEL_3_MUX
		return true;
		#else
		return false;
		#endif
		break;
		
		case 4:
		#ifdef ADC_CHANNEL_4_MUX
		return true;
		#else
		return false;
		#endif
		break;
		
		case 5:
		#ifdef ADC_CHANNEL_5_MUX
		return true;
		#else
		return false;
		#endif
		break;
		
		case 6:
		#ifdef ADC_CHANNEL_6_MUX
		return true;
		#else
		return false;
		#endif
		break;
		
		case 7:
		#ifdef ADC_CHANNEL_7_MUX
		return true;
		#else
		return false;
		#endif
		break;
		
		default:
		return false;
		break;
		
	}
	
}
#endif

bool OpenDeck::adcConnected(uint8_t adcChannel)	{
	
	switch (adcChannel)	{
		
		case 0:
		#if defined(ADC_CHANNEL_0_MUX) || defined (ADC_CHANNEL_0_POT)
		return true;
		#else
		return false;
		#endif
		break;
		
		case 1:
		#if defined(ADC_CHANNEL_1_MUX) || defined (ADC_CHANNEL_1_POT)
		return true;
		#else
		return false;
		#endif
		break;
		
		case 2:
		#if defined(ADC_CHANNEL_2_MUX) || defined (ADC_CHANNEL_2_POT)
		return true;
		#else
		return false;
		#endif
		break;
		
		case 3:
		#if defined(ADC_CHANNEL_3_MUX) || defined (ADC_CHANNEL_3_POT)
		return true;
		#else
		return false;
		#endif
		break;
		
		case 4:
		#if defined(ADC_CHANNEL_4_MUX) || defined (ADC_CHANNEL_4_POT)
		return true;
		#else
		return false;
		#endif
		break;
		
		case 5:
		#if defined(ADC_CHANNEL_5_MUX) || defined (ADC_CHANNEL_5_POT)
		return true;
		#else
		return false;
		#endif
		break;
		
		case 6:
		#if defined(ADC_CHANNEL_6_MUX) || defined (ADC_CHANNEL_6_POT)
		return true;
		#else
		return false;
		#endif
		break;
		
		case 7:
		#if defined(ADC_CHANNEL_7_MUX) || defined (ADC_CHANNEL_7_POT)
		return true;
		#else
		return false;
		#endif
		break;
		
		default:
		return false;
		break;
		
	}
	
}

void OpenDeck::readPots()	{
	
	potNumber = 0;
	
	for (int i=0; i<8; i++)	{
		
		#if defined (AT_POTS) && defined (MUX)
		
		if (adcConnected(i))	{
			
			if (adcChannelMux(i))	readPotsMux(i);
				else	readPotsATmega(i);
			
		}
		
		#elif defined (AT_POTS)
				
		if (adcConnected(i))
			readPotsATmega(i);
					
		#elif defined (MUX)
				
			if (adcConnected(i))
				readPotsMux(i);
		
		#endif
	
	}

}

#ifdef MUX
void OpenDeck::readPotsMux(uint8_t adcChannel) {

	for (int j=0; j<8; j++)	{
		
		HardwareReadSpecific::setMuxOutput(j);

		//read analogue value from mux
		int16_t tempValue = analogRead(adcChannel);

		//if new reading is stable send new MIDI message
		checkPotReading(tempValue, potNumber);
			
		//calculate pot number
		potNumber++;

	}

}
#endif

#ifdef AT_POTS
void OpenDeck::readPotsATmega(uint8_t adcChannel) {

		//read analogue value from mux
		int16_t tempValue = analogRead(adcChannel);

		//if new reading is stable send new MIDI message
		checkPotReading(tempValue, potNumber);
		
		//calculate pot number
		potNumber++;


}
#endif

void OpenDeck::checkPotReading(int16_t currentValue, uint8_t potNumber)	{
	
	//calculate difference between current and previous reading
	int8_t analogueDiff = currentValue - lastAnalogueValue[potNumber];
	if (analogueDiff < 0)	analogueDiff *= -1;
	
	uint32_t timeDifference = (millis() - potTimer[potNumber]);
	
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
				
	#ifdef INVERT_ANALOGUE_VALUE
		ccValue = 127 - (tempValue >> 3);
	#else
		ccValue = tempValue >> 3;
	#endif
				
	if (sendPotCCDataCallback != NULL)	sendPotCCDataCallback(potNumber, ccValue);
			
	#ifdef ENABLE_POT_NOTE_EVENTS
				
		uint8_t noteCurrent = getPotNoteValue(ccValue, potNumber);
				
			if (checkPotNoteValue(potNumber, noteCurrent))	{
					
				if ((lastPotNoteValue[potNumber] != 128) && (sendPotNoteOffDataCallback != NULL))
					sendPotNoteOffDataCallback(lastPotNoteValue[potNumber]);
				
				if (sendPotNoteOnDataCallback != NULL) sendPotNoteOnDataCallback(noteCurrent);
					
				lastPotNoteValue[potNumber] = noteCurrent;;
					
			}
					
	#endif

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

void OpenDeck::setHandlePotCC(void (*fptr)(uint8_t potNumber, uint8_t ccValue))	{
	
	sendPotCCDataCallback = fptr;
	
}

void OpenDeck::setHandlePotNoteOn(void (*fptr)(uint8_t note))	{
	
	sendPotNoteOnDataCallback = fptr;
	
}

void OpenDeck::setHandlePotNoteOff(void (*fptr)(uint8_t note))	{
	
	sendPotNoteOffDataCallback = fptr;
	
}

#endif


//LEDs
#ifdef LED_MATRIX
void OpenDeck::checkLEDs()  {
	
	//get currently active column
	uint8_t currentColumn = getActiveColumn();
	
	if (blinkEnabled)	switchBlinkState();
	
	//if there is an active LED in current column, turn on LED row
	for (int i=0; i<NUMBER_OF_LED_ROWS; i++)
	
	if (ledOn(currentColumn+i*NUMBER_OF_COLUMNS))	HardwareReadSpecific::ledRowOn(i);

}

bool OpenDeck::checkLEDsOn()	{

	//return true if all LEDs are on
	for (int i=0; i<TOTAL_NUMBER_OF_LEDS; i++)	if (ledState[i] != 0)	return false;
	return true;

}

bool OpenDeck::checkLEDsOff()	{

	//return true if all LEDs are off
	for (int i=0; i<TOTAL_NUMBER_OF_LEDS; i++)	if (ledState[i] == 0)	return false;
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
	for (int i=0; i<TOTAL_NUMBER_OF_LEDS; i++)  setConstantLEDstate(i);
	
}

void OpenDeck::allLEDsOff()  {
	
	//turn off all LEDs
	for (int i=0; i<TOTAL_NUMBER_OF_LEDS; i++)  ledState[i] = 0x00;
	
}

void OpenDeck::checkBlinkLEDs() {
	
	//this function will disable blinking
	//if none of the LEDs is in blinking state
	
	//else it will enable it
	
	bool _blinkEnabled = false;
	
	//if any LED is blinking, set timerState to true and exit the loop
	for (int i=0; i<TOTAL_NUMBER_OF_LEDS; i++) if (checkBlinkState(i))  { _blinkEnabled = true; break; }
	
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

	/*
	
	This part of code deals with situations when previous function call has been
	left direction and current one is right and vice versa.
	
	On first function call, let's assume the direction was left to right. That would mean
	that LEDs had to be processed in this order:
	
	LED 1
	LED 2
	LED 3
	LED 4
	
	Now, when function is finished, LEDs are not reset yet with allLEDsOff() function to keep
	track of their previous states. Next function call is right to left. On first run with 
	right to left direction, the LED order would be standard LED 4 to LED 1, however, LED 4 has
	been already turned on by first function call, so we check if its state is already set, and if
	it is we increment or decrement ledNumber by one, depending on previous and current direction.
	When function is called second time with direction different than previous one, the number of 
	times it needs to execute is reduced by one, therefore passCounter is incremented.
	
	*/
    
    //right-to-left direction
    if (!ledDirection)
    //if last LED is turned on
	if (ledOn(ledArray[TOTAL_NUMBER_OF_LEDS-1]))	{

	    //LED index is penultimate LED number
	    ledNumber = ledArray[TOTAL_NUMBER_OF_LEDS-2];
	    //increment counter since the loop has to run one cycle less
	    passCounter++;

    }
    
	//led index is last one if last one isn't already on
	else ledNumber = ledArray[TOTAL_NUMBER_OF_LEDS-1];

	//left-to-right direction
	else
	//if first LED is already on
	if (ledOn(ledArray[0]))	{

		//led index is 1
		ledNumber = ledArray[1];
		//increment counter
		passCounter++;

	}

	else ledNumber = ledArray[0];
      
	}
    
    else  {

	/*
	
	This is situation when all LEDs are turned on and we're turning them off one by one. Same
	logic applies in both cases (see above). In this case we're not checking for whether the LED
	is already turned on, but whether it's already turned off.
	
	*/
      
    //right-to-left direction
    if (!ledDirection)	if (!(ledOn(ledArray[TOTAL_NUMBER_OF_LEDS-1])))	{

		ledNumber = ledArray[TOTAL_NUMBER_OF_LEDS-2];
		passCounter++;

	}
    
	else ledNumber = ledArray[TOTAL_NUMBER_OF_LEDS-1];
         
    //left-to-right direction
    else  if (!(ledOn(ledArray[0])))   {

		ledNumber = ledArray[1];
		passCounter++;

	}

	else ledNumber = ledArray[0];
      
    }

    //on first function call, the while loop is called TOTAL_NUMBER_OF_LEDS+1 times
	//to get empty cycle after processing last LED
	while (passCounter < TOTAL_NUMBER_OF_LEDS+1)  {	
      
		if ((millis() - columnTime) > getTimedLoopTime())  {   
    
			//activate next column
			nextColumn();

			//only process LED after defined time
			if ((millis() - startUpTimer) > START_UP_LED_SWITCH_TIME)  {
  
				if (passCounter < TOTAL_NUMBER_OF_LEDS)  {
  
					//if we're turning LEDs on one by one, turn all the other LEDs off
					if (singleLED && turnOn)  allLEDsOff(); 
                                        
                    //if we're turning LEDs off one by one, turn all the other LEDs on
					else if (!turnOn && singleLED)	allLEDsOn();
                                        
                    //set LED state depending on turnOn parameter
					if (turnOn)  turnOnLED(ledNumber);
						else turnOffLED(ledNumber);

					//make sure out-of-bound index isn't requested from ledArray
				    if (passCounter < TOTAL_NUMBER_OF_LEDS-1)	{

						//right-to-left direction
						if (!ledDirection)	ledNumber = ledArray[TOTAL_NUMBER_OF_LEDS - 2 - passCounter];

						//left-to-right direction
						else	if (passCounter < TOTAL_NUMBER_OF_LEDS-1)	ledNumber = ledArray[passCounter+1];

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
	
	if ((millis() - blinkTimerCounter) >= BLINK_DURATION)	{
		
		//change blinkBit state and write it into ledState variable if LED is in blink state
		for (int i = 0; i<TOTAL_NUMBER_OF_LEDS; i++)
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

void OpenDeck::handleLED(uint8_t ledNumber, bool currentLEDstate, bool blinkMode)   {

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
	
	//ignore every note bigger than TOTAL_NUMBER_OF_LEDS*2 to avoid out-of-bound request
	if (receivedNotePitch < TOTAL_NUMBER_OF_LEDS*2)	{
		
		bool currentLEDstate;
		uint8_t ledNumber;
		
		//if blinkMode is 1, the LED is blinking
		uint8_t blinkMode = 0;
		
		//if velocity is 0, turn off LED
		//else turn it on
		if (receivedNoteVelocity != 0)  currentLEDstate = true;
		else currentLEDstate = false;
		
		if (receivedNotePitch >= TOTAL_NUMBER_OF_LEDS)  blinkMode = 1;
		
		//it's important to get the same led number in either case
		ledNumber = ledArray[receivedNotePitch - (TOTAL_NUMBER_OF_LEDS*blinkMode)];
		
		handleLED(ledNumber, currentLEDstate, blinkMode);
		
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

#endif

//columns

void OpenDeck::nextColumn()   {
	
	if (column == NUMBER_OF_COLUMNS)	column = 0;
	
	HardwareReadSpecific::activateColumn(column);
	
	//increment column
	column++;

}

uint8_t OpenDeck::getActiveColumn() {

	//return currently active column
	return (column - 1);

}

//create instance of library automatically
OpenDeck openDeck;