/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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
#include "Variables.h"

volatile bool       _analogDataAvailable;
uint8_t             activeMux,
                    activeMuxInput,
                    analogBufferCounter;

volatile uint16_t   analogBuffer[ANALOG_BUFFER_SIZE];
uint32_t            retrievedData;
bool                muxHysteresisActive[ANALOG_BUFFER_SIZE/8+1];

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

bool Board::getHysteresisState(uint8_t analogID)
{
    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;

    return bitRead(muxHysteresisActive[arrayIndex], analogIndex);
}

void Board::setHysteresisState(uint8_t analogID, bool state)
{
    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;

    //update state if it's different than last one
    if (bitRead(muxHysteresisActive[arrayIndex], analogIndex) != state)
        bitWrite(muxHysteresisActive[arrayIndex], analogIndex, state);
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

    #ifdef ENABLE_HYSTERESIS
    if (value > HYSTERESIS_THRESHOLD)
    {
        //enable hysteresis
        setHysteresisState(analogID, true);

        value += HYSTERESIS_ADDITION;

        if (value > 1023)
            return 1023;

        return value;
    }
    else
    {
        if (getHysteresisState(analogID))
        {
            //hysteresis is enabled
            if (value > (HYSTERESIS_THRESHOLD-HYSTERESIS_ADDITION))
            {
                value += HYSTERESIS_ADDITION;

                if (value > 1023)
                    return 1023;

                return value;
            }
            else
            {
                //disable hysteresis now
                setHysteresisState(analogID, false);
                return value;
            }
        }

        return value;
    }
    #else
    return value;
    #endif
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
