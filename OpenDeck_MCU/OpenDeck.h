/*

OpenDECK library v1.3
File: OpenDeck.h
Last revision date: 2014-12-25
Author: Igor Petrovic

*/


#ifndef OpenDeck_h
#define OpenDeck_h

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "Board.h"
#include "EEPROM.h"
#include "SysEx.h"
#include "MIDI.h"
#include "Types.h"
#include "Modules.h"
#include "Version.h"
#include "UniqueID.h"

class OpenDeck  {

    public:

    //constructor
    OpenDeck();

    //library initializer
    void init();

    //buttons
    void readButtons();

    //analog inputs
    void readAnalog();

    //encoders
    void readEncoders();

    //LEDs
    void allLEDsOn();
    void allLEDsOff();

    //MIDI in
    void checkMIDIIn();

    friend void storeReceivedNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    friend void processSysEx(uint8_t sysExArray[], uint8_t arrSize, sysExSource messageSource);

    private:

    //variables

    //features
    uint8_t         midiFeatures,
                    buttonFeatures,
                    ledFeatures,
                    analogFeatures,
                    encoderFeatures;

    //MIDI channels
    uint8_t         _noteChannel,
                    _programChangeChannel,
                    _CCchannel,
                    _pitchBendChannel,
                    _inputChannel;

    //buttons
    uint8_t         _buttonType[MAX_NUMBER_OF_BUTTONS/8+1],
                    buttonPCenabled[MAX_NUMBER_OF_BUTTONS/8+1],
                    noteNumber[MAX_NUMBER_OF_BUTTONS+1],
                    previousButtonState[MAX_NUMBER_OF_BUTTONS/8+1],
                    buttonPressed[MAX_NUMBER_OF_BUTTONS/8+1],
                    buttonDebounceCounter[MAX_NUMBER_OF_BUTTONS];

    //analog
    uint8_t         analogEnabled[MAX_NUMBER_OF_ANALOG/8+1],
                    analogType[MAX_NUMBER_OF_ANALOG],
                    analogInverted[MAX_NUMBER_OF_ANALOG/8+1],
                    ccNumber[MAX_NUMBER_OF_ANALOG],
                    analogLowerLimit[MAX_NUMBER_OF_ANALOG],
                    analogUpperLimit[MAX_NUMBER_OF_ANALOG],
                    analogDebounceCounter[MAX_NUMBER_OF_ANALOG];

    int16_t         analogSample[MAX_NUMBER_OF_ANALOG][3+1],
                    lastAnalogueValue[MAX_NUMBER_OF_ANALOG];

    //encoders
    uint8_t         encoderNumber[MAX_NUMBER_OF_ENCODERS],
                    encoderEnabled[MAX_NUMBER_OF_ENCODERS/8+1],
                    pulsesPerStep[MAX_NUMBER_OF_ENCODERS],
                    encoderInverted[MAX_NUMBER_OF_ENCODERS/8+1],
                    encoderFastMode[MAX_NUMBER_OF_ENCODERS/8+1],
                    initialEncoderDebounceCounter[MAX_NUMBER_OF_ENCODERS];

    uint32_t        lastEncoderSpinTime[MAX_NUMBER_OF_ENCODERS];

    //LEDs
    uint8_t         ledActNote[MAX_NUMBER_OF_LEDS],
                    totalNumberOfLEDs;

    uint8_t         receivedNote,
                    receivedVelocity;

    //sysex
    bool            sysExEnabled;
    sysExSource     sysExMessageSource;

    //general
    uint8_t         i;

    //functions

    //init
    void initVariables();
    bool initialEEPROMwrite();

    //configuration retrieval from EEPROM
    void getConfiguration();
    void getMIDIchannels();
    void getFeatures();
    void getButtonsHwParameters();
    void getButtonsType();
    void getButtonsPCenabled();
    void getButtonsNotes();
    void getAnalogEnabled();
    void getAnalogType();
    void getAnalogInversion();
    void getAnalogNumbers();
    void getAnalogLowerLimits();
    void getAnalogUpperLimits();
    void getEncodersEnabled();
    void getEncodersInverted();
    void getEncodersFastMode();
    void getEncodersPulsesPerStep();
    void getEncodersNumbers();
    void getLEDHwParameters();
    void getLEDActivationNotes();

    void clearEEPROM();

    //MIDI
    uint8_t getMIDIchannel(uint8_t);
    bool setMIDIchannel(uint8_t, uint8_t);
    bool standardNoteOffEnabled();
    void sendMIDInote(uint8_t, bool, uint8_t);
    void sendProgramChange(uint8_t, uint8_t);
    void sendControlChange(uint8_t, uint8_t, uint8_t);
    void sendSysEx(uint8_t *sysExArray, uint8_t size);

