/*

OpenDECK library v1.1
File: EEPROM.cpp
Last revision date: 2014-10-02
Author: Igor Petrovic

*/ 

#include "OpenDeck.h"
#include <avr/eeprom.h>
#include "Ownduino.h"


//global configuration getter
void OpenDeck::getConfiguration()   {

    //get configuration from EEPROM
    getMIDIchannels();
    getHardwareParams();
    getFreePinStates();
    getSoftwareFeatures();
    getHardwareFeatures();
    getButtonsType();
    getButtonNotes();
    getEnabledPots();
    getPotInvertStates();
    getCCnumbers();
    getCClowerLimits();
    getCCupperLimits();
    getEncoderPairs();
    getLEDnotes();

    if (_board != 0)    initBoard();
    if (freePinConfEn)  configureFreePins();

}

//individual configuration getters
void OpenDeck::getMIDIchannels()        {

    _buttonNoteChannel          = eeprom_read_byte((uint8_t*)EEPROM_MC_BUTTON_NOTE);
    _longPressButtonNoteChannel = eeprom_read_byte((uint8_t*)EEPROM_MC_LONG_PRESS_BUTTON_NOTE);
    _potCCchannel               = eeprom_read_byte((uint8_t*)EEPROM_MC_POT_CC);
    _potNoteChannel             = eeprom_read_byte((uint8_t*)EEPROM_MC_POT_NOTE);
    _encCCchannel               = eeprom_read_byte((uint8_t*)EEPROM_MC_ENC_CC);
    _inputChannel               = eeprom_read_byte((uint8_t*)EEPROM_MC_INPUT);

}

void OpenDeck::getHardwareParams()      {

    _board                      = eeprom_read_byte((uint8_t*)EEPROM_HW_P_BOARD_TYPE);
    _blinkTime                  = eeprom_read_byte((uint8_t*)EEPROM_HW_P_BLINK_TIME) * 100;
    totalNumberOfLEDs           = eeprom_read_byte((uint8_t*)EEPROM_HW_P_TOTAL_LED_NUMBER);

}

void OpenDeck::getFreePinStates()       {

    for (int i=0; i<SYS_EX_FREE_PIN_END; i++)
        freePinState[i] = eeprom_read_byte((uint8_t*)EEPROM_FREE_PIN_START+i);

}

void OpenDeck::getSoftwareFeatures()    {

    softwareFeatures = eeprom_read_byte((uint8_t*)EEPROM_SOFTWARE_FEATURES_START);

}

void OpenDeck::getHardwareFeatures()    {

    hardwareFeatures = eeprom_read_byte((uint8_t*)EEPROM_HARDWARE_FEATURES_START);

}

void OpenDeck::getButtonsType()         {

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS/8; i++)
        buttonType[i] = eeprom_read_byte((uint8_t*)EEPROM_BUTTON_TYPE_START+i);

}

void OpenDeck::getButtonNotes()         {

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
        buttonNote[i] = eeprom_read_byte((uint8_t*)EEPROM_BUTTON_NOTE_START+i);

}

void OpenDeck::getEnabledPots()         {

    for (int i=0; i<(MAX_NUMBER_OF_POTS/8); i++)
        potEnabled[i] = eeprom_read_byte((uint8_t*)EEPROM_POT_ENABLED_START+i);

}

void OpenDeck::getPotInvertStates()     {

    for (int i=0; i<(MAX_NUMBER_OF_POTS/8); i++)
        potInverted[i] = eeprom_read_byte((uint8_t*)EEPROM_POT_INVERSION_START+i);

}

void OpenDeck::getCCnumbers()           {

    for (int i=0; i<MAX_NUMBER_OF_POTS; i++)
        ccNumber[i] = eeprom_read_byte((uint8_t*)EEPROM_POT_CC_NUMBER_START+i);

}

void OpenDeck::getCClowerLimits()   {

    for (int i=0; i<MAX_NUMBER_OF_POTS; i++)
        ccLowerLimit[i] = eeprom_read_byte((uint8_t*)EEPROM_POT_LOWER_LIMIT_START+i);

}

void OpenDeck::getCCupperLimits()   {

    for (int i=0; i<MAX_NUMBER_OF_POTS; i++)
        ccUpperLimit[i] = eeprom_read_byte((uint8_t*)EEPROM_POT_UPPER_LIMIT_START+i);

}

void OpenDeck::getEncoderPairs()    {

    for (int i=0; i<MAX_NUMBER_OF_ENCODERS/8; i++)
        encoderPairEnabled[i] = eeprom_read_byte((uint8_t*)EEPROM_ENCODER_START+i);

}

void OpenDeck::getLEDnotes()            {

    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        ledNote[i] = eeprom_read_byte((uint8_t*)EEPROM_LED_ACT_NOTE_START+i);

}