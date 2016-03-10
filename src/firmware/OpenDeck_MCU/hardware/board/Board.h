#ifndef BOARD_H_
#define BOARD_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "../adc/ADC.h"
#include "../../Types.h"
#include "../pins/Pins.h"
#include "../usb/usb_common.h"
#include "../usb/usb_private.h"

#define ANALOG_BUFFER_SIZE  MAX_NUMBER_OF_ANALOG

//function prototypes
inline void setAnalogPin(uint8_t muxNumber) __attribute__((always_inline));
inline void setMuxInput(uint8_t muxInput) __attribute__((always_inline));
inline void ledRowsOff() __attribute__((always_inline));
inline void ledRowOn(uint8_t rowNumber, uint8_t intensity) __attribute__((always_inline));
inline void checkLEDs() __attribute__((always_inline));
inline void setBlinkState(uint8_t ledNumber, bool state) __attribute__((always_inline));
inline void activateInputColumn(uint8_t column) __attribute__((always_inline));
inline void activateOutputColumn(uint8_t column) __attribute__((always_inline));
inline void storeDigitalIn(uint8_t column, uint8_t bufferIndex) __attribute__((always_inline));
inline encoderPosition readEncoder(uint8_t encoderID, uint8_t pairState) __attribute__((always_inline));

uint32_t rTimeMillis();
void wait(uint32_t time);
void disableWatchDog();

class Board {

    public:
    //init
    Board();
    void init();

    //digital
    bool buttonDataAvailable();
    bool encoderDataAvailable();
    bool getButtonState(uint8_t buttonIndex);
    encoderPosition getEncoderState(uint8_t encoderID);

    //analog
    bool analogDataAvailable();
    int16_t getAnalogValue(uint8_t analogID);

    //LEDs
    uint8_t getLEDstate(uint8_t ledNumber);
    void setLEDstate(uint8_t ledNumber, ledColor color, bool blinkMode);
    void setLEDblinkTime(uint16_t blinkTime);
    void setLEDTransitionSpeed(uint8_t transitionSteps);

    private:

    //init
    void initPins();
    void initAnalog();
    void configureTimers();
    void checkBlinkLEDs();
    void ledBlinkingStart();
    void ledBlinkingStop();
    bool ledBlinkingActive();
    void handleLED(uint8_t ledNumber, ledColor color, bool blinkMode, ledType type);

};

extern Board board;

#endif
