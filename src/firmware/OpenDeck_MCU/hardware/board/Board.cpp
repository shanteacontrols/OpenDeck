#include "Board.h"
#include "../../sysex/SysEx.h"
#include <util/delay.h>
#include <avr/cpufunc.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include "../../eeprom/Configuration.h"
#include "../../BitManipulation.h"
#include "../../interface/settings/Settings.h"
#include "../../interface/encoders/Encoders.h"
#include "../../interface/leds/LEDs.h"
#include "../../interface/leds/LEDcolors.h"

#define DIGITAL_BUFFER_SIZE 4

//matrix columns
volatile uint8_t            activeButtonColumn  = 0;
volatile uint8_t            activeLEDcolumn = 0;

//buttons
uint64_t                    inputMatrixBufferCopy;

static volatile uint64_t    inputBuffer[DIGITAL_BUFFER_SIZE];
static volatile uint8_t     digital_buffer_head = 0;
static volatile uint8_t     digital_buffer_tail = 0;

//encoders
uint16_t                    encoderData[MAX_NUMBER_OF_ENCODERS];
static const int8_t         encoderLookUpTable[] = { 0, 1, -1, 2, -1, 0, -2, 1, 1, -2, 0, -1, 2, -1, 1, 0 };

bool                        buttonsProcessed,
                            encodersProcessed,
                            dmBufferCopied;

//LEDs
bool                        blinkEnabled = false,
                            blinkState = true;

volatile uint8_t            pwmSteps,
                            ledState[MAX_NUMBER_OF_LEDS];

uint16_t                    ledBlinkTime;

int8_t                      transitionCounter[MAX_NUMBER_OF_LEDS];
volatile uint32_t           blinkTimerCounter = 0;

//analog
volatile bool               _analogDataAvailable = false;
uint8_t                     activeMux = 0,
                            activeMuxInput = 0,
                            analogBufferCounter;
volatile int16_t            analogBuffer[ANALOG_BUFFER_SIZE];
int16_t                     analogBufferCopy[ANALOG_BUFFER_SIZE];
uint8_t                     adcDelayCounter;

//run time in milliseconds
volatile uint32_t           rTime_ms = 0;

void disableWatchDog()  {

    MCUSR &= ~(1 << WDRF);
    wdt_disable();

}

//timer-based functions

uint32_t rTimeMillis()    {

    uint32_t _rTime_mS;
    uint8_t interruptFlag = SREG;

    cli();
    _rTime_mS = rTime_ms;
    SREG = interruptFlag;

    return _rTime_mS;

}

void wait(uint32_t time)    {

    uint32_t _delayTime = rTimeMillis() + time;
    while (_delayTime > rTimeMillis()) {}

}


//inline functions

inline uint8_t getEncoderPairFromButtonIndex(uint8_t buttonIndex)   {

    uint8_t row = buttonIndex/NUMBER_OF_BUTTON_COLUMNS;
    if (row%2) row -= 1;   //uneven row, get info from previous (even) row
    uint8_t column = buttonIndex % NUMBER_OF_BUTTON_COLUMNS;
    return (row*NUMBER_OF_BUTTON_COLUMNS)/2 + column;

}

inline encoderPosition_t readEncoder(uint8_t encoderID, uint8_t pairState)  {

    //add new data
    uint8_t newPairData = 0;
    newPairData |= (((encoderData[encoderID] << 2) & 0x000F) | (uint16_t)pairState);

    //remove old data
    encoderData[encoderID] &= ENCODER_CLEAR_TEMP_STATE_MASK;

    //shift in new data
    encoderData[encoderID] |= (uint16_t)newPairData;

    int8_t encRead = encoderLookUpTable[newPairData];

    if (!encRead) return encStopped;

    bool newEncoderDirection = encRead > 0;
    //get current number of pulses from encoderData
    int8_t currentPulses = (encoderData[encoderID] >> 4) & 0x000F;
    currentPulses += encRead;
    //clear current pulses
    encoderData[encoderID] &= ENCODER_CLEAR_PULSES_MASK;
    //shift in new pulse count
    encoderData[encoderID] |= (uint16_t)(currentPulses << 4);
    //get last encoder direction
    bool lastEncoderDirection = bitRead(encoderData[encoderID], ENCODER_DIRECTION_BIT);
    //write new encoder direction
    bitWrite(encoderData[encoderID], ENCODER_DIRECTION_BIT, newEncoderDirection);

    if (lastEncoderDirection != newEncoderDirection) return encStopped;
    if (currentPulses % PULSES_PER_STEP) return encStopped;

    //clear current pulses
    encoderData[encoderID] &= ENCODER_CLEAR_PULSES_MASK;

    //set default pulse count
    encoderData[encoderID] |= ((uint16_t)ENCODER_DEFAULT_PULSE_COUNT_STATE << 4);

    //clear current moving status
    //encoderData[encoderID] &= ENCODER_CLEAR_MOVING_STATUS_MASK;
    if (newEncoderDirection) return encMoveRight;
    else return encMoveLeft;

}

