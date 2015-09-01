/*

OpenDeck platform firmware v2.0
Last revision date: 2015-05-08
Author: Igor Petrovic

*/

#include "OpenDeck.h"

#ifdef BOARD_OPENDECK_1

//atmega328
//arduino adds some unnecessary stuff in setup/loop

int main()  {

    openDeck.init();

    while(1)    {

        //check incoming MIDI messages
        openDeck.checkMIDIIn();

        //check buttons
        openDeck.readButtons();

        //check analog inputs
        openDeck.readAnalog();

        //check encoders
        openDeck.readEncoders();

    }   return 0;

}
#else

void setup()    {

    //setup
    openDeck.init();

}

void loop() {

    //check incoming MIDI messages
    openDeck.checkMIDIIn();

    //check buttons
    openDeck.readButtons();

    //check analog inputs
    openDeck.readAnalog();

    //check encoders
    openDeck.readEncoders();

    #ifdef SERIAL_MOD
        uart.releaseTX();
    #endif

}
#endif