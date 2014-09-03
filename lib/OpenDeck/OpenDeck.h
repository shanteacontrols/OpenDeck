/*

OpenDECK library v1.96
File: OpenDeck.h
Last revision date: 2014-09-03
Author: Igor Petrovic

*/


#ifndef OpenDeck_h
#define OpenDeck_h

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "HardwareControl.h"

#define BUTTON_DEBOUNCE_TIME        15

//potentiometer must exceed this value before sending new value
#define MIDI_CC_STEP                8

//potentiometer must exceed this value if it hasn't been moved for more than POTENTIOMETER_MOVE_TIMEOUT
#define MIDI_CC_STEP_TIMEOUT        9

//time in ms after which new value from pot must exceed MIDI_CC_STEP_TIMEOUT
#define POTENTIOMETER_MOVE_TIMEOUT  200

#define MAX_NUMBER_OF_POTS          16
#define MAX_NUMBER_OF_BUTTONS       64
#define MAX_NUMBER_OF_LEDS          64

class OpenDeck  {

    public:

    //constructor
    OpenDeck();

    //library initializer
    void init();
    
    //start-up routine
    void startUpRoutine();

    //hardware configuration
    void setHandlePinInit(void (*fptr)());
    void setHandleColumnSwitch(void (*fptr)(uint8_t));
    void setHandleButtonRead(void (*fptr)(uint8_t &));
    void setHandleMuxOutput(void (*fptr)(uint8_t));
    void setHandleLEDrowOn(void (*fptr)(uint8_t));
    void setHandleLEDrowsOff(void (*fptr)());
    void setNumberOfColumns(uint8_t);
    void setNumberOfButtonRows(uint8_t);
    void setNumberOfLEDrows(uint8_t);
    void setNumberOfMux(uint8_t);
    void enableAnalogueInput(uint8_t);

    //buttons
    void setHandleButtonSend(void (*fptr)(uint8_t, bool, uint8_t));
    bool buttonsEnabled();
    void readButtons();

    //pots
    void setHandlePotCC(void (*fptr)(uint8_t, uint8_t, uint8_t));
    void setHandlePotNoteOn(void (*fptr)(uint8_t, uint8_t, uint8_t));
    void setHandlePotNoteOff(void (*fptr)(uint8_t, uint8_t, uint8_t));
    bool potsEnabled();
    void readPots();

    //LEDs
    bool ledsEnabled();
    void oneByOneLED(bool, bool, bool);
    void allLEDsOn();
    void allLEDsOff();
    void turnOnLED(uint8_t);
    void turnOffLED(uint8_t);
    void storeReceivedNote(uint8_t, uint8_t, uint8_t);
    void checkReceivedNote();
    void checkLEDs();

    //columns
    void nextColumn();

    //getters
    uint8_t getInputMIDIchannel();
    bool standardNoteOffEnabled();
    
    //setters
    void setBoard(uint8_t);

    //sysex
    void setHandleSysExSend(void (*fptr)(uint8_t*, uint8_t));
    void processSysEx(uint8_t sysExArray[], uint8_t);

    private:

    //variables

    //hardware params
    uint16_t    _longPressTime,
                _blinkTime,
                _startUpLEDswitchTime;

    //software features
    uint8_t softwareFeatures,
            startUpRoutinePattern;

    //hardware features
    uint8_t hardwareFeatures;

    //buttons
    uint8_t buttonNote[MAX_NUMBER_OF_BUTTONS],
            previousButtonState[MAX_NUMBER_OF_BUTTONS],
            buttonDebounceCompare;

    uint8_t buttonType[MAX_NUMBER_OF_BUTTONS/8],
            buttonPressed[MAX_NUMBER_OF_BUTTONS/8],
            longPressSent[MAX_NUMBER_OF_BUTTONS/8];

    uint32_t longPressState[MAX_NUMBER_OF_BUTTONS];

    //pots
    uint8_t potInverted[MAX_NUMBER_OF_POTS/8],
            potEnabled[MAX_NUMBER_OF_POTS/8];

    uint8_t ccNumber[MAX_NUMBER_OF_POTS],
            lastPotNoteValue[MAX_NUMBER_OF_POTS],
            potNumber;

    uint16_t    lastAnalogueValue[MAX_NUMBER_OF_POTS];
    uint32_t    potTimer[MAX_NUMBER_OF_POTS];

    //LEDs
    uint8_t ledState[MAX_NUMBER_OF_LEDS],
            ledID[MAX_NUMBER_OF_LEDS],
            totalNumberOfLEDs;

    bool    blinkState,
            blinkEnabled;

    uint32_t    blinkTimerCounter;

    //MIDI channels
    uint8_t _buttonNoteChannel,
            _longPressButtonNoteChannel,
            _potCCchannel,
            _encCCchannel,
            _inputChannel;

    //input
    bool    receivedNoteProcessed;