inline void setAnalogPin(uint8_t muxNumber)   {

    uint8_t analogPin;

    switch(muxNumber) {

        case 0:
        analogPin = MUX_1_IN_PIN;
        break;

        case 1:
        analogPin = MUX_2_IN_PIN;
        break;

        default:
        return;

    }

    ADMUX = (ADMUX & 0xF0) | (analogPin & 0x0F);

}

inline void setMuxInput(uint8_t muxInput)    {

    //according to datasheet, propagation delay between setting Sn pins
    //and output appearing on Yn is around 150ns
    //add three NOPs to compensate

    bitRead(muxPinOrderArray[muxInput], 0) ? setHighMacro(MUX_S0_PORT, MUX_S0_PIN) : setLowMacro(MUX_S0_PORT, MUX_S0_PIN);
    bitRead(muxPinOrderArray[muxInput], 1) ? setHighMacro(MUX_S1_PORT, MUX_S1_PIN) : setLowMacro(MUX_S1_PORT, MUX_S1_PIN);
    bitRead(muxPinOrderArray[muxInput], 2) ? setHighMacro(MUX_S2_PORT, MUX_S2_PIN) : setLowMacro(MUX_S2_PORT, MUX_S2_PIN);
    bitRead(muxPinOrderArray[muxInput], 3) ? setHighMacro(MUX_S3_PORT, MUX_S3_PIN) : setLowMacro(MUX_S3_PORT, MUX_S3_PIN);

    _NOP(); _NOP(); _NOP();

}

inline void ledRowsOff()   {

    //turn off PWM
    TCCR1A &= ~(1<<COM1C1);
    TCCR4C &= ~(1<<COM4D1);
    TCCR1A &= ~(1<<COM1A1);
    TCCR4A &= ~(1<<COM4A1);
    TCCR3A &= ~(1<<COM3A1);
    TCCR1A &= ~(1<<COM1B1);

    setHighMacro(LED_ROW_1_PORT, LED_ROW_1_PIN);
    setHighMacro(LED_ROW_2_PORT, LED_ROW_2_PIN);
    setHighMacro(LED_ROW_3_PORT, LED_ROW_3_PIN);
    setHighMacro(LED_ROW_4_PORT, LED_ROW_4_PIN);
    setHighMacro(LED_ROW_5_PORT, LED_ROW_5_PIN);
    setHighMacro(LED_ROW_6_PORT, LED_ROW_6_PIN);


}

inline void ledRowOn(uint8_t rowNumber, uint8_t intensity)  {

    switch (rowNumber)  {

        case 0:
        OCR1C = intensity;
        TCCR1A |= (1<<COM1C1);
        break;

        case 1:
        OCR4D = intensity;
        TCCR4C |= (1<<COM4D1);
        break;

        case 2:
        OCR1A = intensity;
        TCCR1A |= (1<<COM1A1);
        break;

        case 3:
        OCR4A = intensity;
        TCCR4A |= (1<<COM4A1);
        break;

        case 4:
        OCR3A = intensity;
        TCCR3A |= (1<<COM3A1);
        break;

        case 5:
        OCR1B = intensity;
        TCCR1A |= (1<<COM1B1);
        break;

        default:
        break;

    }

}

