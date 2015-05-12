#include "Board.h"
#include "SysEx.h"

#include <avr/interrupt.h>
#include <util/delay.h>

#define BUFFER_SIZE 32

//variables used by interrupts
volatile uint8_t    activeButtonColumn  = 0,
                    activeLEDColumn     = 0,
                    ledState[MAX_NUMBER_OF_LEDS],
                    analogReadFinishedCounter = 0,
                    activeMux = 0,
                    activeMuxInput = 0,
                    tempEncoderState[NUMBER_OF_ENCODERS] = { 0 };

volatile int16_t    analogBuffer[MAX_NUMBER_OF_ANALOG] = { -1 };

volatile uint32_t   blinkTimerCounter = 0;
volatile int32_t    encoderPosition[NUMBER_OF_ENCODERS] = { 0 };

uint8_t             blinkEnabled = false,
                    blinkState = true,
                    analogueEnabledArray[8] = { 0 };

static const int8_t enc_states [] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

uint16_t            ledBlinkTime;

struct ring_buffer  {

    int16_t buffer[BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;

};

ring_buffer buttonReadings = { { 0 }, 0, 0};

//inline functions

inline void setMux(uint8_t muxNumber)   {

    setADCchannel(analogueEnabledArray[muxNumber]);

}

inline void setMuxInput()    {

    #ifdef BOARD_TANNIN
        //TO-DO
    #elif defined BOARD_OPENDECK_1
        PORTC &= 0xF8;
        PORTC |= activeMuxInput;
    #endif

}

inline void storeAnalogIn(int16_t value)  {

    analogBuffer[analogReadFinishedCounter] = value;
    analogReadFinishedCounter++;

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

inline void storeDigitalIn(ring_buffer *buffer)  {

    uint16_t data = 0;

    #ifdef BOARD_TANNIN
    //TO-DO
    #elif defined BOARD_OPENDECK_1
    data = ((PIND >> 4) & 0x0F);
    #endif

    data = data << 8;
    data |= activeButtonColumn;

    uint16_t i = (unsigned int)(buffer->head + 1) % BUFFER_SIZE;

    // if we should be storing the received character into the location
    // just before the tail (meaning that the head would advance to the
    // current location of the tail), we're about to overflow the buffer
    // and so we don't write the character or advance the head.

    if (i != buffer->tail) {

        buffer->buffer[buffer->head] = data;
        buffer->head = i;

    }

}

//init

Board::Board()  {

    //default constructor
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        ledState[i] = 0;

     ledBlinkTime = 0;
     totalLEDnumber = 0;

     _buttonReadings = &buttonReadings;

}

void Board::init()  {

    initPins();
    initAnalog();

    _delay_ms(5);

    setNumberOfColumnPasses();

    //configure column switch timer
    setUpMatrixTimer();

    #ifdef BOARD_OPENDECK_1
        //configure encoder read timer
        setUpEncoderTimer();
    #endif

}

void Board::initPins() {

    #ifdef BOARD_TANNIN
    //TO-DO
    #elif defined BOARD_OPENDECK_1
    DDRD = 0x02;
    DDRB = 0x0F;
    DDRC = 0x3F;

    //enable internal pull-up resistors for button rows and encoder
    PORTD = 0xFC;

    //select first column
    PORTC = 0x00;
    #endif

}

void Board::initAnalog()    {

    setADCprescaler(32);
    set8bitADC();
    enableADCinterrupt();

    #ifdef BOARD_TANNIN
    //TO-DO
    #elif defined BOARD_OPENDECK_1
    enableAnalogueInput(1, 6);
    enableAnalogueInput(0, 7);
    #endif

    startAnalogConversion();

}

void Board::enableAnalogueInput(uint8_t muxNumber, uint8_t adcChannel)  {

    analogueEnabledArray[muxNumber] = adcChannel;

    //disable digital input on enabled analog pins
    disconnectDigitalInADC(adcChannel);

}

void Board::startAnalogConversion()  {

    activeMuxInput = 0;
    activeMux = 0;
    setMux(activeMux);
    setMuxInput();
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analogBuffer[i] = -1;

    analogReadFinishedCounter = 0;
    ADCSRA |= (1<<ADSC);

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

void Board::setUpMatrixTimer()   {

    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2  = 0;

    //turn on CTC mode
    TCCR2A |= (1 << WGM21);

    //set prescaler to 64
    TCCR2B |= (1 << CS22);

    //1ms
    OCR2A = 249;

    //enable CTC interrupt
    TIMSK2 |= (1 << OCIE2A);

}

void Board::setUpEncoderTimer() {

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    //turn on CTC mode
    TCCR1B |= (1 << WGM12);

    //set prescaler to 64
    TCCR1B |= (1 << CS11)|(1<<CS10);

    //500us
    OCR1A = 124;

    //enable CTC interrupt
    TIMSK1 |= (1 << OCIE1A);

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


//buttons
uint16_t Board::digitalInDataAvailable() {

    //save interrupt flag
    uint8_t interruptFlag = SREG;

    //disable interrupts
    cli();

    uint16_t bytesAvailable = ((BUFFER_SIZE + _buttonReadings->head - _buttonReadings->tail) % BUFFER_SIZE);

    SREG = interruptFlag;

    return bytesAvailable;

}

int16_t Board::getDigitalInData()   {

    int16_t returnValue;

    //save interrupt flag
    uint8_t interruptFlag = SREG;

    //disable interrupts
    cli();

    // if the head isn't ahead of the tail, we don't have any characters
    if (_buttonReadings->head == _buttonReadings->tail) returnValue = -1;

    else {

        uint16_t data = _buttonReadings->buffer[_buttonReadings->tail];
        _buttonReadings->tail = (uint16_t)(_buttonReadings->tail + 1) % BUFFER_SIZE;
        returnValue = data;

    }

    SREG = interruptFlag;
    return returnValue;

}

void Board::configureLongPress(uint8_t longPressTime) {

    longPressColumnPass = longPressTime*100 / (COLUMN_SCAN_TIME*NUMBER_OF_BUTTON_COLUMNS);

}

uint8_t Board::getLongPressColumnPass()    {

    return longPressColumnPass;

}


//analog
bool Board::analogInDataAvailable() {

    return (analogReadFinishedCounter == MAX_NUMBER_OF_ANALOG);

}

int16_t Board::getAnalogInData(uint8_t potNumber)   {

    return analogBuffer[potNumber];

}


//encoders
int32_t Board::getEncoderState(uint8_t encoderNumber)  {

    int32_t returnValue = 0;
    cli();
    returnValue = encoderPosition[0];
    sei();

    return returnValue;

}


//ISR
ISR(ADC_vect)   {

    analogBuffer[analogReadFinishedCounter] = ADCH;
    analogReadFinishedCounter++;

    if (!(analogReadFinishedCounter == MAX_NUMBER_OF_ANALOG))  {

        activeMuxInput++;

        if (activeMuxInput == 8) {

            activeMuxInput = 0;
            activeMux++;
            if (activeMux == NUMBER_OF_MUX) activeMux = 0;
            ADMUX = (ADMUX & 0xF0) | (analogueEnabledArray[activeMux] & 0x0F);
            NOP;

        }

        setMuxInput();

        ADCSRA |= (1<<ADSC);

    }

}

ISR(TIMER1_COMPA_vect)  {

    #ifdef BOARD_OPENDECK_1
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

}

ISR(TIMER2_COMPA_vect)  {

    //switch column
    if (activeButtonColumn == NUMBER_OF_BUTTON_COLUMNS) activeButtonColumn = 0;
    //turn off all LED rows before switching to next column

    ledRowsOff();
    activateColumn();

    checkLEDs();
    storeDigitalIn(&buttonReadings);
    activeButtonColumn++;

}


Board boardObject;