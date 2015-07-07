#include "Board.h"
#include "SysEx.h"

#include <Ownduino.h>

#include <avr/interrupt.h>
#include <util/delay.h>

//variables used by interrupts
volatile uint8_t    activeButtonColumn  = 0,
                    activeLEDColumn     = 0,
                    ledState[MAX_NUMBER_OF_LEDS],
                    activeMuxInput = 0,
                    tempEncoderState[NUMBER_OF_ENCODERS] = { 0 };

volatile int16_t    analogBuffer[DIGITAL_BUFFER_SIZE] = { -1 };
volatile int8_t     digitalBuffer[DIGITAL_BUFFER_SIZE] = { -1 };

volatile uint32_t   blinkTimerCounter = 0;
volatile int32_t    encoderPosition[NUMBER_OF_ENCODERS] = { 0 };

uint8_t             blinkEnabled = false,
                    blinkState = true,
                    analogueEnabledArray[8] = { 0 };

static const int8_t enc_states [] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

uint16_t            ledBlinkTime;

volatile bool changeSwitch = true;

volatile bool analogReadFinished = false;
volatile bool digitalReadFinished = false;

volatile int8_t activeMux = 0;

//inline functions

void Board::resetLEDblinkCounter()  {

    blinkTimerCounter = 0;

}

inline void setMuxInternal(uint8_t muxNumber)   {

    setADCchannel(analogueEnabledArray[muxNumber]);

}

inline void setMuxInputInteral(uint8_t muxInput)    {

    #ifdef BOARD_TANNIN
        //TO-DO
    #elif defined BOARD_OPENDECK_1
        PORTC &= 0xF8;
        PORTC |= muxInput;
        NOP;
    #endif

}

inline void ledRowsOff()   {

    #ifdef BOARD_TANNIN
    //TO-DO
    #elif defined BOARD_OPENDECK_1
    PORTB &= 0xF0;
    #endif

}

inline void ledRowOn(uint8_t rowNumber)  {

    #ifdef BOARD_TANNIN
    //TO-DO
    #elif defined BOARD_OPENDECK_1
    switch (rowNumber)  {

        case 0:
        //turn on first LED row
        PORTB |= 0x01;
        break;

        case 1:
        //turn on second LED row
        PORTB |= 0x02;
        break;

        case 2:
        //turn on third LED row
        PORTB |= 0x04;
        break;

        case 3:
        //turn on fourth LED row
        PORTB |= 0x08;
        break;

        default:
        break;

    }
    #endif

}

inline void checkLEDs()  {

    if (blinkEnabled)   {

        if (blinkTimerCounter == 0)  {

            //change blinkBit state and write it into ledState variable if LED is in blink state
            for (int i = 0; i<MAX_NUMBER_OF_LEDS; i++)  {

                if ((ledState[i] >> 1) & (0x01))    {

                    if (blinkState) ledState[i] |= 0x10;
                    else ledState[i] &= 0xEF;

                }

            }

            //invert blink state
            blinkState = !blinkState;

        }

        blinkTimerCounter++;
        if (blinkTimerCounter == ledBlinkTime) blinkTimerCounter = 0;

    }

    uint8_t ledNumber;

    //if there is an active LED in current column, turn on LED row
    for (int i=0; i<NUMBER_OF_LED_ROWS; i++)  {

        ledNumber = activeButtonColumn+i*NUMBER_OF_LED_COLUMNS;
        if  (

        ledState[ledNumber] == 0x05 ||
        ledState[ledNumber] == 0x15 ||
        ledState[ledNumber] == 0x16 ||
        ledState[ledNumber] == 0x1D ||
        ledState[ledNumber] == 0x0D ||
        ledState[ledNumber] == 0x17

        )   ledRowOn(i);

    }

}

inline void setBlinkState(uint8_t ledNumber, bool state)   {

    switch (state) {

        case true:
        ledState[ledNumber] |= 0x10;
        break;

        case false:
        ledState[ledNumber] &= 0xEF;
        break;

    }

}