inline void checkLEDs()  {

    if (blinkEnabled)   {

        if (!blinkTimerCounter)  {

            //change blinkBit state and write it into ledState variable if LED is in blink state
            for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)  {

                if (bitRead(ledState[i], LED_BLINK_ON_BIT))    {

                    if (blinkState) bitWrite(ledState[i], LED_BLINK_STATE_BIT, 1);
                    else bitWrite(ledState[i], LED_BLINK_STATE_BIT, 0);

                }

            }

            blinkState = !blinkState;

        }

    }

    //if there is an active LED in current column, turn on LED row
    //do fancy transitions here
        for (int i=0; i<NUMBER_OF_LED_ROWS; i++)  {

            uint8_t ledNumber = activeLEDcolumn+i*NUMBER_OF_LED_COLUMNS;
            uint8_t ledStateSingle = ledOnLookUpTable[ledState[ledNumber]];

            //don't bother with pwm if it's disabled
            if (!pwmSteps && ledStateSingle)    {

                #ifdef LED_INVERT
                    ledRowOn(i, 0);
                #else
                    ledRowOn(i, 255);
                #endif

            } else {

                if (
                (ledStateSingle && (transitionCounter[ledNumber] != (NUMBER_OF_TRANSITIONS-1))) ||
                (!ledStateSingle && transitionCounter[ledNumber])
                )  {

                    if (ledStateSingle) transitionCounter[ledNumber] += pwmSteps;
                    else transitionCounter[ledNumber] -= pwmSteps;

                    if (transitionCounter[ledNumber] >= NUMBER_OF_TRANSITIONS) transitionCounter[ledNumber] = NUMBER_OF_TRANSITIONS-1;
                    if (transitionCounter[ledNumber] < 0) transitionCounter[ledNumber] = 0;

                }

                if (transitionCounter[ledNumber]) {

                    #ifdef LED_INVERT
                        ledRowOn(i, 255-ledTransitionScale[transitionCounter[ledNumber]]);
                    #else
                        ledRowOn(i, ledTransitionScale[transitionCounter[ledNumber]]);
                    #endif

                }

            }

        }

}

inline void activateInputColumn(uint8_t column)   {

    bitRead(dmColumnArray[column], 0) ? setHighMacro(DEC_DM_A0_PORT, DEC_DM_A0_PIN) : setLowMacro(DEC_DM_A0_PORT, DEC_DM_A0_PIN);
    bitRead(dmColumnArray[column], 1) ? setHighMacro(DEC_DM_A1_PORT, DEC_DM_A1_PIN) : setLowMacro(DEC_DM_A1_PORT, DEC_DM_A1_PIN);
    bitRead(dmColumnArray[column], 2) ? setHighMacro(DEC_DM_A2_PORT, DEC_DM_A2_PIN) : setLowMacro(DEC_DM_A2_PORT, DEC_DM_A2_PIN);

    _NOP();

}

inline void activateOutputColumn(uint8_t column)    {

    bitRead(column, 0) ? setHighMacro(DEC_LM_A0_PORT, DEC_LM_A0_PIN) : setLowMacro(DEC_LM_A0_PORT, DEC_LM_A0_PIN);
    bitRead(column, 1) ? setHighMacro(DEC_LM_A1_PORT, DEC_LM_A1_PIN) : setLowMacro(DEC_LM_A1_PORT, DEC_LM_A1_PIN);
    bitRead(column, 2) ? setHighMacro(DEC_LM_A2_PORT, DEC_LM_A2_PIN) : setLowMacro(DEC_LM_A2_PORT, DEC_LM_A2_PIN);

    _NOP();

}

inline void storeDigitalIn(uint8_t column, uint8_t bufferIndex)  {

    uint8_t data = 0;
    uint8_t dataReorder = 0;

    //make room for new data
    inputBuffer[bufferIndex] <<= 8;

    //pulse latch pin
    pulseLowToHigh(SR_LATCH_PORT, SR_LATCH_PIN);

    for (int i=0; i<8; i++) {

        data <<= 1;
        data |= ((SR_DIN_PIN_REGISTER >> SR_DIN_PIN) & 0x01);
        //pulse clock pin
        pulseHightToLow(SR_CLK_PORT, SR_CLK_PIN);

    }

    //reorder data to match rows on PCB layout
    for (int i=0; i<8; i++)
        bitWrite(dataReorder, i, bitRead(data, dmRowBitArray[i]));

    inputBuffer[bufferIndex] |= (uint64_t)dataReorder;

}

