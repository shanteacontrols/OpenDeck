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
    getFeatures();
    getMIDIchannels();
    getButtonsType();
    getButtonsPCenabled();
    getButtonsNotes();
    getAnalogEnabled();
    getAnalogType();
    getAnalogInversion();
    getAnalogNumbers();
    getAnalogLowerLimits();
    getAnalogUpperLimits();
    getEncodersEnabled();
    getEncodersInverted();
    getEncodersFastMode();
    getEncodersNumbers();
    getEncodersPulsesPerStep();
    getLEDActivationNotes();
    getLEDHwParameters();

}

//individual configuration getters

void OpenDeck::getFeatures()    {

    midiFeatures                = eeprom_read_byte((uint8_t*)EEPROM_FEATURES_MIDI);
    buttonFeatures              = eeprom_read_byte((uint8_t*)EEPROM_FEATURES_BUTTONS);
    ledFeatures                 = eeprom_read_byte((uint8_t*)EEPROM_FEATURES_LEDS);
    analogFeatures              = eeprom_read_byte((uint8_t*)EEPROM_FEATURES_POTS);
    encoderFeatures             = eeprom_read_byte((uint8_t*)EEPROM_FEATURES_ENCODERS);

}

void OpenDeck::getMIDIchannels()        {

    _buttonNoteChannel          = eeprom_read_byte((uint8_t*)EEPROM_MC_BUTTON_NOTE);
    _programChangeChannel       = eeprom_read_byte((uint8_t*)EEPROM_MC_PROGRAM_CHANGE);
    _analogCCchannel            = eeprom_read_byte((uint8_t*)EEPROM_MC_CC);
    _pitchBendChannel           = eeprom_read_byte((uint8_t*)EEPROM_MC_PITCH_BEND);
    _inputChannel               = eeprom_read_byte((uint8_t*)EEPROM_MC_INPUT);

}

void OpenDeck::getButtonsType()         {

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS/8; i++)
        buttonType[i] = eeprom_read_byte((uint8_t*)EEPROM_BUTTONS_TYPE_START+i);

}

void OpenDeck::getButtonsPCenabled()         {

    for (int i=0; i<(MAX_NUMBER_OF_BUTTONS/8); i++)
        buttonPCenabled[i] = eeprom_read_byte((uint8_t*)EEPROM_BUTTONS_PC_ENABLED_START+i);

}

void OpenDeck::getButtonsNotes()         {

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
        buttonNote[i] = eeprom_read_byte((uint8_t*)EEPROM_BUTTONS_NOTE_START+i);

}

void OpenDeck::getAnalogEnabled()         {

    for (int i=0; i<(MAX_NUMBER_OF_ANALOG/8); i++)
        analogEnabled[i] = eeprom_read_byte((uint8_t*)EEPROM_ANALOG_ENABLED_START+i);

}

void OpenDeck::getAnalogType()         {

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analogType[i] = eeprom_read_byte((uint8_t*)EEPROM_ANALOG_TYPE_START+i);

}

void OpenDeck::getAnalogInversion()     {

    for (int i=0; i<(MAX_NUMBER_OF_ANALOG/8); i++)
        analogInverted[i] = eeprom_read_byte((uint8_t*)EEPROM_ANALOG_INVERTED_START+i);

}

void OpenDeck::getAnalogNumbers()           {

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analogNumber[i] = eeprom_read_byte((uint8_t*)EEPROM_ANALOG_NUMBER_START+i);

}

void OpenDeck::getAnalogLowerLimits()   {

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analogLowerLimit[i] = eeprom_read_byte((uint8_t*)EEPROM_ANALOG_LOWER_LIMIT_START+i);

}

void OpenDeck::getAnalogUpperLimits()   {

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analogUpperLimit[i] = eeprom_read_byte((uint8_t*)EEPROM_ANALOG_UPPER_LIMIT_START+i);

}

void OpenDeck::getEncodersEnabled()  {

    for (int i=0; i<NUMBER_OF_ENCODERS; i++)
        encoderEnabled[i] = eeprom_read_byte((uint8_t*)EEPROM_ENCODERS_ENABLED_START+i);

}

void OpenDeck::getEncodersInverted()    {

    for (int i=0; i<NUMBER_OF_ENCODERS; i++)
        encoderInverted[i] = eeprom_read_byte((uint8_t*)EEPROM_ENCODERS_INVERTED_START+i);

}

void OpenDeck::getEncodersFastMode()    {

    for (int i=0; i<NUMBER_OF_ENCODERS; i++)
        encoderFastMode[i] = eeprom_read_byte((uint8_t*)EEPROM_ENCODERS_FAST_MODE_START+i);

}

void OpenDeck::getEncodersNumbers()  {

    for (int i=0; i<NUMBER_OF_ENCODERS; i++)
        encoderNumber[i] = eeprom_read_byte((uint8_t*)EEPROM_ENCODERS_NUMBER_START+i);

}

void OpenDeck::getEncodersPulsesPerStep()   {

    for (int i=0; i<NUMBER_OF_ENCODERS; i++)
        pulsesPerStep[i] = eeprom_read_byte((uint8_t*)EEPROM_ENCODERS_PULSES_PER_STEP_START+i);

}

void OpenDeck::getLEDActivationNotes()            {

    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        ledActNote[i] = eeprom_read_byte((uint8_t*)EEPROM_LEDS_ACT_NOTE_START+i);

}

void OpenDeck::getLEDHwParameters() {

    boardObject.setLEDblinkTime(eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_BLINK_TIME)*100);
    totalNumberOfLEDs   = eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_TOTAL_NUMBER);

}