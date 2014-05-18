/*

Hardware specific library for reading data from multiple sources
v1.0
Last revision date: 2014-05-17
Author: Igor Petrovic

*/

#include "HardwareReadSpecific.h"
#include "Ownduino.h"
#include <avr/io.h>

void HardwareReadSpecific::initPins() {}

void HardwareReadSpecific::activateColumn(uint8_t column)  {
	
	//turn off all LED rows before switching to next column
	#ifdef LED_MATRIX
		ledRowsOff();
	#endif
	
}


#ifdef LED_MATRIX

void HardwareReadSpecific::ledRowOn(uint8_t rowNumber)  {}

void HardwareReadSpecific::ledRowOff(uint8_t rowNumber) {}

void HardwareReadSpecific::ledRowsOn()	{
	
	//turn all LED rows on
	
}

void HardwareReadSpecific::ledRowsOff()	{
	
	//turn all LED rows off
	
}

#endif


#ifdef BUTTON_MATRIX

uint8_t HardwareReadSpecific::readButtons()	{

	//get the readings from all the rows
	
	/*
		
		All readings must fit in a single byte variable, which suggests that there can be maximum of
		eight rows. All row readings must be in right-to-left order in return variable without empty
		readings between rows, otherwise OpenDeck library will send wrong data.
	
	*/

}

#endif


#ifdef NUMBER_OF_MUX

void HardwareReadSpecific::setMuxOutput(uint8_t muxInput)	{

	//switch to next mux input
	
	
	//add a short delay after switching
	NOP;

}

#endif