#include "OpenDeck.h"

//1k
#define FSR_MIN_VALUE                       40
#define FSR_MAX_VALUE                       350
#define AFTERTOUCH_MAX_VALUE                600

#define FSR_DEBOUNCE_TIME                   10

#define AFTERTOUCH_SEND_TIMEOUT_IGNORE      25       //ignore aftertouch reading change below this timeout

#define AFTERTOUCH_SEND_TIMEOUT_STEP        2

#define AFTERTOUCH_SEND_TIMEOUT             100

bool fsrPressed[MAX_NUMBER_OF_ANALOG] = { 0 };
uint8_t lastAfterTouchValue[MAX_NUMBER_OF_ANALOG] = { 0 };

bool debounceTimerStarted[MAX_NUMBER_OF_ANALOG] = { 0 };

uint32_t debounceTimer[MAX_NUMBER_OF_ANALOG] = { 0 };
uint32_t lastReadTime[MAX_NUMBER_OF_ANALOG] = { 0 };
uint32_t afterTouchSendTimer[MAX_NUMBER_OF_ANALOG] = { 0 };

inline uint8_t calibratePressure(int16_t value, pressureType type)  {

    switch(type)    {

        case velocity:
        return (uint8_t)map(constrain(value, FSR_MIN_VALUE, FSR_MAX_VALUE), FSR_MIN_VALUE, FSR_MAX_VALUE, 0, 127);

        case aftertouch:
        return map(constrain(value, FSR_MIN_VALUE, AFTERTOUCH_MAX_VALUE), FSR_MIN_VALUE, AFTERTOUCH_MAX_VALUE, 0, 127);

        default:
        return 0;

    }

}

bool OpenDeck::fsrPressureStable(uint8_t pressDetected, uint8_t analogID)  {

    if (pressDetected) {

        if (!debounceTimerStarted[analogID])  {

            debounceTimerStarted[analogID] = true;
            debounceTimer[analogID] = boardObject.newMillis();
            return false;

        }   return (boardObject.newMillis() - debounceTimer[analogID] > FSR_DEBOUNCE_TIME);


        } else {

        if (!debounceTimerStarted[analogID])  {

            debounceTimerStarted[analogID] = true;
            debounceTimer[analogID] = boardObject.newMillis();
            return false;

        }   return (boardObject.newMillis() - debounceTimer[analogID] > FSR_DEBOUNCE_TIME);

    }

}

void OpenDeck::checkFSRvalue(int16_t pressure, uint8_t analogID, analogType type)  {

    uint8_t calibratedPressure = calibratePressure(pressure, velocity);

    bool pressDetected = ((pressure > FSR_MIN_VALUE) && (calibratedPressure > 0));

    if (!fsrPressureStable(pressDetected, analogID)) return;

    switch (pressDetected)    {

        case true:

        if (!fsrPressed[analogID]) {

            //sensor is really pressed
            fsrPressed[analogID] = true;
            sendMIDInote(ccNumber[analogID], true, calibratedPressure);

        }
        break;

        case false:

        if (fsrPressed[analogID])  {

            fsrPressed[analogID] = false;
            sendMIDInote(ccNumber[analogID], false, 0);

        }

        break;

    }

    //aftertouch
    if (fsrPressed[analogID]) {

        uint8_t calibratedPressureAfterTouch = calibratePressure(pressure, aftertouch);

        uint32_t timeDifference = boardObject.newMillis() - afterTouchSendTimer[analogID];
        bool afterTouchSend = false;

        if (timeDifference > AFTERTOUCH_SEND_TIMEOUT)  {

            if (abs(calibratedPressureAfterTouch - lastAfterTouchValue[analogID]) > AFTERTOUCH_SEND_TIMEOUT_STEP)    {

                afterTouchSend = true;

            }

        }   else if ((calibratedPressureAfterTouch != lastAfterTouchValue[analogID]) && (timeDifference > AFTERTOUCH_SEND_TIMEOUT_IGNORE)) {

            afterTouchSend = true;

        }

        if (afterTouchSend) {

            afterTouchSendTimer[analogID] = boardObject.newMillis();
            lastAfterTouchValue[analogID] = calibratedPressureAfterTouch;
            //sendMIDInote(ccNumber[analogID], true, calibratedPressureAfterTouch);

        }   lastAnalogueValue[analogID] = calibratedPressure;

    }

}