int8_t getInputMatrixBufferSize() {

    uint8_t head, tail;

    head = digital_buffer_head;
    tail = digital_buffer_tail;
    if (head >= tail) return head - tail;
    return DIGITAL_BUFFER_SIZE + head - tail;

}

bool copyInputMatrixBuffer()    {

    int8_t bufferSize = getInputMatrixBufferSize();

    if (bufferSize <= 0) return false;

    //some data in buffer
    //copy oldest member of buffer to inputMatrixBufferCopy
    if (digital_buffer_head == digital_buffer_tail) return false;
    uint8_t index = digital_buffer_tail + 1;
    if (index >= DIGITAL_BUFFER_SIZE) index = 0;
    cli();
    inputMatrixBufferCopy = inputBuffer[index];
    sei();
    dmBufferCopied = true;
    buttonsProcessed = false;
    encodersProcessed = false;
    digital_buffer_tail = index;
    return true;

}


//ISR functions
ISR(TIMER0_COMPA_vect) {

    //run millis and blink update every 1ms
    //update led matrix every 1.5us
    //update button matrix each time

    static bool updateMillisAndBlink = false;
    static uint8_t matrixSwitchCounter = 0;
    uint32_t ms;

    if (matrixSwitchCounter == 1)   {

        ledRowsOff();
        if (activeLEDcolumn == NUMBER_OF_LED_COLUMNS) activeLEDcolumn = 0;
        activateOutputColumn(activeLEDcolumn);
        checkLEDs();
        activeLEDcolumn++;
        matrixSwitchCounter = 0;

    }

    if (updateMillisAndBlink)   {

        ms = rTime_ms;
        ms++;
        //update run time
        rTime_ms = ms;

        matrixSwitchCounter++;

    }   else {

        if (blinkEnabled) {

            blinkTimerCounter++;
            if (blinkTimerCounter >= ledBlinkTime) blinkTimerCounter = 0;

        }

    }

    updateMillisAndBlink = !updateMillisAndBlink;

    //read input matrix
    uint8_t bufferIndex = digital_buffer_head + 1;
    if (bufferIndex >= DIGITAL_BUFFER_SIZE) bufferIndex = 0;
    if (digital_buffer_tail == bufferIndex) return; //buffer full, exit
    inputBuffer[bufferIndex] = 0;
    digital_buffer_head = bufferIndex;

    for (int i=0; i<NUMBER_OF_BUTTON_COLUMNS; i++)  {

        activateInputColumn(i);
        storeDigitalIn(i, bufferIndex);

    }

    if (!_analogDataAvailable && !bitRead(ADCSRA, ADSC))   {

        adcDelayCounter++;
        if (adcDelayCounter == 2)   {

            adcDelayCounter = 0;
            startADCconversion();

        }

    }

}

ISR(ADC_vect)   {

    analogBuffer[analogBufferCounter] = ADC;
    analogBufferCounter++;

    activeMuxInput++;

    bool switchMux = (activeMuxInput == NUMBER_OF_MUX_INPUTS);
    bool bufferFull = (analogBufferCounter == MAX_NUMBER_OF_ANALOG);

    if (switchMux)  {

        activeMuxInput = 0;
        activeMux++;
        if (activeMux == NUMBER_OF_MUX) activeMux = 0;
        setAnalogPin(activeMux);

    }

    if (bufferFull)   {

        analogBufferCounter = 0;
         _analogDataAvailable = true;

    }

    //always set mux input
    setMuxInput(activeMuxInput);
    if (!bufferFull) startADCconversion();

}


//init

Board::Board()  {

    //default constructor

}

void Board::init()  {

    cli();
    disableWatchDog();
    initPins();
    initAnalog();

    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)    {

        encoderData[i] |= ((uint16_t)encStopped << 8);
        encoderData[i] |= ((uint16_t)ENCODER_DEFAULT_PULSE_COUNT_STATE << 4);   //set number of pulses to 8

    }

    _delay_ms(5);

    configureTimers();

    //enable global interrupts
    sei();

}

