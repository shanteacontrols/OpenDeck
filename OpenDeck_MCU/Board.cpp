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

//variables used by interrupts
volatile int8_t             activeButtonColumn  = -1,
                            activeEncoderColumn = -1,
                            activeLEDColumn = -1;

volatile uint8_t            ledState[MAX_NUMBER_OF_LEDS],
                            activeMuxInput = 0,
                            tempEncoderState[NUMBER_OF_ENCODERS] = { 0 };

int16_t                     transitionCounter[MAX_NUMBER_OF_LEDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

volatile uint8_t            digitalBuffer;
volatile int16_t            analogBuffer[8] = { 0 };

volatile uint32_t           blinkTimerCounter = 0;

uint8_t                     blinkEnabled = false,
                            blinkState = true,
                            analogueEnabledArray[8] = { 0 };

uint16_t                    ledBlinkTime;

bool                        encoderDirection[MAX_NUMBER_OF_ENCODERS] = { 0 };
int32_t                     encoderPulses[MAX_NUMBER_OF_ENCODERS] = { 0 };
volatile encoderPosition    encoderMoving[MAX_NUMBER_OF_ENCODERS] = { encStopped };
static const int8_t         encoderLookUpTable[] = { 0, 1, -1, 2, -1, 0, -2, 1, 1, -2, 0, -1, 2, -1, 1, 0 };

volatile int8_t             activeMux = 0;
volatile uint32_t           rTime_ms = 0;

volatile int8_t             pwmSteps = eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_FADE_SPEED);
volatile bool analogDataAvailable = false;
static const uint8_t ledOnLookUpTable[] = { 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0 };
uint8_t lastRequestedColumn = -1;

void Board::resetLEDblinkCounter()  {

    blinkTimerCounter = 0;

}

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
        //PORTB &= 0x1F;
        //PORTD &= 0xFE;
        digitalWrite(4, LOW);
        digitalWrite(5, LOW);
        digitalWrite(A7, LOW);
        digitalWrite(A6, LOW);
    #elif defined BOARD_OPENDECK_1
        PORTB &= 0xF0;
    #endif

}