inline void activateColumn()   {

    #ifdef BOARD_TANNIN
    //TO-DO
    #elif defined BOARD_OPENDECK_1

    //column switching is controlled by 74HC238 decoder
    PORTC &= 0xC7;
    switch (activeButtonColumn) {

        case 0:
        PORTC &= 0xC7;
        break;

        case 1:
        PORTC |= (0xC7 | 0x20);
        break;

        case 2:
        PORTC |= (0xC7 | 0x10);
        break;

        case 3:
        PORTC |= (0xC7 | 0x30);
        break;

        case 4:
        PORTC |= (0xC7 | 0x08);
        break;

        case 5:
        PORTC |= (0xC7 | 0x28);
        break;

        case 6:
        PORTC |= (0xC7 | 0x18);
        break;

        case 7:
        PORTC |= (0xC7 | 0x38);
        break;

        default:
        break;

    }

    #endif

}

inline void storeDigitalIn()  {

    uint8_t data = 0;

    #ifdef BOARD_TANNIN
    //TO-DO
    #elif defined BOARD_OPENDECK_1
    data = ((PIND >> 4) & 0x0F);
    #endif

    digitalBuffer[activeButtonColumn] = data;

    //uint16_t i = (unsigned int)(buffer->head + 1) % DIGITAL_BUFFER_SIZE;
//
    //if (i != buffer->tail) {
//
        //buffer->buffer[buffer->head] = data;
        //buffer->head = i;
//
    //}

}

inline void readEncoders()  {

    #ifdef BOARD_OPENDECK_1
    #ifdef TANNIN_2_PROTOTYPE
    uint8_t shiftRegisterState = 0;

    //pulse latch pin
    PORTB &= 0b11011111;
    NOP; NOP;
    PORTB |= 0b00100000;

    for (int i=0; i<8; i++) {

        shiftRegisterState = (shiftRegisterState << 1);
        shiftRegisterState |= ((PIND >> 3) & 0x01);
        //pulse clock pin
        PORTB |= 0b00010000;
        NOP; NOP;
        PORTB &= 0b11101111;

    }

    for (int i=0; i<NUMBER_OF_ENCODERS; i++)    {

        uint8_t tempState = tempEncoderState[i] & 0x03;
        uint8_t tempRegisterState = (shiftRegisterState >> (i*2)) & 0x03;

        if (tempRegisterState & 0x01)         tempState |= 4;
        if ((tempRegisterState >> 1) & 0x01)  tempState |= 8;

        tempEncoderState[i] = (tempState >> 2);

        switch (tempState) {

            case 1:
            case 7:
            case 8:
            case 14:
            encoderPosition[i]++;
            break;

            case 2:
            case 4:
            case 11:
            case 13:
            encoderPosition[i]--;
            break;

            case 3:
            case 12:
            encoderPosition[i] += 2;
            break;

            case 6:
            case 9:
            encoderPosition[i] -= 2;
            break;

        }

    }
    #else
    uint8_t encRead = (PIND >> 2) & 0x03;
    uint8_t tempState = tempEncoderState[0] & 0x03;

    if (encRead & 0x01)         tempState |= 4;
    if ((encRead >> 1) & 0x01)  tempState |= 8;

    tempEncoderState[0] = (tempState >> 2);

    switch (tempState) {

        case 1:
        case 7:
        case 8:
        case 14:
        encoderPosition[0]++;
        break;

        case 2:
        case 4:
        case 11:
        case 13:
        encoderPosition[0]--;
        break;

        case 3:
        case 12:
        encoderPosition[0] += 2;
        break;

        case 6:
        case 9:
        encoderPosition[0] -= 2;
        break;

    }
    #endif
    #endif

}

//init

Board::Board()  {

    //default constructor
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        ledState[i] = 0;

     ledBlinkTime = 0;
     totalLEDnumber = 0;

}

void Board::init()  {

    initPins();
    initAnalog();
    setUpMillisTimer();

    _delay_ms(5);

    setNumberOfColumnPasses();

    //configure column switch timer
    setUpTimer();

    //enable global interrupts
    sei();

}