void Board::initPins() {

    //configure input matrix
    //shift register
    setInputMacro(SR_DIN_DDR, SR_DIN_PIN);
    setOutputMacro(SR_CLK_DDR, SR_CLK_PIN);
    setOutputMacro(SR_LATCH_DDR, SR_LATCH_PIN);

    //decoder
    setOutputMacro(DEC_DM_A0_DDR, DEC_DM_A0_PIN);
    setOutputMacro(DEC_DM_A1_DDR, DEC_DM_A1_PIN);
    setOutputMacro(DEC_DM_A1_DDR, DEC_DM_A2_PIN);

    //configure led matrix
    //rows

    setHighMacro(LED_ROW_1_PORT, LED_ROW_1_PIN);
    setHighMacro(LED_ROW_2_PORT, LED_ROW_2_PIN);
    setHighMacro(LED_ROW_3_PORT, LED_ROW_3_PIN);
    setHighMacro(LED_ROW_4_PORT, LED_ROW_4_PIN);
    setHighMacro(LED_ROW_5_PORT, LED_ROW_5_PIN);
    setHighMacro(LED_ROW_6_PORT, LED_ROW_6_PIN);

    setOutputMacro(LED_ROW_1_DDR, LED_ROW_1_PIN);
    setOutputMacro(LED_ROW_2_DDR, LED_ROW_2_PIN);
    setOutputMacro(LED_ROW_3_DDR, LED_ROW_3_PIN);
    setOutputMacro(LED_ROW_4_DDR, LED_ROW_4_PIN);
    setOutputMacro(LED_ROW_5_DDR, LED_ROW_5_PIN);
    setOutputMacro(LED_ROW_6_DDR, LED_ROW_6_PIN);

    //decoder
    setOutputMacro(DEC_LM_A0_DDR, DEC_LM_A0_PIN);
    setOutputMacro(DEC_LM_A1_DDR, DEC_LM_A1_PIN);
    setOutputMacro(DEC_LM_A2_DDR, DEC_LM_A2_PIN);

    //configure analog
    //select pins
    setOutputMacro(MUX_S0_DDR, MUX_S0_PIN);
    setOutputMacro(MUX_S1_DDR, MUX_S1_PIN);
    setOutputMacro(MUX_S2_DDR, MUX_S2_PIN);
    setOutputMacro(MUX_S3_DDR, MUX_S3_PIN);

    //mux inputs
    setInputMacro(MUX_1_IN_DDR, MUX_1_IN_PIN);
    setInputMacro(MUX_2_IN_DDR, MUX_2_IN_PIN);

    //bootloader led
    setOutputMacro(BTLDR_LED_DDR, BTLDR_LED_PIN);

}

void Board::initAnalog()    {

    setUpADC();
    setADCprescaler(128);

    setMuxInput(activeMuxInput);
    setADCchannel(MUX_1_IN_PIN);

    disconnectDigitalInADC(MUX_1_IN_PIN);
    disconnectDigitalInADC(MUX_2_IN_PIN);

    _delay_ms(2);
    for (int i=0; i<5; i++)
        getADCvalue();  //few dummy reads to init ADC
    adcInterruptEnable();
    startADCconversion();

}

void Board::configureTimers()   {

    //clear timer0 conf
    TCCR0A = 0;
    TCCR0B = 0;
    TIMSK0 = 0;

    //clear timer1 conf
    TCCR1A = 0;
    TCCR1B = 0;

    //clear timer3 conf
    TCCR3A = 0;
    TCCR3B = 0;

    //clear timer4 conf
    TCCR4A = 0;
    TCCR4B = 0;
    TCCR4C = 0;
    TCCR4D = 0;
    TCCR4E = 0;

    //set timer1, timer3 and timer4 to phase correct pwm mode
    //timer 1
    TCCR1A |= (1<<WGM10);           //phase correct PWM
    TCCR1B |= (1<<CS10);            //prescaler 1
    //timer 3
    TCCR3A |= (1<<WGM30);           //phase correct PWM
    TCCR3B |= (1<<CS30);            //prescaler 1
    //OCR3A = 10000;
    //TIMSK3 |= (1<<OCIE3A);
    //timer 4
    TCCR4A |= (1<<PWM4A);           //Pulse Width Modulator A Enable
    TCCR4B |= (1<<CS40);            //prescaler 1
    TCCR4C |= (1<<PWM4D);           //Pulse Width Modulator D Enable
    TCCR4D |= (1<<WGM40);           //phase correct PWM

    //set timer0 to ctc, used for millis/led matrix
    TCCR0A |= (1<<WGM01);           //CTC mode
    TCCR0B |= (1<<CS01)|(1<<CS00);  //prescaler 64
    OCR0A = 124;                    //500uS
    TIMSK0 |= (1<<OCIE0A);          //compare match interrupt

}

