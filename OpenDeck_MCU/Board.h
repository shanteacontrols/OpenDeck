#ifndef BOARD_H_
#define BOARD_H_

#include <Arduino.h>
#include <avr/io.h>
#include "ADC.h"
#include "EncoderSettings.h"
#include "Types.h"

#include "BoardDefsTannin.h"
//#include "BoardDefsOpenDeck.h"

#define COLUMN_SCAN_TIME            1500    //microseconds
#define MIN_BUTTON_DEBOUNCE_TIME    20      //milliseconds

#define MAX_NUMBER_OF_ANALOG        (NUMBER_OF_MUX*8)
#define MAX_NUMBER_OF_BUTTONS       (NUMBER_OF_BUTTON_COLUMNS*NUMBER_OF_BUTTON_ROWS)
#define MAX_NUMBER_OF_LEDS          (NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS)
#define MAX_NUMBER_OF_ENCODERS      NUMBER_OF_ENCODERS

#define ANALOG_BUFFER_SIZE          8+1 //extra byte to write current mux

//function prototypes
inline void setMuxInternal(uint8_t muxNumber) __attribute__((always_inline));
inline void setMuxInputInteral(uint8_t muxInput) __attribute__((always_inline));
inline void storeAnalogIn(int16_t value) __attribute__((always_inline));
inline void ledRowsOff() __attribute__((always_inline));
inline void ledRowOn(uint8_t rowNumber, uint8_t intensity) __attribute__((always_inline));
inline void checkLEDs() __attribute__((always_inline));
inline void setBlinkState(uint8_t ledNumber, bool state) __attribute__((always_inline));
inline void activateColumn() __attribute__((always_inline));
inline void storeDigitalIn() __attribute__((always_inline));
inline void readEncoders() __attribute__((always_inline));

class Board {

    public:
    //init
    Board();
    void init();

    uint32_t newMillis();
    void newDelay(uint32_t delayTime);
    void rebootBoard();

    //digital
    uint8_t buttonDataAvailable();
    uint8_t getNumberOfColumnPasses();
    bool getButtonState(uint8_t buttonIndex);
    uint8_t getButtonNumber(uint8_t buttonIndex);
    encoderPosition getEncoderState(uint8_t encoderNumber);

    //analog
    uint8_t analogDataAvailable();
    int16_t getAnalogValue(uint8_t analogID);
    uint8_t getAnalogID(uint8_t id);

    //LEDs
    uint8_t getLEDstate(uint8_t);
    void setLEDstate(uint8_t, uint8_t);
    void setBlinkState(bool);
    void handleLED(bool, bool, uint8_t);
    void setLEDblinkTime(uint16_t);
    void turnOnLED(uint8_t);
    void turnOffLED(uint8_t);
    void resetLEDblinkCounter();
    void resetLEDtransitions();
    void setLEDTransitionSpeed(uint8_t);

    private:

    //init
    void initPins();
    void initAnalog();
    void configurePWM();
    void setUpTimer1();
    void setUpTimer4();

    //analog
    void enableAnalogueInput(uint8_t, uint8_t);

    //timers
    void setUpTimer();

    //digital in
    void setNumberOfColumnPasses();

    //LEDs
    bool checkBlinkState(uint8_t);
    void checkBlinkLEDs();
    void switchBlinkState();

    //variables
    uint8_t numberOfColumnPasses;

};

extern Board boardObject;

#endif