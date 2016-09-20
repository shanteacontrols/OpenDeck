/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015, 2016 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Board.h"
#include <util/delay.h>
#include <avr/cpufunc.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#define DIGITAL_BUFFER_SIZE                 2
#define ANALOG_BUFFER_SIZE                  MAX_NUMBER_OF_ANALOG

//matrix columns
volatile uint8_t                            activeButtonColumn  = 0;
volatile uint8_t                            activeLEDcolumn = 0;

//buttons
uint64_t                                    inputMatrixBufferCopy;

static volatile uint64_t                    inputBuffer[DIGITAL_BUFFER_SIZE];
static volatile uint8_t                     digital_buffer_head = 0;
static volatile uint8_t                     digital_buffer_tail = 0;

//encoders
uint16_t                                    encoderData[MAX_NUMBER_OF_ENCODERS];
static const int8_t                         encoderLookUpTable[] = { 0, 1, -1, 2, -1, 0, -2, 1, 1, -2, 0, -1, 2, -1, 1, 0 };

bool                                        buttonsProcessed,
                                            encodersProcessed,
                                            dmBufferCopied;

#define PULSES_PER_STEP                     4

/*

    Encoder data formatting, uint16_t variable type
    0      1      2      3
    0000 | 0000 | 0000 | 0000

    0 - encoder direction (0/1 - left/right)
    1 - encoderMoving (0/1/2 - stopped/left/right)
    2 - counted pulses (default value is 8 to avoid issues with negative values)
    3 - temp encoder state (2 readings of 2 encoder pairs)

*/

#define ENCODER_CLEAR_TEMP_STATE_MASK       0xFFF0
#define ENCODER_CLEAR_PULSES_MASK           0xFF0F
#define ENCODER_CLEAR_MOVING_STATUS_MASK    0xF0FF

#define ENCODER_DIRECTION_BIT               15

#define ENCODER_DEFAULT_PULSE_COUNT_STATE   8

//LEDs
bool                                        blinkEnabled = false,
                                            blinkState = true;

volatile uint8_t                            pwmSteps,
                                            ledState[MAX_NUMBER_OF_LEDS];

uint16_t                                    ledBlinkTime;

int8_t                                      transitionCounter[MAX_NUMBER_OF_LEDS];
volatile uint32_t                           blinkTimerCounter = 0;

#define NUMBER_OF_TRANSITIONS               64

#define LED_CONSTANT_ON_BIT                 0x00
#define LED_BLINK_ON_BIT                    0x01
#define LED_ACTIVE_BIT                      0x02
#define LED_BLINK_STATE_BIT                 0x03

const uint8_t ledOnLookUpTable[]            = { 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0 };

const uint8_t ledTransitionScale[] = {

    0,
    2,
    4,
    6,
    8,
    10,
    12,
    14,
    16,
    18,
    20,
    22,
    24,
    26,
    28,
    30,
    32,
    34,
    36,
    38,
    40,
    42,
    44,
    46,
    48,
    50,
    52,
    54,
    56,
    58,
    60,
    62,
    64,
    68,
    70,
    75,
    80,
    85,
    90,
    95,
    100,
    105,
    110,
    115,
    120,
    125,
    130,
    135,
    140,
    145,
    150,
    155,
    160,
    165,
    170,
    180,
    190,
    200,
    210,
    220,
    230,
    240,
    250,
    255

};

//analog
volatile bool                               _analogDataAvailable = false;
uint8_t                                     activeMux = 0,
                                            activeMuxInput = 0,
                                            analogBufferCounter;
volatile int16_t                            analogBuffer[ANALOG_BUFFER_SIZE];
int16_t                                     analogBufferCopy[ANALOG_BUFFER_SIZE];
uint8_t                                     adcDelayCounter;

//run time in milliseconds
volatile uint32_t           rTime_ms = 0;

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

    while(time--)   {

        _delay_ms(1);

    }

}


//inline functions

uint8_t Board::getEncoderPairFromButtonIndex(uint8_t buttonIndex)   {

    uint8_t row = buttonIndex/NUMBER_OF_BUTTON_COLUMNS;
    if (row%2) row -= 1;   //uneven row, get info from previous (even) row
    uint8_t column = buttonIndex % NUMBER_OF_BUTTON_COLUMNS;
    return (row*NUMBER_OF_BUTTON_COLUMNS)/2 + column;

}

