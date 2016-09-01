#ifndef LEDS_H_
#define LEDS_H_

#include "../../hardware/core/Core.h"
#include "../../BitManipulation.h"
#include "../../eeprom/Configuration.h"

class LEDs {

    public:
    LEDs();
    void init();
    ledColor_t velocity2color(bool blinEnabled, uint8_t receivedVelocity);
    bool velocity2blinkState(uint8_t receivedVelocity);
    void noteToLEDstate(uint8_t receivedNote, uint8_t receivedVelocity);
    void allLEDsOn();
    void allLEDsOff();
    void setFadeSpeed(uint8_t speed);

    private:
    //data processing
    void handleLED(bool newLEDstate, bool blinkMode, uint8_t ledNumber);
    bool checkLEDsOn();
    bool checkLEDsOff();

    //animation
    void oneByOneLED(bool ledDirection, bool singleLED, bool turnOn);
    void startUpAnimation();

};

extern LEDs leds;

#endif