inline void ledRowOn(uint8_t rowNumber, uint8_t intensity)  {

    #ifdef BOARD_TANNIN
        switch (rowNumber)  {

            case 0:
            //PORTB |= 0b10000000;
            analogWrite(4, intensity);
            break;

            case 1:
            //PORTD |= 0b00000001;
            analogWrite(5, intensity);
            break;

            case 2:
            //PORTB |= 0b00100000;
            analogWrite(A7, intensity);
            break;

            case 3:
            //PORTB |= 0b01000000;
            analogWrite(A6, intensity);
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
            uint8_t ledStateSingle = ledOnLookUpTable[ledState[ledNumber]];
            if (ledStateSingle) ledRowOn(i, 255);

        }
    #elif defined BOARD_TANNIN
        //do fancy transitions here
        for (int i=0; i<NUMBER_OF_LED_ROWS; i++)  {

            uint8_t ledNumber = activeButtonColumn+i*NUMBER_OF_LED_COLUMNS;
            uint8_t ledStateSingle = ledOnLookUpTable[ledState[ledNumber]];

            if (!pwmSteps && ledStateSingle) ledRowOn(i, ledStateSingle);
            else if (pwmSteps) {

                if (ledStateSingle && (transitionCounter[ledNumber] == (NUMBER_OF_TRANSITIONS-1)))  {

                    ledRowOn(i, ledTransitionScale[NUMBER_OF_TRANSITIONS-1]);
                    continue;

                } else if (!ledStateSingle && !transitionCounter[ledNumber]) continue;

                if (transitionCounter[ledNumber]) ledRowOn(i, ledTransitionScale[transitionCounter[ledNumber]]);
                transitionCounter[ledNumber] += ledStateSingle ? pwmSteps : (pwmSteps*-1);
                if (transitionCounter[ledNumber] >= NUMBER_OF_TRANSITIONS) transitionCounter[ledNumber] = NUMBER_OF_TRANSITIONS-1;
                else if (transitionCounter[ledNumber] < 0) transitionCounter[ledNumber] = 0;

            }

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

inline void activateColumn()   {

    #ifdef BOARD_TANNIN
        PORTD &= 0b000011111;
        PORTD |= (activeButtonColumn << 5);
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

inline void activateEncoderColumn(uint8_t column)   {

    PORTC |= 0b11000000;

    switch (column) {

        case 0:
        PORTC &= 0b10111111;
        break;

        case 1:
        PORTC &= 0b01111111;
        break;

        default:
        break;

    }

}

inline void storeDigitalIn()  {

    uint8_t data = 0;

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

    _delay_ms(5);

    setNumberOfColumnPasses();

    #if defined (BOARD_TANNIN)
        setUpTimer4();
        activateEncoderColumn(activeEncoderColumn);
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

        //encoder pull-ups
        PORTB |= 0b00001111;
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
    getADCvalue();
    enableADCinterrupt();

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

    uint8_t rowPassTime = (COLUMN_SCAN_TIME/1000)*NUMBER_OF_BUTTON_COLUMNS;
    uint8_t mod = 0;

    if ((MIN_BUTTON_DEBOUNCE_TIME % rowPassTime) > 0)   mod = 1;

    numberOfColumnPasses = ((MIN_BUTTON_DEBOUNCE_TIME / rowPassTime) + mod);

}

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

uint8_t Board::getNumberOfColumnPasses()    {

    return numberOfColumnPasses;

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


//digital
uint8_t Board::getActiveColumn()    {

    return lastRequestedColumn;

}

uint8_t Board::getDigitalInData()   {

    uint8_t returnValue;
    cli();
    returnValue = digitalBuffer;
    sei();
    return returnValue;

}

//analog
void Board::setMux(uint8_t muxNumber)   {

    setMuxInternal(muxNumber);

}

void Board::setMuxInput(uint8_t muxInput)   {

    setMuxInputInteral(muxInput);

}

bool Board::analogInDataAvailable() {

    bool returnValue;
    cli();
    returnValue = analogDataAvailable;
    sei();
    return returnValue;

}

int16_t Board::getMuxInputValue(uint8_t analogID) {

    //setMuxInputInteral(analogID);
    //return getADCvalue();
    int16_t returnValue;
    cli();
    returnValue = analogBuffer[analogID];
    sei();
    return returnValue;

}

uint8_t Board::getAnalogID(uint8_t id)  {

    int8_t _activeMux;
    cli();
    _activeMux = activeMux;
    sei();

    if (!_activeMux) _activeMux = NUMBER_OF_MUX-1;
    else _activeMux--;

    return id+(_activeMux*8);

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

#if defined(TIMER2_COMPA_vect)
    ISR(TIMER2_COMPA_vect)  {

        //turn off all LED rows before switching to next column
        ledRowsOff();
        activeButtonColumn++;
        if (activeButtonColumn == NUMBER_OF_BUTTON_COLUMNS) activeButtonColumn = 0;
        activateColumn();
        storeDigitalIn();
        checkLEDs();

    }
#elif defined (TIMER3_COMPA_vect)
    ISR(TIMER3_COMPA_vect)  {

        //turn off all LED rows before switching to next column
        ledRowsOff();
        activeButtonColumn++;
        if (activeButtonColumn == NUMBER_OF_BUTTON_COLUMNS) activeButtonColumn = 0;
        activateColumn();
        storeDigitalIn();
        checkLEDs();

    }
#endif

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

#ifdef BOARD_TANNIN

    void Board::configurePWM()   {

        //increase default PWM frequency to 31kHz
        cli();
        TCCR1B = (TCCR1B & 0b11111000) | 0x01;
        TCCR1A = (TCCR1A & 0b11111000) | 0x01;
        TCCR0B = (TCCR0B & 0b11111000) | 0x01;
        TCCR0A = (TCCR0A & 0b11111000) | 0x01;
        sei();

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
#endif

ISR(ADC_vect)   {

    analogBuffer[activeMuxInput] = ADC;
    activeMuxInput++;

    bool startConversion = activeMuxInput != 8;

    if (!startConversion)    {

        activeMuxInput = 0;
        activeMux++;
        if (activeMux == NUMBER_OF_MUX) activeMux = 0;
        analogDataAvailable = true;
        setMuxInternal(activeMux);

    }

    //always set mux input
    setMuxInputInteral(activeMuxInput);
    if (startConversion) ADCSRA |= (1<<ADSC);

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

void Board::startAnalogConversion() {

    cli();
    analogDataAvailable = false;
    sei();

    ADCSRA |= (1<<ADSC);

}

bool Board::digitalInDataAvailable()    {

    int8_t column;

    cli();
    column = activeButtonColumn;
    sei();

    if (column != lastRequestedColumn)  {

        lastRequestedColumn = column;
        return true;

    } return false;

}

Board boardObject;
