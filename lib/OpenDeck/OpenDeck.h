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

//velocity for on and off events
#define MIDI_NOTE_ON_VELOCITY       127
#define MIDI_NOTE_OFF_VELOCITY      0

#define NUMBER_OF_START_UP_ROUTINES 5

class OpenDeck  {

    public:

    //constructor
    OpenDeck();

    //library initializer
    void init();

    //callbacks
    void setHandleNoteSend(void (*fptr)(uint8_t, bool, uint8_t));
    void setHandleProgramChangeSend(void (*fptr)(uint8_t, uint8_t));
    void setHandleControlChangeSend(void (*fptr)(uint8_t, uint8_t, uint8_t));
    void setHandlePitchBendSend(void (*fptr)(uint16_t, uint8_t));
    void setHandleSysExSend(void (*fptr)(uint8_t*, uint8_t));

    //buttons
    void readButtons();

    //analog inputs
    void readAnalog();

    //encoders
    void readEncoders();

    //LEDs
    void allLEDsOn();
    void allLEDsOff();
    void storeReceivedNoteOn(uint8_t, uint8_t, uint8_t);
    void checkLEDs(uint8_t);

    //getters
    uint8_t getInputMIDIchannel();
    bool standardNoteOffEnabled();
    bool ledOn(uint8_t);

    //sysex
    void processSysEx(uint8_t sysExArray[], uint8_t);
    bool sysExSetDefaultConf();

    private:

    //variables

    //features
    uint8_t         midiFeatures,
                    buttonFeatures,
                    ledFeatures,
                    analogFeatures,
                    encoderFeatures;

    //MIDI channels
    uint8_t         _buttonNoteChannel,
                    _longPressButtonNoteChannel,
                    _programChangeChannel,
                    _analogCCchannel,
                    _pitchBendChannel,
                    _inputChannel;

    //buttons
    uint8_t         buttonType[MAX_NUMBER_OF_BUTTONS/8],
                    buttonPCenabled[MAX_NUMBER_OF_BUTTONS/8],
                    buttonNote[MAX_NUMBER_OF_BUTTONS],
                    previousButtonState[MAX_NUMBER_OF_BUTTONS/8],
                    buttonPressed[MAX_NUMBER_OF_BUTTONS/8],
                    longPressSent[MAX_NUMBER_OF_BUTTONS/8],
                    longPressCounter[MAX_NUMBER_OF_BUTTONS],
                    lastColumnState[8],
                    columnPassCounter[8];

    //analog
    uint8_t         analogEnabled[MAX_NUMBER_OF_ANALOG/8],
                    analogType[MAX_NUMBER_OF_ANALOG],
                    analogInverted[MAX_NUMBER_OF_ANALOG/8],
                    analogNumber[MAX_NUMBER_OF_ANALOG],
                    analogLowerLimit[MAX_NUMBER_OF_ANALOG],
                    analogUpperLimit[MAX_NUMBER_OF_ANALOG],
                    analogDebounceCounter[MAX_NUMBER_OF_ANALOG];

    uint16_t        lastAnalogueValue[MAX_NUMBER_OF_ANALOG];

    //encoders
    int32_t         lastEncoderState[NUMBER_OF_ENCODERS];
    uint8_t         initialEncoderDebounceCounter[NUMBER_OF_ENCODERS];
    bool            encoderDirection[NUMBER_OF_ENCODERS];
    uint32_t        lastEncoderSpinTime[NUMBER_OF_ENCODERS];
    uint8_t         encoderNumber[NUMBER_OF_ENCODERS],
                    encoderEnabled[NUMBER_OF_ENCODERS],
                    pulsesPerStep[NUMBER_OF_ENCODERS],
                    pulseCounter[NUMBER_OF_ENCODERS],
                    encoderInverted[NUMBER_OF_ENCODERS],
                    encoderFastMode[NUMBER_OF_ENCODERS];

    //LEDs
    uint8_t         ledActNote[MAX_NUMBER_OF_LEDS],
                    totalNumberOfLEDs;

    uint8_t         receivedChannel,
                    receivedNote,
                    receivedVelocity;

    //sysex
    bool            sysExEnabled;

    //general
    uint8_t         i;

    //functions

    //init
    void initVariables();
    bool initialEEPROMwrite();

    //callbacks
    void (*sendNoteCallback)(uint8_t, bool, uint8_t);
    void (*sendProgramChangeCallback)(uint8_t, uint8_t);
    void (*sendControlChangeCallback)(uint8_t, uint8_t, uint8_t);
    void (*sendPitchBendCallback)(uint16_t, uint8_t);
    void (*sendSysExCallback)(uint8_t*, uint8_t);

