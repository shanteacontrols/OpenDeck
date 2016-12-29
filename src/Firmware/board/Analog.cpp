#include "Board.h"
#include "Variables.h"

#define ANALOG_BUFFER_SIZE  MAX_NUMBER_OF_ANALOG

volatile bool       _analogDataAvailable;
uint8_t             activeMux,
                    activeMuxInput,
                    analogBufferCounter;

volatile uint16_t   analogBuffer[ANALOG_BUFFER_SIZE];
uint32_t            retrievedData;


void Board::initAnalog()
{
    disconnectDigitalInADC(MUX_1_IN_PIN);
    disconnectDigitalInADC(MUX_2_IN_PIN);

    setUpADC();

    setMuxInput(activeMuxInput);
    setADCchannel(MUX_1_IN_PIN);

    _delay_ms(2);

    for (int i=0; i<5; i++)
        getADCvalue();  //few dummy reads to init ADC

    adcInterruptEnable();
    startADCconversion();
}

inline void setMuxInput(uint8_t muxInput)
{
    //add NOPs to compensate for propagation delay

    bitRead(muxPinOrderArray[muxInput], 0) ? setHigh(MUX_S0_PORT, MUX_S0_PIN) : setLow(MUX_S0_PORT, MUX_S0_PIN);
    bitRead(muxPinOrderArray[muxInput], 1) ? setHigh(MUX_S1_PORT, MUX_S1_PIN) : setLow(MUX_S1_PORT, MUX_S1_PIN);
    bitRead(muxPinOrderArray[muxInput], 2) ? setHigh(MUX_S2_PORT, MUX_S2_PIN) : setLow(MUX_S2_PORT, MUX_S2_PIN);
    bitRead(muxPinOrderArray[muxInput], 3) ? setHigh(MUX_S3_PORT, MUX_S3_PIN) : setLow(MUX_S3_PORT, MUX_S3_PIN);

    _NOP(); _NOP(); _NOP();
    _NOP(); _NOP(); _NOP();
    _NOP(); _NOP(); _NOP();
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

    return state;
}

uint16_t Board::getAnalogValue(uint8_t analogID)
{
    uint16_t value;
    value = analogBuffer[analogID];
    bitSet(retrievedData, analogID);

    if (retrievedData == 0xFFFFFFFF)
    {
        //all data is retrieved, start new readout
        retrievedData = 0;
        startADCconversion();
        _analogDataAvailable = false;
    }

    return value;
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
