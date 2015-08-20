#include "Board.h"
#include "SysEx.h"

#include "LEDsettings.h"
#include "EEPROM.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/cpufunc.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include "Types.h"


//matrix columns
volatile uint8_t            activeButtonColumn  = 0;

//buttons
volatile bool               _buttonDataAvailable = false;
volatile uint16_t           digitalBuffer = 0;
uint16_t                    digitalBufferCopy = 0;

//encoders
bool                        encoderDirection[MAX_NUMBER_OF_ENCODERS] = { 0 };
volatile uint8_t            tempEncoderState[NUMBER_OF_ENCODERS] = { 0 };
static const int8_t         encoderLookUpTable[] = { 0, 1, -1, 2, -1, 0, -2, 1, 1, -2, 0, -1, 2, -1, 1, 0 };
int32_t                     encoderPulses[MAX_NUMBER_OF_ENCODERS] = { 0 };
volatile encoderPosition    encoderMoving[MAX_NUMBER_OF_ENCODERS] = { encStopped };

//LEDs
bool                        blinkEnabled = false,
                            blinkState = true;
volatile uint8_t            pwmSteps = eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_FADE_SPEED),
                            ledState[MAX_NUMBER_OF_LEDS];
static const uint8_t        ledOnLookUpTable[] = { 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0 };
uint16_t                    ledBlinkTime;
int16_t                     transitionCounter[MAX_NUMBER_OF_LEDS] = { 0 };
volatile uint32_t           blinkTimerCounter = 0;

//analog
volatile bool               _analogDataAvailable = false;
uint8_t                     analogueEnabledArray[8] = { 0 };
volatile uint8_t            activeMux = 0,
                            activeMuxInput = 0;
volatile int16_t            analogBuffer[ANALOG_BUFFER_SIZE] = { 0 };
int16_t                     analogBufferCopy[ANALOG_BUFFER_SIZE] = { 0 };

//millis
volatile uint32_t           rTime_ms = 0;


//inline functions

inline void setMuxInternal(uint8_t muxNumber)   {

    setADCchannel(analogueEnabledArray[muxNumber]);

}

inline void setMuxInputInteral(uint8_t muxInput)    {

    #ifdef BOARD_TANNIN
        PORTF &= 0b00011111;
        PORTF |= (muxInput << 5);
        _NOP();
    #elif defined BOARD_OPENDECK_1
        PORTC &= 0xF8;
        PORTC |= muxInput;
        _NOP();
    #endif

}

inline void ledRowsOff()   {

    #ifdef BOARD_TANNIN
        //turn off PWM
        TCCR1A &= ~(1<<COM1C1); //pin 4
        TCCR0A &= ~(1<<COM0B1); //pin 5
        TCCR1A &= ~(1<<COM1A1); //pin A7
        TCCR1A &= ~(1<<COM1B1); //pin A6

        //turn off pins
        PORTB &= 0x1F;
        PORTD &= 0xFE;
    #elif defined BOARD_OPENDECK_1
        PORTB &= 0xF0;
    #endif

}