    //configuration retrieval from EEPROM
    void getConfiguration();
    void getFeatures();
    void getMIDIchannels();
    void getButtonsType();
    void getButtonsPCenabled();
    void getButtonsNotes();
    void getButtonsHwParameters();
    void getAnalogEnabled();
    void getAnalogType();
    void getAnalogInversion();
    void getAnalogNumbers();
    void getAnalogLowerLimits();
    void getAnalogUpperLimits();
    void getEncodersEnabled();
    void getEncodersInverted();
    void getEncodersFastMode();
    void getEncodersNumbers();
    void getEncodersPulsesPerStep();
    void getLEDnotes();
    void getLEDHwParameters();

    //buttons
    void procesButtonReading(uint8_t buttonNumber, uint8_t buttonState);
    uint8_t getButtonType(uint8_t);
    uint8_t getButtonNote(uint8_t);
    bool getButtonPCenabled(uint8_t);
    bool getButtonPressed(uint8_t);
    bool getButtonLongPressed(uint8_t);
    void setButtonPressed(uint8_t, bool);
    void setButtonLongPressed(uint8_t, bool);
    void processMomentaryButton(uint8_t, bool);
    void processLatchingButton(uint8_t, bool);
    void updateButtonState(uint8_t, uint8_t);
    bool getPreviousButtonState(uint8_t);
    bool columnStable(uint16_t columnState, uint8_t columnNumber);

    void resetLongPress(uint8_t);
    void handleLongPress(uint8_t, bool);

    //analog
    void readAnalogInitial();
    bool checkAnalogReading(int16_t, uint8_t);
    void processAnalogReading(int16_t, uint8_t);
    bool getAnalogEnabled(uint8_t);
    uint8_t getAnalogType(uint8_t);
    bool getAnalogInvertState(uint8_t);
    uint8_t getAnalogNumber(uint8_t);

    //encoders
    bool getEncoderEnabled(uint8_t encoderNumber);
    bool getEncoderInverted(uint8_t encoderNumber);
    uint8_t getEncoderPairEnabled(uint8_t);
    bool checkMemberOfEncPair(uint8_t, uint8_t);
    void processEncoderPair(uint8_t, uint8_t, uint8_t);
    uint8_t getEncoderPairNumber(uint8_t, uint8_t);
    bool getEncoderFastMode(uint8_t);

    //LEDs
    void oneByOneLED(bool, bool, bool);
    void startUpRoutine();
    bool checkLEDsOn();
    bool checkLEDsOff();
    void setLEDState();
    uint8_t getLEDnumber();
    uint8_t getLEDnote(uint8_t);
    bool checkSameLEDvalue(uint8_t, uint8_t);

    //sysex
    //message check
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
    //sysex response
    void sysExGenerateError(uint8_t);
    void sysExGenerateAck();
    void sysExGenerateResponse(uint8_t*, uint8_t);
    //getters
    uint8_t sysExGet(uint8_t, uint8_t, uint8_t);
    uint8_t sysExGetMIDIchannel(uint8_t);
    uint8_t sysExGetHardwareParameter(uint8_t);
    bool sysExGetFeature(uint8_t, uint8_t);
    uint8_t sysExGetButtonHwParameter(uint8_t);
    uint8_t sysExGetLEDHwParameter(uint8_t);
    //setters
    bool sysExSet(uint8_t, uint8_t, uint8_t, uint8_t);
    bool sysExSetMIDIchannel(uint8_t, uint8_t);
    bool sysExSetFeature(uint8_t, uint8_t, bool);
    bool sysExSetButtonType(uint8_t, bool);
    bool sysExSetButtonNote(uint8_t, uint8_t);
    bool sysExSetAnalogEnabled(uint8_t, bool);
    bool sysExSetAnalogInvertState(uint8_t, bool);
    bool sysExSetAnalogNumber(uint8_t, uint8_t);
    bool sysExSetAnalogLimit(uint8_t, uint8_t, uint8_t);
    bool sysExSetEncoderEnabled(uint8_t, bool);
    bool sysExSetEncoderInvertState(uint8_t, bool);
    bool sysExSetEncoderNumber(uint8_t, uint8_t);
    bool sysExSetEncoderPulsesPerStep(uint8_t, uint8_t);
    bool sysExSetEncoderFastMode(uint8_t, bool);
    bool sysExSetLEDnote(uint8_t, uint8_t);
    bool sysExSetLEDstartNumber(uint8_t, uint8_t);
    bool sysExSetEncoderPair(uint8_t, bool);
    bool sysExSetButtonHwParameter(uint8_t, uint8_t);
    bool sysExSetButtonPPenabled(uint8_t, bool);
    bool sysExSetHardwareConfig(uint8_t, uint8_t);
    bool sysExSetLEDHwParameter(uint8_t, uint8_t);
    bool sysExSetAnalogType(uint8_t, uint8_t);
    //restore
    bool sysExRestore(uint8_t, uint8_t, uint16_t, int16_t);

    //input
    void checkReceivedNoteOn();

};

extern OpenDeck openDeck;

#endif