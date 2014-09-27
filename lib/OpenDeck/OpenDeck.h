/*

OpenDECK library v1.1
File: OpenDeck.h
Last revision date: 2014-09-28
Author: Igor Petrovic

*/


#ifndef OpenDeck_h
#define OpenDeck_h

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "EEPROM.h"
#include "SysEx.h"

#define BUTTON_DEBOUNCE_TIME        15

//potentiometer must exceed this value before sending new value
#define MIDI_CC_STEP                8

//potentiometer must exceed this value if it hasn't been moved for more than POTENTIOMETER_MOVE_TIMEOUT
#define MIDI_CC_STEP_TIMEOUT        9

//time in ms after which new value from pot must exceed MIDI_CC_STEP_TIMEOUT
#define POTENTIOMETER_MOVE_TIMEOUT  200

#define MAX_NUMBER_OF_POTS          16
#define MAX_NUMBER_OF_BUTTONS       56
#define MAX_NUMBER_OF_LEDS          64

#define NUMBER_OF_START_UP_ROUTINES 5


class OpenDeck  {

    public:

    //constructor
    OpenDeck();

    //library initializer
    void init();

    //buttons
    void setHandleButtonSend(void (*fptr)(uint8_t, bool, uint8_t));
    void readButtons();

    //pots
    void setHandlePotCC(void (*fptr)(uint8_t, uint8_t, uint8_t));
    void setHandlePotNoteOn(void (*fptr)(uint8_t, uint8_t));
    void setHandlePotNoteOff(void (*fptr)(uint8_t, uint8_t));
    void readPots();

    //LEDs
    void oneByOneLED(bool, bool, bool);
    void allLEDsOn();
    void allLEDsOff();
    void turnOnLED(uint8_t);
    void turnOffLED(uint8_t);
    void storeReceivedNoteOn(uint8_t, uint8_t, uint8_t);
    void checkReceivedNoteOn();
    void checkLEDs();

    //columns
    void nextColumn();

    //getters
    uint8_t getInputMIDIchannel();
    bool standardNoteOffEnabled();
    bool runningStatusEnabled();

    //setters
    bool sysExSetDefaultConf();

    //sysex
    void setHandleSysExSend(void (*fptr)(uint8_t*, uint8_t));
    void processSysEx(uint8_t sysExArray[], uint8_t);

    private:

    //variables

    //MIDI channels
    uint8_t     _buttonNoteChannel,
                _longPressButtonNoteChannel,
                _potCCchannel,
                _potNoteChannel,
                _encCCchannel,
                _inputChannel;

    //hardware params
    uint16_t    _longPressTime,
                _blinkTime,
                _startUpLEDswitchTime;

    //free pins
    bool        freePinConfEn;
    uint8_t     freePinState[SYS_EX_FREE_PIN_END],
                freePinsAsBRows,
                freePinsAsLRows;

    //software features
    uint8_t     softwareFeatures,
                startUpRoutinePattern;

    //hardware features
    uint8_t     hardwareFeatures;

    //buttons
    uint8_t     buttonNote[MAX_NUMBER_OF_BUTTONS],
                previousButtonState[MAX_NUMBER_OF_BUTTONS],
                buttonDebounceCompare;

    uint8_t     buttonType[MAX_NUMBER_OF_BUTTONS/8],
                buttonPressed[MAX_NUMBER_OF_BUTTONS/8],
                longPressSent[MAX_NUMBER_OF_BUTTONS/8];

    uint32_t    longPressState[MAX_NUMBER_OF_BUTTONS];
    bool        _longPressEnabled,
                _standardNoteOffEnabled;

    //pots
    uint8_t     potInverted[MAX_NUMBER_OF_POTS/8],
                potEnabled[MAX_NUMBER_OF_POTS/8];

    uint8_t     ccNumber[MAX_NUMBER_OF_POTS],
                lastPotNoteValue[MAX_NUMBER_OF_POTS],
                ccLowerLimit[MAX_NUMBER_OF_POTS],
                ccUpperLimit[MAX_NUMBER_OF_POTS];

    uint16_t    lastAnalogueValue[MAX_NUMBER_OF_POTS];
    uint32_t    potTimer[MAX_NUMBER_OF_POTS];

    //LEDs
    uint8_t     ledState[MAX_NUMBER_OF_LEDS],
                ledNote[MAX_NUMBER_OF_LEDS],
                totalNumberOfLEDs;

    bool        blinkState,
                blinkEnabled;

    uint32_t    blinkTimerCounter;

    //input
    bool        receivedNoteOnProcessed;

    uint8_t     receivedChannel,
                receivedNote,
                receivedVelocity;

    //hardware
    uint8_t     column,
                _numberOfColumns,
                _numberOfButtonRows,
                _numberOfLEDrows,
                _numberOfMux,
                _board;

    uint8_t     _analogueIn;