    uint8_t receivedNoteChannel,
            receivedNotePitch,
            receivedNoteVelocity;

    //hardware
    uint8_t column,
            _numberOfColumns,
            _numberOfButtonRows,
            _numberOfLEDrows,
            _numberOfMux,
            _board;

    uint8_t _analogueIn;
    
    //sysex
    bool sysExEnabled;

    //general
    uint8_t i;

    //functions

    //init
    void initVariables();
    void (*sendInitPinsCallback)();

    //read configuration
    void getConfiguration();
    void getMIDIchannels();
    void getHardwareParams();
    void getSoftwareFeatures();
    void getStartUpRoutinePattern();
    void getHardwareFeatures();
    void getButtonsType();
    void getButtonNotes();
    void getEnabledPots();
    void getPotInvertStates();
    void getCCnumbers();
    void getLEDIDs();
    void getTotalLEDnumber();

    //buttons
    void (*sendButtonReadCallback)(uint8_t &buttonColumnState);
    void (*sendButtonDataCallback)(uint8_t, bool, uint8_t);
    void setNumberOfColumnPasses();
    void setButtonDebounceCompare(uint8_t);
    uint8_t checkButton(uint8_t, uint8_t);

    //pots
    void (*sendSwitchMuxOutCallback)(uint8_t);
    void (*sendPotCCDataCallback)(uint8_t, uint8_t, uint8_t);
    void (*sendPotNoteOnDataCallback)(uint8_t, uint8_t, uint8_t);
    void (*sendPotNoteOffDataCallback)(uint8_t, uint8_t, uint8_t);
    bool adcConnected(uint8_t);
    void readPotsMux(uint8_t);
    void checkPotReading(int16_t, uint8_t);
    void processPotReading(uint8_t, int16_t);
    uint8_t getPotNoteValue(uint8_t, uint8_t);
    bool checkPotNoteValue(uint8_t, uint8_t);

    //LEDs
    void (*sendLEDrowOnCallback)(uint8_t);
    void (*sendLEDrowsOffCallback)();
    bool ledOn(uint8_t);
    bool checkLEDsOn();
    bool checkLEDsOff();
    void checkBlinkLEDs();
    bool checkBlinkState(uint8_t);
    void handleLED(uint8_t, bool, bool);
    void setLEDState();
    void setConstantLEDstate(uint8_t);
    void setBlinkState(uint8_t, bool);
    void switchBlinkState();

    //columns
    void (*sendColumnSwitchCallback)(uint8_t);
    uint8_t getActiveColumn();

    //getters
    bool getFeature(uint8_t, uint8_t);
    bool getButtonType(uint8_t);
    uint8_t getButtonNote(uint8_t);
    bool getButtonPressed(uint8_t);
    bool getButtonLongPressed(uint8_t);
    bool getPotEnabled(uint8_t);
    bool getPotInvertState(uint8_t);
    uint8_t getCCnumber(uint8_t);
    uint8_t getLEDID(uint8_t);

    //setters
    void setButtonPressed(uint8_t, bool);
    void setButtonLongPressed(uint8_t, bool);

    //sysex
    void (*sendSysExDataCallback)(uint8_t*, uint8_t);
    bool sysExCheckMessageValidity(uint8_t*, uint8_t);
    bool sysExCheckID(uint8_t, uint8_t, uint8_t);
    bool sysExCheckWish(uint8_t);
    bool sysExCheckSingleAll(uint8_t);
    bool sysExCheckMessageType(uint8_t);
    bool sysExCheckMessageSubType(uint8_t, uint8_t);
    bool sysExCheckParameterID(uint8_t, uint8_t);
    bool sysExCheckNewParameterID(uint8_t, uint8_t, uint8_t);
    void sysExGenerateError(uint8_t);
    void sysExGenerateAck();
    uint8_t sysExGenerateMinMessageLenght(bool, bool, uint8_t);
    void sysExGenerateResponse(uint8_t*, uint8_t);
    //getters
    bool sysExGetFeature(uint8_t, uint8_t);
    uint8_t sysExGetHardwareParameter(uint8_t);
    uint8_t sysExGetMIDIchannel(uint8_t);
    //setters
    bool sysExSetFeature(uint8_t, uint8_t, bool);
    bool sysExSetHardwareParameter(uint8_t, uint8_t);
    bool sysExSetButtonType(uint8_t, bool);
    bool sysExSetButtonNote(uint8_t, uint8_t);
    bool sysExSetPotEnabled(uint8_t, bool);
    bool sysExSetPotInvertState(uint8_t, bool);
    bool sysExSetCCnumber(uint8_t, uint8_t);
    bool sysExSetAllPotsEnable();
    bool sysExSetAllPotsDisable();
    bool sysExSetLEDID(uint8_t, uint8_t);
    bool sysExSetMIDIchannel(uint8_t, uint8_t);
    void sysExSetDefaultConf();

};

extern OpenDeck openDeck;

#endif