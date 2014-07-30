/*

Ownduino v0.1

Author: Igor Petrovic
Last edit date: 2014-05-01

*/

#include "Ownduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>

int main(void)	{
	
	init();
	setup();
	
	while (1) {
		
		loop();
		
		#ifdef TIMED_LOOP
		if (checkTimedLoopSwitch())	{
			
			timedLoop();
			updateTimedLoopTime();
			
		}
		#endif
		
	}
	
}