void Board::initPins() {

    #ifdef BOARD_TANNIN
    //TO-DO
    #elif defined BOARD_OPENDECK_1
    DDRD = 0x02;

    #ifdef TANNIN_2_PROTOTYPE
        DDRB = 0x3F;       //tannin 2 mod
    #else
        DDRB = 0x0F;
    #endif

    DDRC = 0x3F;

    //enable internal pull-up resistors for button rows and encoder
    #ifdef TANNIN_2_PROTOTYPE
        PORTD = 0xF0;   //tannin 2 mod
    #else
        PORTD = 0xFC;
    #endif
    //select first column
    PORTC = 0x00;

    #ifdef TANNIN_2_PROTOTYPE
        //write high to latch pin
        PORTB |= 0b00100000;       //tannin 2 mod
    #endif

    #endif

}

void Board::initAnalog()    {

    setUpADC();
    setADCprescaler(32);

    #ifdef BOARD_TANNIN
    //TO-DO
    #elif defined BOARD_OPENDECK_1
    enableAnalogueInput(1, 6);
    enableAnalogueInput(0, 7);
    #endif

    setMuxInputInteral(activeMuxInput);
    setADCchannel(analogueEnabledArray[activeMux]);
    _delay_ms(5);
    getADCvalue();

}

void Board::enableAnalogueInput(uint8_t muxNumber, uint8_t adcChannel)  {

    analogueEnabledArray[muxNumber] = adcChannel;

    //disable digital input on enabled analog pins
    disconnectDigitalInADC(adcChannel);

}

void Board::setNumberOfColumnPasses() {

    /*

        Algorithm calculates how many times does it need to read whole row
        before it can declare button reading stable.

    */

    uint8_t rowPassTime = COLUMN_SCAN_TIME*NUMBER_OF_BUTTON_COLUMNS;
    uint8_t mod = 0;

    if ((MIN_BUTTON_DEBOUNCE_TIME % rowPassTime) > 0)   mod = 1;

    numberOfColumnPasses = ((MIN_BUTTON_DEBOUNCE_TIME / rowPassTime) + mod);

}

void Board::setUpTimer()   {

    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2  = 0;

    //turn on CTC mode
    TCCR2A |= (1 << WGM21);

    //set prescaler to 64
    TCCR2B |= (1 << CS22);

    //1ms
    OCR2A = 180;

    //enable CTC interrupt
    TIMSK2 |= (1 << OCIE2A);

}

uint8_t Board::getNumberOfColumnPasses()    {

    return numberOfColumnPasses;

}


//LEDs
void Board::setLEDstate(uint8_t ledNumber, uint8_t state)   {

    ledState[ledNumber] = state;

}

uint8_t Board::getLEDstate(uint8_t ledNumber)   {

    return ledState[ledNumber];

}

void Board::handleLED(bool currentLEDstate, bool blinkMode, uint8_t ledNumber) {

    /*

    LED state is stored into one byte (ledState). The bits have following meaning (7 being the MSB bit):

    7: x
    6: x
    5: x
    4: Blink bit (timer changes this bit)
    3: "Remember" bit, used to restore previous LED state
    2: LED is active (either it blinks or it's constantly on), this bit is OR function between bit 0 and 1
    1: LED blinks
    0: LED is constantly turned on

    */

    //if blink note is received, and blinking is disabled, exit the function
    //if (blinkMode && (!(bitRead(ledFeatures, SYS_EX_FEATURES_LEDS_BLINK))))
        //return;

    if (ledNumber < 128)    {

        switch (currentLEDstate) {

            case false:
            //note off event

            //if remember bit is set
            if ((ledState[ledNumber] >> 3) & (0x01))   {

                //if note off for blink state is received
                //clear remember bit and blink bits
                //set constant state bit
                if (blinkMode) ledState[ledNumber] = 0x05;
                //else clear constant state bit and remember bit
                //set blink bits
                else           ledState[ledNumber] = 0x16;

                }   else    {

                if (blinkMode)  ledState[ledNumber] &= 0x15; /*clear blink bit */
                else            ledState[ledNumber] &= 0x16; /* clear constant state bit */

            }

            //if bits 0 and 1 are 0, LED is off so we set ledState to zero
            if (!(ledState[ledNumber] & 3)) ledState[ledNumber] = 0;

            break;

            case true:
            //note on event

            //if constant note on is received and LED is already blinking
            //clear blinking bits and set remember bit and constant bit
            if ((!blinkMode) && checkBlinkState(ledNumber)) ledState[ledNumber] = 0x0D;

            //set bit 2 to 1 in any case (constant/blink state)
            else    ledState[ledNumber] |= (0x01 << blinkMode) | 0x04 | (blinkMode << 4);

        }

    }

    if (blinkMode && currentLEDstate)   blinkEnabled = true;
    else    checkBlinkLEDs();

}

