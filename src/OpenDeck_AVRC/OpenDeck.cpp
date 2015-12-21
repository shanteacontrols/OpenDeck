/*
    OpenDeck MIDI platform firmware
    https://shanteacontrols.wordpress.com
    Copyright (c) 2015 Igor Petrovic
    Version: 1.0

    Released under GPLv3.
*/

#include "hardware\board\Board.h"
#include "interface\analog\Analog.h"
#include "interface\buttons\Buttons.h"
#include "interface\encoders\Encoders.h"
#include "interface\leds\LEDs.h"
#include "eeprom\Configuration.h"
#include "sysex\SysEx.h"
#include "hardware\reset\Reset.h"

void onReboot()  {

    //turn off all leds slowly before reseting
    board.setLEDTransitionSpeed(1);
    leds.allLEDsOff();
    //make sure all leds are off
    wait(1000);
    //this will reset the board into bootloader mode
    reboot();

}

uint8_t onGet(uint8_t messageType, uint8_t messageSubtype, uint8_t parameter) {

    switch(messageType) {

        case CONF_MIDI_BLOCK:
        return midi.getParameter(messageSubtype, parameter);
        break;

        case CONF_BUTTON_BLOCK:
        return buttons.getParameter(messageSubtype, parameter);
        break;

        case CONF_ENCODER_BLOCK:
        return encoders.getParameter(messageSubtype, parameter);
        break;

        case CONF_ANALOG_BLOCK:
        return analog.getParameter(messageSubtype, parameter);
        break;

        case CONF_LED_BLOCK:
        return leds.getParameter(messageSubtype, parameter);
        break;

    } return INVALID_VALUE;

}

bool onSet(uint8_t messageType, uint8_t messageSubtype, uint8_t parameter, uint8_t newParameter)   {

    switch(messageType) {

        case CONF_MIDI_BLOCK:
        return midi.setParameter(messageSubtype, parameter, newParameter);
        break;

        case CONF_BUTTON_BLOCK:
        return buttons.setParameter(messageSubtype, parameter, newParameter);
        break;

        case CONF_ENCODER_BLOCK:
        return encoders.setParameter(messageSubtype, parameter, newParameter);
        break;

        case CONF_ANALOG_BLOCK:
        return analog.setParameter(messageSubtype, parameter, newParameter);
        break;

        case CONF_LED_BLOCK:
        return leds.setParameter(messageSubtype, parameter, newParameter);
        break;

    }   return false;

}

bool onReset(uint8_t messageType, uint8_t messageSubtype, uint8_t parameter) {

    switch(messageType) {

        case CONF_MIDI_BLOCK:
        return midi.setParameter(messageSubtype, parameter, RESET_VALUE);
        break;

        case CONF_BUTTON_BLOCK:
        return buttons.setParameter(messageSubtype, parameter, RESET_VALUE);
        break;

        case CONF_ENCODER_BLOCK:
        return analog.setParameter(messageSubtype, parameter, RESET_VALUE);
        break;

        case CONF_LED_BLOCK:
        return leds.setParameter(messageSubtype, parameter, RESET_VALUE);
        break;

        case CONF_ANALOG_BLOCK:
        return encoders.setParameter(messageSubtype, parameter, RESET_VALUE);
        break;

    }   return false;

}

void setup()    {

    configuration.init();

    sysEx.setHandleReboot(onReboot);
    sysEx.setHandleGet(onGet);
    sysEx.setHandleSet(onSet);
    sysEx.setHandleReset(onReset);

    board.init();
    midi.init();
    buttons.init();
    leds.init();
    analog.init();
    encoders.init();

}

int main()  {

    setup();
    while(1) { midi.checkInput(); buttons.update(); analog.update(); encoders.update(); }
    return 0;

}