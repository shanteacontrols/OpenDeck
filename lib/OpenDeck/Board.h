#ifndef BOARD_H_
#define BOARD_H_

#include <avr/io.h>
#include "Ownduino.h"

//#include "BoardDefsTannin.h"
#include "BoardDefsOpenDeck.h"

#define MIN_BUTTON_DEBOUNCE_TIME    20
#define COLUMN_SCAN_TIME            1

#define MAX_NUMBER_OF_ANALOG        16
#define MAX_NUMBER_OF_BUTTONS       32
#define MAX_NUMBER_OF_LEDS          32
#define MAX_NUMBER_OF_ENCODERS      16

#define ENCODER_DIRECTION_LEFT      -1
#define ENCODER_DIRECTION_RIGHT     1
#define ENCODER_STOPPED             0

//function prototypes
inline void setMux(uint8_t muxNumber) __attribute__((always_inline));
inline void setMuxInput(uint8_t muxInput) __attribute__((always_inline));
inline void storeAnalogIn(int16_t value) __attribute__((always_inline));
inline void ledRowsOff() __attribute__((always_inline));
inline void ledRowOn(uint8_t rowNumber) __attribute__((always_inline));
inline void checkLEDs() __attribute__((always_inline));
inline void setBlinkState(uint8_t ledNumber, bool state) __attribute__((always_inline));
inline void activateColumn() __attribute__((always_inline));
inline void storeDigitalIn(ring_buffer *buffer) __attribute__((always_inline));

struct ring_buffer;

class Board {

    public:
    //init
    Board();
    void init();
    void configureLongPress(uint8_t);

    //LEDs
    uint8_t getLEDstate(uint8_t);
    void setLEDstate(uint8_t, uint8_t);
    void setBlinkState(bool);
    void handleLED(bool, bool, uint8_t);
    void setLEDblinkTime(uint16_t);
    void turnOnLED(uint8_t);
    void turnOffLED(uint8_t);

    //getters
    uint16_t digitalInDataAvailable();
    int16_t getDigitalInData();
    bool analogInDataAvailable();
    int16_t getAnalogInData(uint8_t);
    uint8_t getLongPressColumnPass();
    uint8_t getNumberOfColumnPasses();
    int32_t getEncoderState(uint8_t encoderNumber);

    //setters
    void startAnalogConversion();

    private:

    //init
    void initPins();
    void initAnalog();

    //analog
    void enableAnalogueInput(uint8_t, uint8_t);

    //timers
    void setUpMatrixTimer();
    void setUpEncoderTimer();

    //digital in
    void setNumberOfColumnPasses();

    //LEDs
    bool checkBlinkState(uint8_t);
    void checkBlinkLEDs();
    void switchBlinkState();

    //variables
    uint8_t numberOfColumnPasses,
            longPressColumnPass,
            totalLEDnumber,
            startUpLEDswitchTime;

    ring_buffer *_buttonReadings;

};

extern Board boardObject;

#endif /* BOARD_H_ */