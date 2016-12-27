#include "Board.h"
#include "Variables.h"

//run time in milliseconds
volatile uint32_t rTime_ms;

inline void activateInputColumn(uint8_t column)
{
    bitRead(dmColumnArray[column], 0) ? setHigh(DEC_DM_A0_PORT, DEC_DM_A0_PIN) : setLow(DEC_DM_A0_PORT, DEC_DM_A0_PIN);
    bitRead(dmColumnArray[column], 1) ? setHigh(DEC_DM_A1_PORT, DEC_DM_A1_PIN) : setLow(DEC_DM_A1_PORT, DEC_DM_A1_PIN);
    bitRead(dmColumnArray[column], 2) ? setHigh(DEC_DM_A2_PORT, DEC_DM_A2_PIN) : setLow(DEC_DM_A2_PORT, DEC_DM_A2_PIN);

    _NOP();
}

inline void storeDigitalIn(uint8_t column, uint8_t bufferIndex)
{
    uint8_t data = 0;
    uint8_t dataReorder = 0;

    //make room for new data
    inputBuffer[bufferIndex] <<= 8;

    //pulse latch pin
    pulseLowToHigh(SR_LATCH_PORT, SR_LATCH_PIN);

    for (int i=0; i<8; i++)
    {
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

inline void activateOutputColumn(uint8_t column)
{
    bitRead(column, 0) ? setHigh(DEC_LM_A0_PORT, DEC_LM_A0_PIN) : setLow(DEC_LM_A0_PORT, DEC_LM_A0_PIN);
    bitRead(column, 1) ? setHigh(DEC_LM_A1_PORT, DEC_LM_A1_PIN) : setLow(DEC_LM_A1_PORT, DEC_LM_A1_PIN);
    bitRead(column, 2) ? setHigh(DEC_LM_A2_PORT, DEC_LM_A2_PIN) : setLow(DEC_LM_A2_PORT, DEC_LM_A2_PIN);

    _NOP();
}

inline void ledRowsOff()
{
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

inline void ledRowOn(uint8_t rowNumber, uint8_t intensity)
{
    switch (rowNumber)
    {
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

inline void checkLEDs()
{
    if (blinkEnabled)
    {
        if (!blinkTimerCounter)
        {
            //change blinkBit state and write it into ledState variable if LED is in blink state
            for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            {
                if (bitRead(ledState[i], LED_BLINK_ON_BIT))
                {
                    if (blinkState)
                    bitWrite(ledState[i], LED_BLINK_STATE_BIT, 1);
                    else
                    bitWrite(ledState[i], LED_BLINK_STATE_BIT, 0);
                }
            }

            blinkState = !blinkState;
        }
    }

    //if there is an active LED in current column, turn on LED row
    //do fancy transitions here
    for (int i=0; i<NUMBER_OF_LED_ROWS; i++)
    {
        uint8_t ledNumber = activeLEDcolumn+i*NUMBER_OF_LED_COLUMNS;
        uint8_t ledStateSingle = ledOnLookUpTable[ledState[ledNumber]];

        //don't bother with pwm if it's disabled
        if (!pwmSteps && ledStateSingle)
        {
            #ifdef LED_INVERT
            ledRowOn(i, 0);
            #else
            ledRowOn(i, 255);
            #endif
        }
        else
        {
            if (
            (ledStateSingle && (transitionCounter[ledNumber] != (NUMBER_OF_LED_TRANSITIONS-1))) ||
            (!ledStateSingle && transitionCounter[ledNumber])
            )
            {
                if (ledStateSingle)
                transitionCounter[ledNumber] += pwmSteps;
                else
                transitionCounter[ledNumber] -= pwmSteps;

                if (transitionCounter[ledNumber] >= NUMBER_OF_LED_TRANSITIONS)
                transitionCounter[ledNumber] = NUMBER_OF_LED_TRANSITIONS-1;
                if (transitionCounter[ledNumber] < 0)
                transitionCounter[ledNumber] = 0;
            }

            if (transitionCounter[ledNumber])
            {
                #ifdef LED_INVERT
                ledRowOn(i, 255-ledTransitionScale[transitionCounter[ledNumber]]);
                #else
                ledRowOn(i, ledTransitionScale[transitionCounter[ledNumber]]);
                #endif
            }
        }
    }
}

ISR(TIMER0_COMPA_vect)
{
    //run millis and blink update every 1ms
    //update led matrix every 1.5ms
    //update button matrix each time

    static bool updateMillisAndBlink = false;
    static uint8_t matrixSwitchCounter = 0;
    //uint32_t ms;

    if (matrixSwitchCounter == 1)
    {
        ledRowsOff();

        if (activeLEDcolumn == NUMBER_OF_LED_COLUMNS)
            activeLEDcolumn = 0;

        activateOutputColumn(activeLEDcolumn);
        checkLEDs();
        activeLEDcolumn++;
        matrixSwitchCounter = 0;
    }

    if (updateMillisAndBlink)
    {
        //ms = rTime_ms;
        //ms++;
        ////update run time
        //rTime_ms = ms;

        matrixSwitchCounter++;
    }
    else
    {
        if (blinkEnabled)
        {
            blinkTimerCounter++;
            if (blinkTimerCounter >= ledBlinkTime)
                blinkTimerCounter = 0;
        }
    }

    updateMillisAndBlink = !updateMillisAndBlink;

    //read input matrix
    uint8_t bufferIndex = digital_buffer_head + 1;

    if (bufferIndex >= DIGITAL_BUFFER_SIZE)
        bufferIndex = 0;

    if (digital_buffer_tail == bufferIndex)
        return; //buffer full, exit

    inputBuffer[bufferIndex] = 0;
    digital_buffer_head = bufferIndex;

    for (int i=0; i<NUMBER_OF_BUTTON_COLUMNS; i++)
    {
        activateInputColumn(i);
        storeDigitalIn(i, bufferIndex);
    }

    if (!_analogDataAvailable && !bitRead(ADCSRA, ADSC))
    {
        adcDelayCounter++;

        if (adcDelayCounter == 2)
        {
            adcDelayCounter = 0;
            startADCconversion();
        }
    }
}