void Board::checkBlinkLEDs() {

    //this function will disable blinking
    //if none of the LEDs is in blinking state

    //else it will enable it

    bool _blinkEnabled = false;

    //if any LED is blinking, set timerState to true and exit the loop
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)

    if (checkBlinkState(i)) {

        _blinkEnabled = true;
        break;

    }

    if (_blinkEnabled)  blinkEnabled = true;

    //don't bother reseting variables if blinking is already disabled
    else    if (!_blinkEnabled && blinkEnabled) {

        //reset blinkState to default value
        blinkState = true;
        cli();
        blinkTimerCounter = 0;
        sei();
        blinkEnabled = false;

    }

}

bool Board::checkBlinkState(uint8_t ledNumber)   {

    //function returns true if blinking bit in ledState is set
    return ((ledState[ledNumber] >> 1) & (0x01));

}

void Board::setLEDblinkTime(uint16_t time)  {

    ledBlinkTime = time;

}

void Board::turnOnLED(uint8_t ledNumber)    {

    setLEDstate(ledNumber, 0x05);

}

void Board::turnOffLED(uint8_t ledNumber)   {

    setLEDstate(ledNumber, 0);

}


//digital
uint8_t Board::getActiveColumn()    {

    return (activeButtonColumn-1);

}

bool Board::digitalInDataAvailable() {

    return (changeSwitch && !digitalReadFinished);

}

int8_t Board::getDigitalInData()   {

    return digitalBuffer[getActiveColumn()];

}

void Board::setDigitalProcessingFinished(bool state)  {

    digitalReadFinished = state;

}

//analog
void Board::setMux(uint8_t muxNumber)   {

    setMuxInternal(muxNumber);

}

void Board::setMuxInput(uint8_t muxInput)   {

    setMuxInputInteral(muxInput);

}

bool Board::analogInDataAvailable() {

    return (!changeSwitch && !analogReadFinished);

}

int16_t Board::getMuxInputValue(uint8_t analogID) {

    setMuxInputInteral(analogID);
    return getADCvalue();

}

uint8_t Board::getAnalogID(uint8_t id)  {

    int8_t _activeMux = activeMux;

    //return currently active column
    return id+((_activeMux - 1)*8);

}

void Board::setAnalogProcessingFinished(bool state) {

    analogReadFinished = state;

}

//encoders
int32_t Board::getEncoderState(uint8_t encoderNumber)  {

    int32_t returnValue = 0;
    cli();
    returnValue = encoderPosition[encoderNumber];
    sei();

    return returnValue;

}

//timer
ISR(TIMER2_COMPA_vect)  {

    readEncoders();
    bool _changeSwitch = changeSwitch;

    switch(_changeSwitch)    {

        case true:
        //switch column
        if (activeButtonColumn == NUMBER_OF_BUTTON_COLUMNS) activeButtonColumn = 0;
        //turn off all LED rows before switching to next column
        ledRowsOff();
        activateColumn();
        checkLEDs();
        storeDigitalIn();
        activeButtonColumn++;
        digitalReadFinished = false;
        break;

        case false:
        //switch analogue input
        if (activeMux == NUMBER_OF_MUX) activeMux = 0;
        setMuxInternal(activeMux);
        activeMux++;
        analogReadFinished = false;
        break;

        default:
        break;

    }

    _changeSwitch = !_changeSwitch;
    changeSwitch = _changeSwitch;

}


Board boardObject;