    //sysex
    bool        sysExEnabled;

    //general
    uint8_t     i;

    //functions

    //init
    void initVariables();
    bool initialEEPROMwrite();

    //configuration retrieval from EEPROM
    void getConfiguration();
    void getMIDIchannels();
    void getHardwareParams();
    void getFreePinStates();
    void getSoftwareFeatures();
    void getHardwareFeatures();
    void getButtonsType();
    void getButtonNotes();
    void getEnabledPots();
    void getPotInvertStates();
    void getCCnumbers();
    void getCClowerLimits();
    void getCCupperLimits();
    void getLEDnotes();

    //buttons
    void (*sendButtonDataCallback)(uint8_t, bool, uint8_t);
    void setNumberOfColumnPasses();
    void setButtonDebounceCompare(uint8_t);
    bool checkButton(uint8_t, uint8_t);
    void procesButtonReading(uint8_t buttonNumber, uint8_t buttonState);
    uint8_t getButtonType(uint8_t);
    uint8_t getButtonNote(uint8_t);
    bool getButtonPressed(uint8_t);
    bool getButtonLongPressed(uint8_t);
    void setButtonPressed(uint8_t, bool);
    void setButtonLongPressed(uint8_t, bool);

    //pots
    void (*sendPotCCDataCallback)(uint8_t, uint8_t, uint8_t);
    void (*sendPotNoteOnDataCallback)(uint8_t, uint8_t);
    void (*sendPotNoteOffDataCallback)(uint8_t, uint8_t);
    bool adcConnected(uint8_t);
    void readPotsMux(uint8_t, uint8_t);
    bool checkPotReading(int16_t, uint8_t);
    void processPotReading(int16_t, uint8_t);
    bool getPotEnabled(uint8_t);
    bool getPotInvertState(uint8_t);
    uint8_t getCCnumber(uint8_t);
    uint8_t getPotNoteValue(uint8_t, uint8_t);
    bool checkPotNoteValue(uint8_t, uint8_t);

    //LEDs
    void startUpRoutine();
    bool ledOn(uint8_t);
    bool checkLEDsOn();
    bool checkLEDsOff();
    void checkBlinkLEDs();
    bool checkBlinkState(uint8_t);
    void handleLED(bool, bool, uint8_t);
    void setLEDState();
    void setConstantLEDstate(uint8_t);
    void setBlinkState(uint8_t, bool);
    void switchBlinkState();
    uint8_t getLEDnumber();
    uint8_t getLEDnote(uint8_t);

    //columns
    uint8_t getActiveColumn();


    //sysex
    //callback
    void (*sendSysExDataCallback)(uint8_t*, uint8_t);
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
    uint8_t sysExGenerateMinMessageLenght(uint8_t, uint8_t, uint8_t);
    //sysex response
    void sysExGenerateError(uint8_t);
    void sysExGenerateAck();
    void sysExGenerateResponse(uint8_t*, uint8_t);
    //getters
    uint8_t sysExGet(uint8_t, uint8_t, uint8_t);
    uint8_t sysExGetMIDIchannel(uint8_t);
    uint8_t sysExGetHardwareParameter(uint8_t);
    uint8_t sysExGetFreePinState(uint8_t pin);
    bool sysExGetFeature(uint8_t, uint8_t);
    //setters
    bool sysExSet(uint8_t, uint8_t, uint8_t, uint8_t);
    bool sysExSetMIDIchannel(uint8_t, uint8_t);
    bool sysExSetHardwareParameter(uint8_t, uint8_t);
    bool sysExSetFreePin(uint8_t, uint8_t);
    bool sysExSetFeature(uint8_t, uint8_t, bool);
    bool sysExSetButtonType(uint8_t, bool);
    bool sysExSetButtonNote(uint8_t, uint8_t);
    bool sysExSetPotEnabled(uint8_t, bool);
    bool sysExSetPotInvertState(uint8_t, bool);
    bool sysExSetCCnumber(uint8_t, uint8_t);
    bool sysExSetCClimit(uint8_t, uint8_t, uint8_t);
    bool sysExSetLEDnote(uint8_t, uint8_t);
    bool sysExSetLEDstartNumber(uint8_t, uint8_t);
    //restore
    bool sysExRestore(uint8_t, uint8_t, uint16_t, int16_t);

    //hardware control
    void initBoard();
    void initPins();
    void activateColumn(uint8_t);
    void readButtonColumn(uint8_t &);
    bool enableAnalogueInput(uint8_t);
    void setMuxOutput(uint8_t);
    void ledRowOn(uint8_t);
    void ledRowsOff();

    //free pins
    void configureFreePins();
    bool configureFreePin(uint8_t, uint8_t);
    uint8_t readButtonRowFreePin(uint8_t);
    void ledRowOffFreePin(uint8_t);
    void ledRowOnFreePin(uint8_t);

};

extern OpenDeck openDeck;

#endif