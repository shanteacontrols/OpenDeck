#include "Board.h"
#include "Variables.h"

#define ANALOG_BUFFER_SIZE  MAX_NUMBER_OF_ANALOG

volatile bool       _analogDataAvailable = false;
uint8_t             activeMux = 0,
                    activeMuxInput = 0,
                    analogBufferCounter,
                    adcDelayCounter;

volatile int16_t    analogBuffer[ANALOG_BUFFER_SIZE];
int16_t             analogBufferCopy[ANALOG_BUFFER_SIZE];


void Board::initAnalog()
{
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

inline void setMuxInput(uint8_t muxInput)
{
    //according to datasheet, propagation delay between setting Sn pins
    //and output appearing on Yn is around 150ns
    //add three NOPs to compensate

    bitRead(muxPinOrderArray[muxInput], 0) ? setHigh(MUX_S0_PORT, MUX_S0_PIN) : setLow(MUX_S0_PORT, MUX_S0_PIN);
    bitRead(muxPinOrderArray[muxInput], 1) ? setHigh(MUX_S1_PORT, MUX_S1_PIN) : setLow(MUX_S1_PORT, MUX_S1_PIN);
    bitRead(muxPinOrderArray[muxInput], 2) ? setHigh(MUX_S2_PORT, MUX_S2_PIN) : setLow(MUX_S2_PORT, MUX_S2_PIN);
    bitRead(muxPinOrderArray[muxInput], 3) ? setHigh(MUX_S3_PORT, MUX_S3_PIN) : setLow(MUX_S3_PORT, MUX_S3_PIN);

    _NOP(); _NOP(); _NOP();
}

inline void setAnalogPin(uint8_t muxNumber)
{
    uint8_t analogPin;

    switch(muxNumber)
    {
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

bool Board::analogDataAvailable()
{
    bool state;
    state = _analogDataAvailable;

    if (state)
    {
        //no cli/sei needed since adc conversion is stopped at the moment
        for (int i=0; i<ANALOG_BUFFER_SIZE; i++)
            analogBufferCopy[i] = analogBuffer[i];

        _analogDataAvailable = false;
        adcDelayCounter = 0;
        return true;
    }

    return false;
}

int16_t Board::getAnalogValue(uint8_t analogID)
{
    return analogBufferCopy[analogID];
}

ISR(ADC_vect)
{
    analogBuffer[analogBufferCounter] = ADC;
    analogBufferCounter++;

    activeMuxInput++;

    bool switchMux = (activeMuxInput == NUMBER_OF_MUX_INPUTS);
    bool bufferFull = (analogBufferCounter == MAX_NUMBER_OF_ANALOG);

    if (switchMux)
    {
        activeMuxInput = 0;
        activeMux++;

        if (activeMux == NUMBER_OF_MUX)
        activeMux = 0;

        setAnalogPin(activeMux);
    }

    if (bufferFull)
    {
        analogBufferCounter = 0;
        _analogDataAvailable = true;
    }

    //always set mux input
    setMuxInput(activeMuxInput);

    if (!bufferFull)
        startADCconversion();
}