    //features
    bool getFeature(uint8_t, uint8_t);
    bool setFeature(uint8_t, uint8_t, bool);

    //buttons
    buttonType getButtonType(uint8_t);
    bool setButtonType(uint8_t, bool);
    bool setButtonNote(uint8_t, uint8_t);
    uint8_t getButtonNote(uint8_t);
    void procesButtonReading(uint8_t, uint8_t);
    bool getButtonPCenabled(uint8_t);
    bool setButtonPCenabled(uint8_t, bool);
    bool getButtonPressed(uint8_t);
    void setButtonPressed(uint8_t, bool);
    void processMomentaryButton(uint8_t, bool);
    void processLatchingButton(uint8_t, bool);
    void updateButtonState(uint8_t, uint8_t);
    bool getPreviousButtonState(uint8_t);
    bool buttonDebounced(uint8_t, bool);

    //analog
    bool getAnalogEnabled(uint8_t);
    bool setAnalogEnabled(uint8_t, bool);
    bool getAnalogInvertState(uint8_t);
    bool setAnalogInvertState(uint8_t, bool);
    uint8_t getAnalogType(uint8_t);
    bool setAnalogType(uint8_t, uint8_t);
    uint8_t getCCnumber(uint8_t);
    bool setCCnumber(uint8_t, uint8_t);
    bool setAnalogLimit(uint8_t, uint8_t, uint8_t);
    void processAnalogReading(int16_t, uint8_t);
    int16_t getMedianValue(uint8_t analogID);
    bool checkAnalogValueDifference(int16_t, uint8_t);
    void addAnalogSample(uint8_t, int16_t);
    bool analogValueSampled(uint8_t);

    //encoders
    bool getEncoderEnabled(uint8_t);
    bool setEncoderEnabled(uint8_t, bool);
    bool getEncoderInvertState(uint8_t);
    bool setEncoderInvertState(uint8_t, bool);
    uint8_t getEncoderPairEnabled(uint8_t);
    bool checkMemberOfEncPair(uint8_t, uint8_t);
    void processEncoderPair(uint8_t, uint8_t, uint8_t);
    uint8_t getEncoderPairNumber(uint8_t, uint8_t);
    bool getEncoderFastMode(uint8_t);
    bool setEncoderNumber(uint8_t, uint8_t);
    bool setEncoderPulsesPerStep(uint8_t, uint8_t);
    bool setEncoderFastMode(uint8_t, bool);
    bool setEncoderPair(uint8_t, bool);
    void resetEncoderValues(uint8_t);

    //LEDs
    uint8_t getLEDHwParameter(uint8_t);
    bool setLEDHwParameter(uint8_t, uint8_t);
    uint8_t getLEDActivationNote(uint8_t);
    bool setLEDActivationNote(uint8_t, uint8_t);
    bool setLEDstartNumber(uint8_t, uint8_t);
    uint8_t getLEDid();
    void oneByOneLED(bool, bool, bool);
    void startUpRoutine();
    bool checkLEDsOn();
    bool checkLEDsOff();
    void setLEDState();
    bool checkSameLEDvalue(uint8_t, uint8_t);
    bool ledOn(uint8_t);
    void handleLED(bool, bool, uint8_t);
    void checkBlinkLEDs();
    bool checkBlinkState(uint8_t);

    //sysex
    bool sysExCheckMessageValidity(uint8_t*, uint8_t);
    bool sysExCheckID(uint8_t, uint8_t, uint8_t);
    bool sysExCheckWish(uint8_t);
    bool sysExCheckAmount(uint8_t);
    bool sysExCheckMessageType(uint8_t);
    bool sysExCheckMessageSubType(uint8_t, uint8_t);
    bool sysExCheckParameterID(uint8_t, uint8_t, uint8_t);
    bool sysExCheckNewParameterID(uint8_t, uint8_t, uint8_t, uint8_t);
    bool sysExCheckSpecial(uint8_t, uint8_t, uint8_t);
    uint8_t sysExGenerateMinMessageLenght(uint8_t, uint8_t, uint8_t, uint8_t);
    void sysExGenerateError(sysExError);
    void sysExGenerateAck();
    void sysExGenerateResponse(uint8_t*, uint8_t);
    uint8_t sysExGet(uint8_t, uint8_t, uint8_t);
    bool sysExSet(uint8_t, uint8_t, uint8_t, uint8_t);
    bool sysExRestore(uint8_t, uint8_t, uint16_t, int16_t);
    bool sysExSetDefaultConf();
    void setSysExSource(sysExSource);

};

extern OpenDeck openDeck;

#endif