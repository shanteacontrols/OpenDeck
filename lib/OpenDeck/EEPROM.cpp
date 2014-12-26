/*

OpenDECK library v1.3
File: EEPROM.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/ 

#include "OpenDeck.h"
#include <avr/eeprom.h>
#include "Ownduino.h"


//global configuration getter
void OpenDeck::getConfiguration()   {

    //get configuration from EEPROM
    getHardwareConfig();
    getFeatures();
    getMIDIchannels();
    getButtonsType();
    getPPenabledButtons();
    getButtonNotes();
    getEnabledPots();
    getPPenabledPots();
    getPotInvertStates();
    getCCPPnumbers();
    getCCPPlowerLimits();
    getCCPPupperLimits();
    getLEDnotes();
    getLEDHwParameters();

}

//individual configuration getters
void OpenDeck::getHardwareConfig()        {

    _board                      = eeprom_read_byte((uint8_t*)EEPROM_BOARD_TYPE);
    hardwareEnabled             = eeprom_read_byte((uint8_t*)EEPROM_HARDWARE_ENABLED);

}

void OpenDeck::getFeatures()    {

    midiFeatures                = eeprom_read_byte((uint8_t*)EEPROM_FEATURES_MIDI);
    buttonFeatures              = eeprom_read_byte((uint8_t*)EEPROM_FEATURES_BUTTONS);
    ledFeatures                 = eeprom_read_byte((uint8_t*)EEPROM_FEATURES_LEDS);
    potFeatures                 = eeprom_read_byte((uint8_t*)EEPROM_FEATURES_POTS);

}

void OpenDeck::getMIDIchannels()        {

    _buttonNoteChannel          = eeprom_read_byte((uint8_t*)EEPROM_MC_BUTTON_NOTE);
    _longPressButtonNoteChannel = eeprom_read_byte((uint8_t*)EEPROM_MC_LONG_PRESS_BUTTON_NOTE);
    _buttonPPchannel            = eeprom_read_byte((uint8_t*)EEPROM_MC_BUTTON_PP);
    _potCCchannel               = eeprom_read_byte((uint8_t*)EEPROM_MC_POT_CC);
    _potPPchannel               = eeprom_read_byte((uint8_t*)EEPROM_MC_POT_PP);
    _potNoteChannel             = eeprom_read_byte((uint8_t*)EEPROM_MC_POT_NOTE);
    _inputChannel               = eeprom_read_byte((uint8_t*)EEPROM_MC_INPUT);

}

void OpenDeck::getButtonsType()         {

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS/8; i++)
        buttonType[i] = eeprom_read_byte((uint8_t*)EEPROM_BUTTON_TYPE_START+i);

}

void OpenDeck::getPPenabledButtons()         {

    for (int i=0; i<(MAX_NUMBER_OF_BUTTONS/8); i++)
        buttonPPenabled[i] = eeprom_read_byte((uint8_t*)EEPROM_BUTTON_PP_ENABLED_START+i);

}

void OpenDeck::getButtonNotes()         {

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
        buttonNote[i] = eeprom_read_byte((uint8_t*)EEPROM_BUTTON_NOTE_START+i);

}

void OpenDeck::getEnabledPots()         {

    for (int i=0; i<(MAX_NUMBER_OF_POTS/8); i++)
        potEnabled[i] = eeprom_read_byte((uint8_t*)EEPROM_POT_ENABLED_START+i);

}

void OpenDeck::getPPenabledPots()         {

    for (int i=0; i<(MAX_NUMBER_OF_POTS/8); i++)
        potPPenabled[i] = eeprom_read_byte((uint8_t*)EEPROM_POT_PP_ENABLED_START+i);

}

void OpenDeck::getPotInvertStates()     {

    for (int i=0; i<(MAX_NUMBER_OF_POTS/8); i++)
        potInverted[i] = eeprom_read_byte((uint8_t*)EEPROM_POT_INVERSION_START+i);

}

void OpenDeck::getCCPPnumbers()           {

    for (int i=0; i<MAX_NUMBER_OF_POTS; i++)
        ccppNumber[i] = eeprom_read_byte((uint8_t*)EEPROM_POT_CC_PP_NUMBER_START+i);

}

void OpenDeck::getCCPPlowerLimits()   {

    for (int i=0; i<MAX_NUMBER_OF_POTS; i++)
        ccLowerLimit[i] = eeprom_read_byte((uint8_t*)EEPROM_POT_LOWER_LIMIT_START+i);

}

void OpenDeck::getCCPPupperLimits()   {

    for (int i=0; i<MAX_NUMBER_OF_POTS; i++)
        ccUpperLimit[i] = eeprom_read_byte((uint8_t*)EEPROM_POT_UPPER_LIMIT_START+i);

}

void OpenDeck::getLEDnotes()            {

    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        ledActNote[i] = eeprom_read_byte((uint8_t*)EEPROM_LED_ACT_NOTE_START+i);

}

void OpenDeck::getLEDHwParameters() {

    _blinkTime          = eeprom_read_byte((uint8_t*)EEPROM_LED_HW_P_BLINK_TIME)*100;
    totalNumberOfLEDs   = eeprom_read_byte((uint8_t*)EEPROM_LED_HW_P_TOTAL_NUMBER);

}