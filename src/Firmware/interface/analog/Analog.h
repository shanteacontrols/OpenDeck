#ifndef ANALOG_H_
#define ANALOG_H_

#include "../../hardware/core/Core.h"
#include "../../interface/midi/MIDI.h"
#include "../../BitManipulation.h"
#include "../../eeprom/Configuration.h"

#define NUMBER_OF_SAMPLES 3 //do not change

class Analog {

    public:
    Analog();
    void update();
    void debounceReset(uint16_t index);

    private:

    //variables
    uint8_t analogDebounceCounter,
            fsrPressed[MAX_NUMBER_OF_ANALOG/8+1],
            fsrLastAfterTouchValue[MAX_NUMBER_OF_ANALOG];

    int16_t analogSample[MAX_NUMBER_OF_ANALOG][NUMBER_OF_SAMPLES],
            lastAnalogueValue[MAX_NUMBER_OF_ANALOG];

    //data processing
    void checkPotentiometerValue(uint8_t analogID, int16_t tempValue);
    void checkFSRvalue(uint8_t analogID, int16_t pressure);
    bool fsrPressureStable(uint8_t analogID);
    bool getFsrPressed(uint8_t fsrID);
    void setFsrPressed(uint8_t fsrID, bool state);
    bool getFsrDebounceTimerStarted(uint8_t fsrID);
    void setFsrDebounceTimerStarted(uint8_t fsrID, bool state);
    int16_t getMedianValue(uint8_t analogID);
    void addAnalogSamples();
    bool analogValuesSampled();
    inline uint8_t mapAnalog_uint8(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max) {

        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

    };

};

extern Analog analog;

#endif