inline void ledRowOn(uint8_t rowNumber, uint8_t intensity)  {

    #ifdef BOARD_TANNIN
        switch (rowNumber)  {

            case 0:
            if (intensity)  {

                OCR1C = intensity;
                TCCR1A |= (1<<COM1C1); //pin 4

            }   else {

                TCCR1A &= ~(1<<COM1C1);
                PORTB &= 0b01111111;

            }
            break;

            case 1:
            if (intensity)  {

                OCR0B = intensity;
                TCCR0A |= (1<<COM0B1); //pin 5

            }   else {

                TCCR0A &= ~(1<<COM0B1);
                PORTD &= 0b11111110;

            }
            break;

            case 2:
            if (intensity)  {

                OCR1A = intensity;
                TCCR1A |= (1<<COM1A1); //pin A7

            }   else {

                TCCR1A &= ~(1<<COM1A1);
                PORTB &= 0b11011111;

            };
            break;

            case 3:
            if (intensity)  {

                OCR1B = intensity;
                TCCR1A |= (1<<COM1B1); //pin A6

            }   else {

                TCCR1A &= ~(1<<COM1B1);
                PORTB &= 0b10111111;

            }
            break;

            default:
            break;

        }
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

        if (!blinkTimerCounter)  {

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

    //if there is an active LED in current column, turn on LED row
    #if defined (BOARD_OPENDECK_1)
        //don't bother with LED transitions as PWM is not supported on OpenDeck board yet
        for (int i=0; i<NUMBER_OF_LED_ROWS; i++)  {

            uint8_t ledNumber = activeButtonColumn+i*NUMBER_OF_LED_COLUMNS;
            if (ledOnLookUpTable[ledState[ledNumber]])  ledRowOn(i, 255);   //value ignored

        }
    #elif defined BOARD_TANNIN
        //do fancy transitions here
        for (int i=0; i<NUMBER_OF_LED_ROWS; i++)  {

            uint8_t ledNumber = activeButtonColumn+i*NUMBER_OF_LED_COLUMNS;
            uint8_t ledStateSingle = ledOnLookUpTable[ledState[ledNumber]];

            if (!pwmSteps && ledStateSingle) ledRowOn(i, 255); //don't bother with pwm if it's disabled
            else {

                if (
                (ledStateSingle && (transitionCounter[ledNumber] != (NUMBER_OF_TRANSITIONS-1))) ||
                (!ledStateSingle && transitionCounter[ledNumber])
                )  {

                    //skip this check if led state is off and transition counter is 0 or
                    //led is on and transition counter has max value

                    if (ledStateSingle) transitionCounter[ledNumber] += pwmSteps;
                    else transitionCounter[ledNumber] -= pwmSteps;

                    if (transitionCounter[ledNumber] >= NUMBER_OF_TRANSITIONS) transitionCounter[ledNumber] = NUMBER_OF_TRANSITIONS-1;
                    if (transitionCounter[ledNumber] < 0) transitionCounter[ledNumber] = 0;

                }

            }   if (transitionCounter[ledNumber]) ledRowOn(i, ledTransitionScale[transitionCounter[ledNumber]]);

        }
    #endif

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

inline void activateColumn(uint8_t column)   {

    #ifdef BOARD_TANNIN
        PORTD &= 0b000011111;
        PORTD |= (column << 5);
    #elif defined BOARD_OPENDECK_1
        //column switching is controlled by 74HC238 decoder
        PORTC &= 0xC7;
        switch (column) {

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

inline void activateEncoderColumn(uint8_t column)   {

    PORTC |= 0b11000000;

    switch (column) {

        case 0:
        PORTC &= 0b10111111;
        _NOP();
        break;

        case 1:
        PORTC &= 0b01111111;
        _NOP();
        break;

        default:
        break;

    }

}

inline void storeDigitalIn()  {

    uint16_t data = 0;

    #ifdef BOARD_TANNIN
        //pulse latch pin
        PORTD &= 0b11101111;
        _NOP();
        PORTD |= 0b00010000;

        for (int i=0; i<8; i++) {

            data = (data << 1);
            data |= ((PIND >> 1) & 0x01);
            //pulse clock pin
            PORTB |= 0b00010000;
            _NOP();
            PORTB &= 0b11101111;

        }
    #elif defined BOARD_OPENDECK_1
        data = ((PIND >> 4) & 0x0F);
    #endif

    digitalBuffer = data;
    digitalBuffer <<= 8;
    digitalBuffer |= (uint16_t)activeButtonColumn;

}

inline void readEncoders()  {

    #if defined (BOARD_OPENDECK_1)

        uint8_t encRead = (PIND >> 2) & 0x03;
        uint8_t encoderNumber = 0;

        tempEncoderState[encoderNumber] = ((tempEncoderState[encoderNumber] << 2) | encoderValue) & 0x0F;
        bool lastEncoderDirection = encoderDirection[encoderNumber];

        int8_t encRead = encoderLookUpTable[tempEncoderState[encoderNumber]];

        if (encRead)    {

            encoderPulses[encoderNumber] += encRead;
            encoderDirection[encoderNumber] = encRead > 0;

            if (lastEncoderDirection == encoderDirection[encoderNumber])    {

                if ((encoderPulses[encoderNumber] % PULSES_PER_STEP) == 0)   {

                    if (lastEncoderDirection) encoderMoving[encoderNumber] = encMoveLeft;
                    else encoderMoving[encoderNumber] = encMoveRight;

                }

            }

        }

    #elif defined (BOARD_TANNIN)

        //encoders are setup in matrix setup, like buttons

        uint8_t columnState;

        for (int i=0; i<NUMBER_OF_ENCODER_COLUMNS; i++) {

            activateEncoderColumn(i);
            columnState = PINB & 0x0F;  //two encoder states

            for (int j=0; j<NUMBER_OF_ENCODER_ROWS; j++)    {

                bool rowEven = (j % 2 == 0);
                if (!rowEven) continue;

                uint8_t matrixIndex = i+j*NUMBER_OF_ENCODER_COLUMNS;
                uint8_t encoderNumber = matrixIndex - j;

                uint8_t encoderValue = (columnState >> j) & 0x03;

                tempEncoderState[encoderNumber] = ((tempEncoderState[encoderNumber] << 2) | encoderValue) & 0x0F;
                bool lastEncoderDirection = encoderDirection[encoderNumber];

                int8_t encRead = encoderLookUpTable[tempEncoderState[encoderNumber]];

                if (encRead)    {

                    encoderPulses[encoderNumber] += encRead;
                    encoderDirection[encoderNumber] = encRead > 0;

                    if (lastEncoderDirection == encoderDirection[encoderNumber])    {

                        if ((encoderPulses[encoderNumber] % PULSES_PER_STEP) == 0)   {

                            if (lastEncoderDirection) encoderMoving[encoderNumber] = encMoveLeft;
                            else encoderMoving[encoderNumber] = encMoveRight;

                        }

                    }

                }

            }

        }

    #endif

}


//ISR functions

#if defined(BOARD_OPENDECK_1)
ISR(TIMER2_COMPA_vect)  {

    //switch column
    if (activeButtonColumn == NUMBER_OF_BUTTON_COLUMNS) activeButtonColumn = 0;
    //turn off all LED rows before switching to next column
    ledRowsOff();
    activateColumn(activeButtonColumn);
    checkLEDs();
    storeDigitalIn();
    activeButtonColumn++;
    _buttonDataAvailable = true;

}

ISR(TIMER1_COMPA_vect) {

    static bool updateMS = true;
    uint32_t ms;

    updateMS = !updateMS;

    if (updateMS)   {

        ms = rTime_ms;
        ms++;
        //update run time
        rTime_ms = ms;

    }

    readEncoders();

}
#elif defined (BOARD_TANNIN)
ISR(TIMER3_COMPA_vect)  {

    //switch column
    if (activeButtonColumn == NUMBER_OF_BUTTON_COLUMNS) activeButtonColumn = 0;
    //turn off all LED rows before switching to next column
    ledRowsOff();
    activateColumn(activeButtonColumn);
    checkLEDs();
    storeDigitalIn();
    activeButtonColumn++;
    _buttonDataAvailable = true;

}

ISR(TIMER4_OVF_vect) {

    static bool updateMS = true;
    uint32_t ms;

    updateMS = !updateMS;

    if (updateMS)   {

        ms = rTime_ms;
        ms++;
        //update run time
        rTime_ms = ms;

    }

    readEncoders();

}
#endif

ISR(ADC_vect)   {

    analogBuffer[activeMuxInput] = ADC;
    activeMuxInput++;

    bool startConversion = activeMuxInput != 8;

    if (!startConversion)    {

        analogBuffer[ANALOG_BUFFER_SIZE-1] = activeMux;
        activeMuxInput = 0;
        activeMux++;
        if (activeMux == NUMBER_OF_MUX) activeMux = 0;
        _analogDataAvailable = true;
        ADMUX = (ADMUX & 0xF0) | (analogueEnabledArray[activeMux] & 0x0F);

    }

    //always set mux input
    setMuxInputInteral(activeMuxInput);
    if (startConversion) ADCSRA |= (1<<ADSC);

}


//init

Board::Board()  {

    //default constructor

}

void Board::init()  {

    cli();
    initPins();
    initAnalog();

    _delay_ms(5);

    #if defined (BOARD_TANNIN)
        setUpTimer4();
        configurePWM();
    #elif defined (BOARD_OPENDECK_1)
        setUpTimer1();
    #endif

    //configure column switch timer
    setUpTimer();

    //enable global interrupts
    sei();

}

void Board::initPins() {

    #ifdef BOARD_TANNIN
        DDRF = 0b11100000;
        DDRB = 0b11110000;
        DDRD = 0b11111001;
        DDRC = 0b11000000;

        //encoder pull-ups (rows)
        PORTB |= 0b00001111;

        //turn off encoder columns
        PORTC |= 0b11000000;
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

    setUpADC();
    setADCprescaler(64);

    #ifdef BOARD_TANNIN
        enableAnalogueInput(0, 4);
        enableAnalogueInput(1, 1);
    #elif defined BOARD_OPENDECK_1
        enableAnalogueInput(1, 6);
        enableAnalogueInput(0, 7);
    #endif

    setMuxInputInteral(activeMuxInput);
    setADCchannel(analogueEnabledArray[activeMux]);
    _delay_ms(5);
    getADCvalue();  //dummy read to init ADC
    enableADCinterrupt();
    startADCconversion();   //start ADC conversions

}

void Board::enableAnalogueInput(uint8_t muxNumber, uint8_t adcChannel)  {

    analogueEnabledArray[muxNumber] = adcChannel;

    //disable digital input on enabled analog pins
    disconnectDigitalInADC(adcChannel);

}

#ifdef BOARD_TANNIN

void Board::configurePWM()   {

    //stop default timer0 interrupt
    TCCR0A = 0;
    TCCR0B = 0;

    //increase default PWM frequency to 31kHz
    TCCR1B = (TCCR1B & 0b11111000) | 0x01;
    TCCR1A = (TCCR1A & 0b11111000) | 0x01;
    TCCR0B = (TCCR0B & 0b11111000) | 0x01;
    TCCR0A = (TCCR0A & 0b11111000) | 0x01;

}

void Board::setUpTimer4()   {

    TCCR4B = 0;
    TCCR4A = 0;
    TCCR4C = 0;
    TCCR4D = 0;
    TCCR4E = 0;

    TCCR4B = (1<<CS42) | (1<<CS41) | (1<<CS40) | (1<<PSR4);

    OCR4C = 124;

    TIFR4 = (1<<TOV4);
    TCNT4 = 0;
    TIMSK4 = (1<<TOIE4);

}
#elif defined (BOARD_OPENDECK_1)
void Board::setUpTimer1() {

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1H = 0;
    TCNT1L = 0;

    //turn on CTC mode
    TCCR1B |= (1 << WGM12);

    //set prescaler to 64
    TCCR1B |= (1 << CS11)|(1 << CS10);

    //1ms
    OCR1A = 124;

    //enable CTC interrupt
    TIMSK1 |= (1 << OCIE1A);

}

#endif

void Board::setUpTimer()   {

    #if defined (TCCR2A) && defined (TCCR2B)
        TCCR2A = 0;
        TCCR2B = 0;
        TCNT2  = 0;

        //turn on CTC mode
        TCCR2A |= (1 << WGM21);

        //set prescaler to 64
        TCCR2B |= (1 << CS22);

        //using COLUMN_SWITCH_TIME to calculate timer count
        //known constants: F_CPU/16 MHz, prescaler/64

        uint32_t timerCount = COLUMN_SCAN_TIME/4;

        OCR2A = timerCount;

        //enable CTC interrupt
        TIMSK2 |= (1 << OCIE2A);
    #elif defined (TCCR3A) && defined (TCCR3B)
        TCCR3A = 0;
        TCCR3B = 0;
        TCNT3H = 0;
        TCNT3L = 0;

        //turn on CTC mode
        TCCR3B |= (1 << WGM32);

        //set prescaler to 64
        TCCR3B |= (1 << CS31)|(1 << CS30);

        uint32_t timerCount = COLUMN_SCAN_TIME/4;

        //1ms
        OCR3A = timerCount;

        //enable CTC interrupt
        TIMSK3 |= (1 << OCIE3A);
    #endif

}


//LEDs

void Board::setLEDstate(uint8_t ledNumber, uint8_t state)   {

    cli();
    ledState[ledNumber] = state;
    sei();

}

uint8_t Board::getLEDstate(uint8_t ledNumber)   {

    uint8_t returnValue;
    cli();
    returnValue = ledState[ledNumber];
    sei();
    return returnValue;

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

void Board::resetLEDtransitions()   {

    cli();
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    transitionCounter[i] = 0;
    sei();

}

void Board::setLEDTransitionSpeed(uint8_t steps) {

    cli();
    pwmSteps = steps;
    sei();

}

void Board::resetLEDblinkCounter()  {

    blinkTimerCounter = 0;

}


//analog

uint8_t Board::analogDataAvailable() {

    bool state;
    cli();
    state = _analogDataAvailable;
    sei();
    if (state) {

        cli();
        _analogDataAvailable = false;
        for (int i=0; i<ANALOG_BUFFER_SIZE; i++)
            analogBufferCopy[i] = analogBuffer[i];
        sei();
        startADCconversion();
        return ANALOG_BUFFER_SIZE-1;

    } return 0;

}

int16_t Board::getAnalogValue(uint8_t analogID) {

    return analogBufferCopy[analogID];

}

uint8_t Board::getAnalogID(uint8_t id)  {

    return id+(analogBufferCopy[ANALOG_BUFFER_SIZE-1]*8);

}


//encoders

encoderPosition Board::getEncoderState(uint8_t encoderNumber)  {

    encoderPosition returnValue;
    cli();
    returnValue = encoderMoving[encoderNumber];
    encoderMoving[encoderNumber] = encStopped;
    sei();
    return returnValue;

}


//buttons

uint8_t Board::buttonDataAvailable() {

    bool state;
    cli();
    state = _buttonDataAvailable;
    sei();
    if (state) {

        cli();
        _buttonDataAvailable = false;
        digitalBufferCopy = digitalBuffer;
        sei();
        return NUMBER_OF_BUTTON_ROWS;

    } return 0;

}

bool Board::getButtonState(uint8_t buttonIndex) {

    return !(((digitalBufferCopy >> 8) >> buttonIndex) & 0x01);

}

uint8_t Board::getButtonNumber(uint8_t buttonIndex) {

    return (digitalBufferCopy & 0xFF)+buttonIndex*NUMBER_OF_BUTTON_COLUMNS;

}


//timer-based functions

uint32_t Board::newMillis()    {

    uint32_t _rTime_mS;

    //save interrupt flag
    uint8_t interruptFlag = SREG;

    //disable interrupts
    cli();

    _rTime_mS = rTime_ms;

    //restore interruptFlag
    SREG = interruptFlag;

    return _rTime_mS;

}

void Board::newDelay(uint32_t delayTime)    {

    uint32_t _delayTime = newMillis() + delayTime;

    while (_delayTime > newMillis()) {}

}


Board boardObject;