//LEDs

inline uint8_t getRGBIDFromLEDID(uint8_t ledID) {

    uint8_t row = ledID/NUMBER_OF_LED_COLUMNS;

    uint8_t mod = row%3;    //RGB LED = 3 normal LEDs
    row -= mod;

    uint8_t column = ledID % NUMBER_OF_BUTTON_COLUMNS;

    return (row*NUMBER_OF_LED_COLUMNS)/3 + column;

}

void Board::setLEDstate(uint8_t ledNumber, ledColor_t color, bool blinkMode)   {

    uint8_t rgbID = getRGBIDFromLEDID(ledNumber);
    bool rgbEnabled = configuration.readParameter(CONF_LED_BLOCK, ledRGBenabledSection, rgbID);

    if (!rgbEnabled)    {

        if (color != colorOff)
            color = colorOnDefault;
        handleLED(ledNumber, color, blinkMode, singleLED);

    }   else handleLED(rgbID, color, blinkMode, rgbLED);

    if (blinkMode && (color != colorOff)) ledBlinkingStart();
    else    checkBlinkLEDs();

}

uint8_t Board::getLEDstate(uint8_t ledNumber)   {

    uint8_t returnValue;
    returnValue = ledState[ledNumber];
    return returnValue;

}

void Board::setLEDblinkTime(uint16_t blinkTime)  {

    cli();
    ledBlinkTime = blinkTime*100;
    blinkTimerCounter = 0;
    sei();

}

void Board::setLEDTransitionSpeed(uint8_t transitionSteps) {

    //reset transition counter
    cli();
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        transitionCounter[i] = 0;
    sei();

    pwmSteps = transitionSteps;

}

void Board::ledBlinkingStart() {

    if (!blinkEnabled)  {

        blinkEnabled = true;
        blinkState = true;
        blinkTimerCounter = 0;

    }

}

void Board::ledBlinkingStop()   {

    blinkState = true;
    cli();
    blinkTimerCounter = 0;
    sei();
    blinkEnabled = false;

}

bool Board::ledBlinkingActive() {

    bool state;
    state = blinkEnabled;
    return state;

}

void Board::checkBlinkLEDs() {

    //this function will disable blinking
    //if none of the LEDs is in blinking state

    //else it will enable it

    bool _blinkEnabled = false;
    uint8_t ledState;

    //if any LED is blinking, set timerState to true and exit the loop
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    {

        ledState = getLEDstate(i);

        if (bitRead(ledState, LED_BLINK_ON_BIT)) {

            _blinkEnabled = true;
            break;

        }

    }

    if (_blinkEnabled)  ledBlinkingStart();

    //don't bother reseting variables if blinking is already disabled
    else    if (!_blinkEnabled && ledBlinkingActive()) {

        //reset blinkState to default value
        ledBlinkingStop();

    }

}

inline uint8_t getRGBfirstID(uint8_t rgbID)    {

    //get first RGB LED address (out of three)
    uint8_t column = rgbID % NUMBER_OF_LED_COLUMNS;
    uint8_t row  = (rgbID/NUMBER_OF_BUTTON_COLUMNS)*3;

    return column + NUMBER_OF_LED_COLUMNS*row;

}


