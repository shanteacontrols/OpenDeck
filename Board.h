#ifndef BOARD_H_
#define BOARD_H_

#include <avr/io.h>
#include <Ownduino.h>

//#include "BoardDefsTannin.h"
#include "BoardDefsOpenDeck.h"

#define DIGITAL_BUFFER_SIZE         8
#define ANALOG_BUFFER_SIZE          16

#define MIN_BUTTON_DEBOUNCE_TIME    15
#define COLUMN_SCAN_TIME            1

#define MAX_NUMBER_OF_ANALOG        16
#define MAX_NUMBER_OF_BUTTONS       32
#define MAX_NUMBER_OF_LEDS          32
#define MAX_NUMBER_OF_ENCODERS      16

#define ENCODER_DIRECTION_LEFT      -1
#define ENCODER_DIRECTION_RIGHT     1
#define ENCODER_STOPPED             0

//function prototypes
inline void setMuxInternal(uint8_t muxNumber) __attribute__((always_inline));
inline void setMuxInputInteral(uint8_t muxInput) __attribute__((always_inline));
inline void storeAnalogIn(int16_t value) __attribute__((always_inline));
inline void ledRowsOff() __attribute__((always_inline));
inline void ledRowOn(uint8_t rowNumber) __attribute__((always_inline));
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

    //digital
    bool digitalInDataAvailable();
    void setDigitalProcessingFinished(bool state);
    uint8_t getActiveColumn();
    int8_t getDigitalInData();
    uint8_t getNumberOfColumnPasses();
    int32_t getEncoderState(uint8_t encoderNumber);

    //analog
    bool analogInDataAvailable();
    void setAnalogProcessingFinished(bool state);
    int16_t getAnalogValue(uint8_t analogID);
    uint8_t getAnalogID(uint8_t id);
    void setMux(uint8_t muxNumber);
    void setMuxInput(uint8_t muxInput);

    //LEDs
    uint8_t getLEDstate(uint8_t);
    void setLEDstate(uint8_t, uint8_t);
    void setBlinkState(bool);
    void handleLED(bool, bool, uint8_t);
    void setLEDblinkTime(uint16_t);
    void turnOnLED(uint8_t);
    void turnOffLED(uint8_t);
    void resetLEDblinkCounter();

    private:

    //init
    void initPins();
    void initAnalog();

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
    uint8_t numberOfColumnPasses,
            totalLEDnumber,
            startUpLEDswitchTime;

};

extern Board boardObject;

#endif /* BOARD_H_ */