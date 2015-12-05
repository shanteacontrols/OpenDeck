/*

    OpenDeck MIDI platform firmware
    https://shanteacontrols.wordpress.com
    Copyright (c) 2015 Igor Petrovic
    Version: 1.0

    Released under GPLv3.

*/

#include "hardware/Board.h"
#include "Buttons.h"
#include "Analog.h"
#include "Encoders.h"
#include "LEDs.h"
#include "eeprom/EEPROMsettings.h"
#include "sysex/ProtocolDefinitions.h"
#include "sysex/SysEx.h"
#include "hardware/Reset.h"

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

        case SYS_EX_MT_MIDI:
        return midi.getParameter(messageSubtype, parameter);
        break;

        case SYS_EX_MT_BUTTONS:
        return buttons.getParameter(messageSubtype, parameter);
        break;

        case SYS_EX_MT_ANALOG:
        return analog.getParameter(messageSubtype, parameter);
        break;

        case SYS_EX_MT_LEDS:
        return leds.getParameter(messageSubtype, parameter);
        break;

        case SYS_EX_MT_ENCODERS:
        return encoders.getParameter(messageSubtype, parameter);
        break;

    } return INVALID_VALUE;

}

bool onSet(uint8_t messageType, uint8_t messageSubtype, uint8_t parameter, uint8_t newParameter)   {

    switch(messageType) {

        case SYS_EX_MT_MIDI:
        return midi.setParameter(messageSubtype, parameter, newParameter);
        break;

        case SYS_EX_MT_BUTTONS:
        return buttons.setParameter(messageSubtype, parameter, newParameter);
        break;

        case SYS_EX_MT_ANALOG:
        return analog.setParameter(messageSubtype, parameter, newParameter);
        break;

        case SYS_EX_MT_LEDS:
        return leds.setParameter(messageSubtype, parameter, newParameter);
        break;

        case SYS_EX_MT_ENCODERS:
        return encoders.setParameter(messageSubtype, parameter, newParameter);
        break;

    }   return false;

}

bool onReset(uint8_t messageType, uint8_t messageSubtype, uint8_t parameter) {

    switch(messageType) {

        case SYS_EX_MT_MIDI:
        return midi.setParameter(messageSubtype, parameter, RESET_VALUE);
        break;

        case SYS_EX_MT_BUTTONS:
        return buttons.setParameter(messageSubtype, parameter, RESET_VALUE);
        break;

        case SYS_EX_MT_ANALOG:
        return analog.setParameter(messageSubtype, parameter, RESET_VALUE);
        break;

        case SYS_EX_MT_LEDS:
        return leds.setParameter(messageSubtype, parameter, RESET_VALUE);
        break;

        case SYS_EX_MT_ENCODERS:
        return encoders.setParameter(messageSubtype, parameter, RESET_VALUE);
        break;

    }   return false;

}

void onSysEx(uint8_t sysExArray[], uint8_t arraySize)   {

    sysEx.handleSysEx(sysExArray, arraySize);

}

void onNote(uint8_t note, uint8_t noteVelocity) {

    //we're using received note data to control LEDs
    leds.noteToLEDstate(note, noteVelocity);

}

void setup()    {

    eepromSettings.init();

    sysEx.setHandleReboot(onReboot);
    sysEx.setHandleGet(onGet);
    sysEx.setHandleSet(onSet);
    sysEx.setHandleReset(onReset);

    midi.setHandleSysEx(onSysEx);
    midi.setHandleNote(onNote);

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