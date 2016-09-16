#ifndef LEDS_H_
#define LEDS_H_

#include "../../board/Board.h"
#include "../../BitManipulation.h"
#include "../../eeprom/Configuration.h"

#include "LEDcolors.h"

class LEDs : Board {

    public:
    LEDs();
    void init();
    ledColor_t velocity2color(bool blinEnabled, uint8_t receivedVelocity);
    bool velocity2blinkState(uint8_t receivedVelocity);
    void noteToLEDstate(uint8_t receivedNote, uint8_t receivedVelocity);
    void allOn();
    void allOff();
    uint8_t getState(uint8_t ledNumber);
    void setState(uint8_t ledNumber, ledColor_t color, bool blinkMode);
    void setBlinkTime(uint16_t blinkTime);
    void setFadeTime(uint8_t fadeTime);

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
