/*

OpenDECK library v0.99
File: EEPROM.cpp
Last revision date: 2014-09-15
Author: Igor Petrovic

*/ 

#include "OpenDeck.h"
#include <avr/eeprom.h>
#include "Ownduino.h"

//configuration retrieve

//global configuration getter
void OpenDeck::getConfiguration()   {

    //get configuration from EEPROM
    getMIDIchannels();
    getHardwareParams();
    getFreePinStates();
    getSoftwareFeatures();
    getHardwareFeatures();
    getEnabledPots();
    getPotInvertStates();
    getCCnumbers();
    getButtonsType();
    getButtonNotes();
    getLEDnotes();

}

//individual configuration getters
void OpenDeck::getMIDIchannels()        {

    _buttonNoteChannel          = eeprom_read_byte((uint8_t*)EEPROM_MC_BUTTON_NOTE);
    _longPressButtonNoteChannel = eeprom_read_byte((uint8_t*)EEPROM_MC_LONG_PRESS_BUTTON_NOTE);
    _potCCchannel               = eeprom_read_byte((uint8_t*)EEPROM_MC_POT_CC);
    _encCCchannel               = eeprom_read_byte((uint8_t*)EEPROM_MC_ENC_CC);
    _inputChannel               = eeprom_read_byte((uint8_t*)EEPROM_MC_INPUT);

}

void OpenDeck::getHardwareParams()      {

    _board                      = eeprom_read_byte((uint8_t*)EEPROM_HW_P_BOARD_TYPE);
    _longPressTime              = eeprom_read_byte((uint8_t*)EEPROM_HW_P_LONG_PRESS_TIME) * 100;
    _blinkTime                  = eeprom_read_byte((uint8_t*)EEPROM_HW_P_BLINK_TIME) * 100;
    totalNumberOfLEDs           = eeprom_read_byte((uint8_t*)EEPROM_HW_P_TOTAL_LED_NUMBER);
    _startUpLEDswitchTime       = eeprom_read_byte((uint8_t*)EEPROM_HW_P_START_UP_SWITCH_TIME) * 10;
    startUpRoutinePattern       = eeprom_read_byte((uint8_t*)EEPROM_HW_P_START_UP_ROUTINE);

}

void OpenDeck::getFreePinStates()       {

    uint16_t eepromAddress = EEPROM_FREE_PIN_START;

    for (int i=0; i<NUMBER_OF_FREE_PINS; i++)   {

        freePinState[i] = eeprom_read_byte((uint8_t*)eepromAddress);
        eepromAddress++;

    }

}

void OpenDeck::getSoftwareFeatures()    {

    softwareFeatures = eeprom_read_byte((uint8_t*)EEPROM_SOFTWARE_FEATURES_START);

}

void OpenDeck::getHardwareFeatures()    {

    hardwareFeatures = eeprom_read_byte((uint8_t*)EEPROM_HARDWARE_FEATURES_START);

}

void OpenDeck::getButtonsType()         {

    uint16_t eepromAddress = EEPROM_BUTTON_TYPE_START;

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS/8; i++)   {

        buttonType[i] = eeprom_read_byte((uint8_t*)eepromAddress);
        eepromAddress++;

    }

}

void OpenDeck::getButtonNotes()         {

    uint16_t eepromAddress = EEPROM_BUTTON_NOTE_START;

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++) {

        buttonNote[i] = eeprom_read_byte((uint8_t*)eepromAddress);
        eepromAddress++;

    }

}

void OpenDeck::getEnabledPots()         {

    uint16_t eepromAddress = EEPROM_POT_ENABLED_START;

    for (int i=0; i<(MAX_NUMBER_OF_POTS/8); i++)    {

        potEnabled[i] = eeprom_read_byte((uint8_t*)eepromAddress);
        eepromAddress++;

    }

}

void OpenDeck::getPotInvertStates()     {

    uint16_t eepromAddress = EEPROM_POT_INVERSION_START;

    for (int i=0; i<(MAX_NUMBER_OF_POTS/8); i++)    {

        potInverted[i] = eeprom_read_byte((uint8_t*)eepromAddress);
        eepromAddress++;

    }

}

void OpenDeck::getCCnumbers()           {

    uint16_t eepromAddress = EEPROM_POT_CC_NUMBER_START;

    for (int i=0; i<MAX_NUMBER_OF_POTS; i++)    {

        ccNumber[i] = eeprom_read_byte((uint8_t*)eepromAddress);
        eepromAddress++;

    }

}

void OpenDeck::getLEDnotes()            {

    uint16_t eepromAddress = EEPROM_LED_ACT_NOTE_START;

    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    {

        ledNote[i] = eeprom_read_byte((uint8_t*)eepromAddress);
        eepromAddress++;

    }

}