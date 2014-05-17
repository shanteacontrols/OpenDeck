/*

Hardware specific library for reading data from multiple sources
v1.0
Last revision date: 2014-05-17
Author: Igor Petrovic

*/

#include "HardwareReadSpecific.h"
#include "Ownduino.h"
#include <avr/io.h>

HardwareReadSpecific::HardwareReadSpecific()    {

  //initialization

}

void HardwareReadSpecific::initPins() {
	
	//set in/out pins

}

void HardwareReadSpecific::activateColumn(uint8_t column)  {
	
	#ifdef LED_MATRIX
		ledRowsOff();
	#endif
	
	//there can only be one column active at the time, the rest is set to HIGH
	switch (column)  {
		
		case 0:
		break;
		
		case 1:
		break;
		
		case 2:
		break;
		
		case 3:
		break;
		
		case 4:
		break;
		
		case 5:
		break;
		
		case 6:
		break;
		
		case 7:
		break;
		
		default:
		break;
		
	}
	
}


#ifdef LED_MATRIX

void HardwareReadSpecific::ledRowOn(uint8_t rowNumber)  {

	switch (rowNumber)	{
		
		case 0:
		//turn on first LED row
		break;
		
		default:
		break;
		
	}

}

void HardwareReadSpecific::ledRowOff(uint8_t rowNumber) {

	switch (rowNumber)	{
		
		case 0:
		//turn off first LED row
		break;
		
		default:
		break;
		
	}

}

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


#ifdef MUX

void HardwareReadSpecific::setMuxOutput(uint8_t muxInput)	{

	//switch to next mux input
	
	
	//add a short delay after switching
	NOP;

}

#endif