void Board::handleLED(uint8_t ledNumber, ledColor_t color, bool blinkMode, ledType_t type) {

    /*

    LED state is stored into one byte (ledState). The bits have following meaning (7 being the MSB bit):

    7: x
    6: x
    5: x
    4: x
    3: Blink bit (timer changes this bit)
    2: LED is active (either it blinks or it's constantly on), this bit is OR function between bit 0 and 1
    1: LED blinks
    0: LED is constantly turned on

    */

    uint8_t currentState[3];
    bool newLEDstate[3];
    uint8_t loops = 1;

    if ((color == colorOnDefault) && (type == rgbLED))
        type = singleLED; //this is a mistake, handle led in single mode instead

    switch(type)    {

        case singleLED:
        loops = 1;
        currentState[0] = getLEDstate(ledNumber);
        newLEDstate[0] = (color != colorOff);
        break;

        case rgbLED:
        loops = 3;
        ledNumber = getRGBfirstID(ledNumber);
        currentState[0] = getLEDstate(ledNumber);
        currentState[1] = getLEDstate(ledNumber+NUMBER_OF_LED_COLUMNS*1);
        currentState[2] = getLEDstate(ledNumber+NUMBER_OF_LED_COLUMNS*2);

        newLEDstate[0] = rgbColors[color][0];
        newLEDstate[1] = rgbColors[color][1];
        newLEDstate[2] = rgbColors[color][2];
        break;

    }

    for (int i=0; i<loops; i++) {

        ledNumber += 8*(bool)i;

        switch (newLEDstate[i]) {

            case false:
            //turn off the led
            currentState[i] = 0;
            break;

            case true:
            //turn on the led
            //if led was already active, clear the on bits before setting new state
            if (bitRead(currentState[i], LED_ACTIVE_BIT))
                currentState[i] = 0;

            bitWrite(currentState[i], LED_ACTIVE_BIT, 1);
            if (blinkMode)  {

                bitWrite(currentState[i], LED_BLINK_ON_BIT, 1);
                //this will turn the led immediately no matter how little time it's
                //going to blink first time
                bitWrite(currentState[i], LED_BLINK_STATE_BIT, 1);

            }   else bitWrite(currentState[i], LED_CONSTANT_ON_BIT, 1);
            break;

        }   ledState[ledNumber] = currentState[i];

    }

}


//analog

bool Board::analogDataAvailable() {

    bool state;
    state = _analogDataAvailable;
    if (state) {

        //no cli/sei needed since adc conversion is stopped at the moment
        for (int i=0; i<ANALOG_BUFFER_SIZE; i++)
            analogBufferCopy[i] = analogBuffer[i];

        _analogDataAvailable = false;
        adcDelayCounter = 0;
        return true;

    } return false;

}

int16_t Board::getAnalogValue(uint8_t analogID) {

    return analogBufferCopy[analogID];

}

inline void checkInputMatrixBufferCopy()    {

    if ((buttonsProcessed == true) && (encodersProcessed == true) && (dmBufferCopied == true))  {

        dmBufferCopied = false;
        buttonsProcessed = false;
        encodersProcessed = false;

    }

}


//encoders

encoderPosition_t Board::getEncoderState(uint8_t encoderNumber)  {

    uint8_t column = encoderNumber % NUMBER_OF_BUTTON_COLUMNS;
    uint8_t row  = (encoderNumber/NUMBER_OF_BUTTON_COLUMNS)*2;
    uint8_t shiftAmount = ((NUMBER_OF_BUTTON_COLUMNS-1)*8) - column*8;
    uint8_t pairState = inputMatrixBufferCopy >> shiftAmount;
    pairState = ((pairState >> row) & 0x03);

    return readEncoder(encoderNumber, pairState);

}

bool Board::encoderDataAvailable()  {

    checkInputMatrixBufferCopy();

    bool returnValue = true;
    bool _dmBufferCopied;
    _dmBufferCopied = dmBufferCopied;

    if (!_dmBufferCopied)    {   //buffer isn't copied

        returnValue = copyInputMatrixBuffer();

    }

    encodersProcessed = true;
    return returnValue;

}


//buttons

bool Board::buttonDataAvailable()   {

    checkInputMatrixBufferCopy();

    bool returnValue = true;
    bool _dmBufferCopied;
    _dmBufferCopied = dmBufferCopied;

    if (!_dmBufferCopied)    {   //buffer isn't copied

        returnValue = copyInputMatrixBuffer();

    }

    buttonsProcessed = true;
    return returnValue;

}

bool Board::getButtonState(uint8_t buttonIndex) {

    uint8_t encoderPairIndex = getEncoderPairFromButtonIndex(buttonIndex);
    if (encoders.getEncoderEnabled(encoderPairIndex))
        return false;   //button is member of encoder pair, return "not pressed" state

    uint8_t row = buttonIndex/NUMBER_OF_BUTTON_COLUMNS;
    uint8_t column = (NUMBER_OF_BUTTON_COLUMNS-1) - buttonIndex % NUMBER_OF_BUTTON_COLUMNS; //invert column order
    buttonIndex = column*8 + row;

    return !((inputMatrixBufferCopy >> buttonIndex) & 0x01);

}

Board board;
