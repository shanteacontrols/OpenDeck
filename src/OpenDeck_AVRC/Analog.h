#ifndef ANALOG_H_
#define ANALOG_H_

#include "hardware/Board.h"

#define NUMBER_OF_SAMPLES 3 //do not change

class Analog {

    public:
    Analog();
    void init();
    void update();
    uint8_t getParameter(uint8_t messageType, uint8_t parameter);
    bool setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter);

    private:

    //variables
    uint8_t analogDebounceCounter[MAX_NUMBER_OF_ANALOG];

    int16_t analogSample[MAX_NUMBER_OF_ANALOG][3+1],
            lastAnalogueValue[MAX_NUMBER_OF_ANALOG];

    //data processing
    void checkPotentiometerValue(int16_t tempValue, uint8_t analogID);
    int16_t getMedianValue(uint8_t analogID);
    void addAnalogSample(uint8_t analogID, int16_t sample);
    bool analogValueSampled(uint8_t analogID);
    inline uint8_t mapAnalog(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max) {

        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

    };

    //set
    bool setAnalogEnabled(uint8_t analogID, uint8_t state);
    bool setAnalogType(uint8_t analogID, uint8_t type);
    bool setAnalogInvertState(uint8_t analogID, uint8_t state);
    bool setMIDIid(uint8_t analogID, uint8_t midiID);
    bool setCClimit(ccLimitType type, uint8_t analogID, uint8_t limit);

    //get
    bool getAnalogEnabled(uint8_t analogID);
    analogType getAnalogType(uint8_t analogID);
    bool getAnalogInvertState(uint8_t analogID);
    uint8_t getMIDIid(uint8_t analogID);
    uint8_t getCClimit(uint8_t analogID, ccLimitType type);

};

extern Analog analog;

#endif