inline int8_t readEncoder(uint8_t encoderID, uint8_t pairState)  {

    //add new data
    uint8_t newPairData = 0;
    newPairData |= (((encoderData[encoderID] << 2) & 0x000F) | (uint16_t)pairState);

    //remove old data
    encoderData[encoderID] &= ENCODER_CLEAR_TEMP_STATE_MASK;

    //shift in new data
    encoderData[encoderID] |= (uint16_t)newPairData;

    int8_t encRead = encoderLookUpTable[newPairData];

    if (!encRead) return 0;

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

    if (lastEncoderDirection != newEncoderDirection) return 0;
    if (currentPulses % PULSES_PER_STEP) return 0;

    //clear current pulses
    encoderData[encoderID] &= ENCODER_CLEAR_PULSES_MASK;

    //set default pulse count
    encoderData[encoderID] |= ((uint16_t)ENCODER_DEFAULT_PULSE_COUNT_STATE << 4);

    //clear current moving status
    //encoderData[encoderID] &= ENCODER_CLEAR_MOVING_STATUS_MASK;
    if (newEncoderDirection) return 1;
    else return -1;

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

    bitRead(muxPinOrderArray[muxInput], 0) ? setHigh(MUX_S0_PORT, MUX_S0_PIN) : setLow(MUX_S0_PORT, MUX_S0_PIN);
    bitRead(muxPinOrderArray[muxInput], 1) ? setHigh(MUX_S1_PORT, MUX_S1_PIN) : setLow(MUX_S1_PORT, MUX_S1_PIN);
    bitRead(muxPinOrderArray[muxInput], 2) ? setHigh(MUX_S2_PORT, MUX_S2_PIN) : setLow(MUX_S2_PORT, MUX_S2_PIN);
    bitRead(muxPinOrderArray[muxInput], 3) ? setHigh(MUX_S3_PORT, MUX_S3_PIN) : setLow(MUX_S3_PORT, MUX_S3_PIN);

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

    setHigh(LED_ROW_1_PORT, LED_ROW_1_PIN);
    setHigh(LED_ROW_2_PORT, LED_ROW_2_PIN);
    setHigh(LED_ROW_3_PORT, LED_ROW_3_PIN);
    setHigh(LED_ROW_4_PORT, LED_ROW_4_PIN);
    setHigh(LED_ROW_5_PORT, LED_ROW_5_PIN);
    setHigh(LED_ROW_6_PORT, LED_ROW_6_PIN);

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

void checkLEDs()  {

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

    bitRead(dmColumnArray[column], 0) ? setHigh(DEC_DM_A0_PORT, DEC_DM_A0_PIN) : setLow(DEC_DM_A0_PORT, DEC_DM_A0_PIN);
    bitRead(dmColumnArray[column], 1) ? setHigh(DEC_DM_A1_PORT, DEC_DM_A1_PIN) : setLow(DEC_DM_A1_PORT, DEC_DM_A1_PIN);
    bitRead(dmColumnArray[column], 2) ? setHigh(DEC_DM_A2_PORT, DEC_DM_A2_PIN) : setLow(DEC_DM_A2_PORT, DEC_DM_A2_PIN);

    _NOP();

}

inline void activateOutputColumn(uint8_t column)    {

    bitRead(column, 0) ? setHigh(DEC_LM_A0_PORT, DEC_LM_A0_PIN) : setLow(DEC_LM_A0_PORT, DEC_LM_A0_PIN);
    bitRead(column, 1) ? setHigh(DEC_LM_A1_PORT, DEC_LM_A1_PIN) : setLow(DEC_LM_A1_PORT, DEC_LM_A1_PIN);
    bitRead(column, 2) ? setHigh(DEC_LM_A2_PORT, DEC_LM_A2_PIN) : setLow(DEC_LM_A2_PORT, DEC_LM_A2_PIN);

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
        data |= readPin(SR_DIN_PORT, SR_DIN_PIN);
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
    //disable watchdog
    MCUSR &= ~(1 << WDRF);
    wdt_disable();
    initPins();
    initAnalog();
    initEncoders();

    _delay_ms(5);

    configureTimers();

    //enable global interrupts
    sei();

}

void Board::initPins() {

    //configure input matrix
    //shift register
    setInput(SR_DIN_PORT, SR_DIN_PIN);
    setOutput(SR_CLK_PORT, SR_CLK_PIN);
    setOutput(SR_LATCH_PORT, SR_LATCH_PIN);

    //decoder
    setOutput(DEC_DM_A0_PORT, DEC_DM_A0_PIN);
    setOutput(DEC_DM_A1_PORT, DEC_DM_A1_PIN);
    setOutput(DEC_DM_A1_PORT, DEC_DM_A2_PIN);

    //configure led matrix
    //rows

    setHigh(LED_ROW_1_PORT, LED_ROW_1_PIN);
    setHigh(LED_ROW_2_PORT, LED_ROW_2_PIN);
    setHigh(LED_ROW_3_PORT, LED_ROW_3_PIN);
    setHigh(LED_ROW_4_PORT, LED_ROW_4_PIN);
    setHigh(LED_ROW_5_PORT, LED_ROW_5_PIN);
    setHigh(LED_ROW_6_PORT, LED_ROW_6_PIN);

    setOutput(LED_ROW_1_PORT, LED_ROW_1_PIN);
    setOutput(LED_ROW_2_PORT, LED_ROW_2_PIN);
    setOutput(LED_ROW_3_PORT, LED_ROW_3_PIN);
    setOutput(LED_ROW_4_PORT, LED_ROW_4_PIN);
    setOutput(LED_ROW_5_PORT, LED_ROW_5_PIN);
    setOutput(LED_ROW_6_PORT, LED_ROW_6_PIN);

    //decoder
    setOutput(DEC_LM_A0_PORT, DEC_LM_A0_PIN);
    setOutput(DEC_LM_A1_PORT, DEC_LM_A1_PIN);
    setOutput(DEC_LM_A2_PORT, DEC_LM_A2_PIN);

    //configure analog
    //select pins
    setOutput(MUX_S0_PORT, MUX_S0_PIN);
    setOutput(MUX_S1_PORT, MUX_S1_PIN);
    setOutput(MUX_S2_PORT, MUX_S2_PIN);
    setOutput(MUX_S3_PORT, MUX_S3_PIN);

    //mux inputs
    setInput(MUX_1_IN_PORT, MUX_1_IN_PIN);
    setInput(MUX_2_IN_PORT, MUX_2_IN_PIN);

    //bootloader led
    setOutput(BTLDR_LED_PORT, BTLDR_LED_PIN);

}

void Board::initAnalog()    {

    setUpADC();

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

void Board::initEncoders()   {

    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)    {

        encoderData[i] |= ((uint16_t)0 << 8);
        encoderData[i] |= ((uint16_t)ENCODER_DEFAULT_PULSE_COUNT_STATE << 4);   //set number of pulses to 8

    }

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

uint8_t Board::getRGBIDFromLEDID(uint8_t ledID) {

    uint8_t row = ledID/NUMBER_OF_LED_COLUMNS;

    uint8_t mod = row%3;    //RGB LED = 3 normal LEDs
    row -= mod;

    uint8_t column = ledID % NUMBER_OF_BUTTON_COLUMNS;

    return (row*NUMBER_OF_LED_COLUMNS)/3 + column;

}

uint8_t Board::getLEDstate(uint8_t ledNumber)   {

    uint8_t returnValue;
    returnValue = ledState[ledNumber];
    return returnValue;

}

void Board::setSingleLED(uint8_t ledNumber, bool state, bool blinkMode)   {

    handleLED(ledNumber, state, blinkMode);

    if (blinkMode && state)
        ledBlinkingStart();
    else
        checkBlinkLEDs();

}

void Board::setRGBled(uint8_t ledNumber, rgb color, bool blinkMode) {

    handleLED(ledNumber, color, blinkMode);

    if (blinkMode && color.r && color.g && color.b)
        ledBlinkingStart();
    else
        checkBlinkLEDs();

}

void Board::setLEDblinkTime(uint16_t blinkTime)  {

    cli();
    ledBlinkTime = blinkTime*100;
    blinkTimerCounter = 0;
    sei();

}

void Board::setLEDfadeTime(uint8_t transitionSteps) {

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

void Board::handleLED(uint8_t ledNumber, rgb color, bool blinkMode)    {

    uint8_t led1 = getRGBfirstID(ledNumber);
    uint8_t led2 = led1 + NUMBER_OF_LED_COLUMNS*1;
    uint8_t led3 = led1 + NUMBER_OF_LED_COLUMNS*2;

    handleLED(led1, color.r, blinkMode);
    handleLED(led2, color.g, blinkMode);
    handleLED(led3, color.b, blinkMode);

}

void Board::handleLED(uint8_t ledNumber, bool state, bool blinkMode) {

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

    uint8_t currentState = getLEDstate(ledNumber);

    switch(state) {

        case false:
        //turn off the led
        currentState = 0;
        break;

        case true:
        //turn on the led
        //if led was already active, clear the on bits before setting new state
        if (bitRead(currentState, LED_ACTIVE_BIT))
        currentState = 0;

        bitWrite(currentState, LED_ACTIVE_BIT, 1);
        if (blinkMode)  {

            bitWrite(currentState, LED_BLINK_ON_BIT, 1);
            //this will turn the led immediately no matter how little time it's
            //going to blink first time
            bitWrite(currentState, LED_BLINK_STATE_BIT, 1);

        }   else bitWrite(currentState, LED_CONSTANT_ON_BIT, 1);
        break;

    }   ledState[ledNumber] = currentState;

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

int8_t Board::getEncoderState(uint8_t encoderNumber)  {

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

    uint8_t row = buttonIndex/NUMBER_OF_BUTTON_COLUMNS;
    uint8_t column = (NUMBER_OF_BUTTON_COLUMNS-1) - buttonIndex % NUMBER_OF_BUTTON_COLUMNS; //invert column order
    buttonIndex = column*8 + row;

    return !((inputMatrixBufferCopy >> buttonIndex) & 0x01);

}

Board board;
