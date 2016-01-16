#ifndef LEDS_H_
#define LEDS_H_

#include "..\hardware\board\Board.h"
#include "..\BitManipulation.h"
#include "..\eeprom\Configuration.h"

class LEDs {

    public:
    LEDs();
    void init();
    uint8_t getParameter(uint8_t messageType, uint8_t parameterID);
    bool setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter);
    ledColor velocity2color(bool blinEnabled, uint8_t receivedVelocity);
    bool velocity2blinkState(uint8_t receivedVelocity);
    void noteToLEDstate(uint8_t receivedNote, uint8_t receivedVelocity);
    void allLEDsOn();
    void allLEDsOff();

    private:
    //data processing
    void handleLED(bool newLEDstate, bool blinkMode, uint8_t ledNumber);
    bool checkLEDsOn();
    bool checkLEDsOff();
    bool checkLEDstartUpNumber(uint8_t ledID);
    bool checkLEDactivationNote(uint8_t activationNote);

    //get
    uint8_t getLEDHwParameter(uint8_t parameter);
    uint8_t getLEDActivationNote(uint8_t ledNumber);
    uint8_t getLEDstartUpNumber(uint8_t ledNumber);
    bool getRGBenabled(uint8_t ledNumber);
    uint8_t getLEDid(uint8_t midiID);

    //set
    bool setLEDHwParameter(uint8_t parameter, uint8_t newParameter);
    bool setLEDActivationNote(uint8_t ledNumber, uint8_t ledActNote);
    bool setLEDstartNumber(uint8_t startNumber, uint8_t ledNumber);
    bool setRGBenabled(uint8_t ledNumber, bool state);

    //animation
    void oneByOneLED(bool ledDirection, bool singleLED, bool turnOn);
    void startUpAnimation();

};

extern LEDs leds;

#endif