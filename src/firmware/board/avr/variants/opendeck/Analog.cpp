/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

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
#include "Hardware.h"

///
/// \brief Holds currently active multiplexer.
///
static uint8_t      activeMux;

///
/// \brief Holds currently active multiplexer input.
///
static uint8_t      activeMuxInput;

///
/// \brief Holds currently active analog index which is being read.
/// Once all analog inputs are read, analog index is reset to 0.
///
static uint8_t      analogIndex;

///
/// brief Holds currently active sample count.
/// Once all analog inputs are read, sample count is increased.
///
volatile uint8_t    analogSampleCounter;

///
/// \brief Array in which analog samples are stored.
///
volatile int16_t    analogBuffer[MAX_NUMBER_OF_ANALOG];

///
/// Due to non-linearity of standard potentiometers on their extremes (low and high values),
/// hysteresis is used to avoid incorrect values. These arrays hold information on whether
/// low or high hysteresis values should be used.
/// @{

static uint8_t      lowHysteresisActive[MAX_NUMBER_OF_ANALOG/8+1];
static uint8_t      highHysteresisActive[MAX_NUMBER_OF_ANALOG/8+1];

/// @}

///
/// brief Checks if specified hysteresis is active for requested analog index.
/// @param[in] type     Hysteresis type. Enumerated type (see hysteresisType_t).
/// @param[in] analogID Analog index for which hysteresis state is being checked.
/// \returns True if hysteresis is currently active, false otherwise.
///
static bool isHysteresisActive(hysteresisType_t type, uint8_t analogID)
{
    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;

    if (type == lowHysteresis)
        return BIT_READ(lowHysteresisActive[arrayIndex], analogIndex);
    else
        return BIT_READ(highHysteresisActive[arrayIndex], analogIndex);
}

///
/// brief Enables or disables specific hysteresis type for specified analog index.
/// @param[in] type     Hysteresis type. Enumerated type (see hysteresisType_t).
/// @param[in] analogID Analog index for which hysteresis state is being changed.
/// @param[in] state    New hystersis state (true/enabled, false/disabled).
///
static void updateHysteresisState(hysteresisType_t type, uint8_t analogID, bool state)
{
    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;

    if (type == lowHysteresis)
        BIT_WRITE(lowHysteresisActive[arrayIndex], analogIndex, state);
    else
        BIT_WRITE(highHysteresisActive[arrayIndex], analogIndex, state);
}

///
/// \brief Configures one of 16 inputs/outputs on 4067 multiplexer.
///
inline void setMuxInput()
{
    BIT_READ(muxPinOrderArray[activeMuxInput], 0) ? setHigh(MUX_S0_PORT, MUX_S0_PIN) : setLow(MUX_S0_PORT, MUX_S0_PIN);
    BIT_READ(muxPinOrderArray[activeMuxInput], 1) ? setHigh(MUX_S1_PORT, MUX_S1_PIN) : setLow(MUX_S1_PORT, MUX_S1_PIN);
    BIT_READ(muxPinOrderArray[activeMuxInput], 2) ? setHigh(MUX_S2_PORT, MUX_S2_PIN) : setLow(MUX_S2_PORT, MUX_S2_PIN);
    BIT_READ(muxPinOrderArray[activeMuxInput], 3) ? setHigh(MUX_S3_PORT, MUX_S3_PIN) : setLow(MUX_S3_PORT, MUX_S3_PIN);
}

///
/// \brief Initializes analog variables and ADC peripheral.
///
void Board::initAnalog()
{
    disconnectDigitalInADC(MUX_1_IN_PIN);
    disconnectDigitalInADC(MUX_2_IN_PIN);

    adcConf adcConfiguration;

    adcConfiguration.prescaler = ADC_PRESCALER_128;
    adcConfiguration.vref = ADC_VREF_AREF;

    setUpADC(adcConfiguration);
    setMuxInput();
    setADCchannel(muxInPinArray[0]);

    _delay_ms(2);

    for (int i=0; i<5; i++)
        getADCvalue();  //few dummy reads to init ADC

    adcInterruptEnable();
}

///
/// \brief Checks if data from multiplexers is available.
/// Data is read in ISR and stored into samples array.
/// Once all mux inputs are read, data is considered available.
/// At this point, analogSampleCounter variable is set to invalid value
/// to stop further data reading from ISR until continueADCreadout
/// function is called.
/// \returns True if data is available, false otherwise.
///
bool Board::analogDataAvailable()
{
    return (analogSampleCounter == NUMBER_OF_ANALOG_SAMPLES);
}

///
/// \brief Resets active pad index and starts data acquisition from pads again.
///
void Board::continueADCreadout()
{
    analogSampleCounter = 0;
    analogBufferCounter = 0;
}

///
/// brief Checks for current analog value for specified analog index.
/// @param[in] analogID     Analog index for which ADC value is being checked.
/// \returns ADC value for requested analog index.
///
int16_t Board::getAnalogValue(uint8_t analogID)
{
    int16_t value;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        value = analogBuffer[analogID] >> ANALOG_SAMPLE_SHIFT;
        analogBuffer[analogID] = 0;
    }

    if (value > HYSTERESIS_THRESHOLD_HIGH)
    {
        updateHysteresisState(highHysteresis, analogID, true);
        updateHysteresisState(lowHysteresis, analogID, false);

        value += HYSTERESIS_ADDITION;

        if (value > 1023)
            return 1023;

        return value;
    }
    else
    {
        if (value < (HYSTERESIS_THRESHOLD_HIGH - HYSTERESIS_ADDITION))
        {
            //value is now either in non-hysteresis area or low hysteresis area

            updateHysteresisState(highHysteresis, analogID, false);

            if (value < (HYSTERESIS_THRESHOLD_LOW + HYSTERESIS_SUBTRACTION))
            {
                if (value < HYSTERESIS_THRESHOLD_LOW)
                {
                    updateHysteresisState(lowHysteresis, analogID, true);
                    value -= HYSTERESIS_SUBTRACTION;

                    if (value < 0)
                        value = 0;

                    return value;
                }
                else
                {
                    if (isHysteresisActive(lowHysteresis, analogID))
                    {
                        value -= HYSTERESIS_SUBTRACTION;

                        if (value < 0)
                            return 0;
                    }

                    return value;
                }
            }

            updateHysteresisState(lowHysteresis, analogID, false);
            updateHysteresisState(highHysteresis, analogID, false);

            return value;
        }
        else
        {
            if (isHysteresisActive(highHysteresis, analogID))
            {
                //high hysteresis still enabled
                value += HYSTERESIS_ADDITION;

                if (value > 1023)
                    return 1023;

                return value;
            }
            else
            {
                updateHysteresisState(highHysteresis, analogID, false);
                return value;
            }
        }
    }
}

///
/// brief Scales specified ADC value from minimum of 0 to maximum value specified.
/// @param[in] value    ADC value which is being scaled.
/// @param[in] maxValue Maximum value to which ADC value should be scaled.
///
uint16_t Board::scaleADC(uint16_t value, uint16_t maxValue)
{
    if (maxValue == 1023)
    {
        return value;
    }
    else if (maxValue == MIDI_7_BIT_VALUE_MAX)
    {
        return value >> 3;
    }
    else if (maxValue == MIDI_14_BIT_VALUE_MAX)
    {
        return value << 4;
    }
    else
    {
        //use mapRange_uint32 to avoid overflow issues
        return mapRange_uint32(value, 0, 1023, 0, maxValue);
    }
}

///
/// \brief ADC ISR used to read values from multiplexers.
///
ISR(ADC_vect)
{
    analogBuffer[analogBufferCounter] += ADC;
    analogBufferCounter++;
    activeMuxInput++;

    bool switchMux = (activeMuxInput == NUMBER_OF_MUX_INPUTS);

    if (switchMux)
    {
        activeMuxInput = 0;
        activeMux++;

        if (activeMux == NUMBER_OF_MUX)
        {
            activeMux = 0;
            analogBufferCounter = 0;
            analogSampleCounter++;
        }

        setADCchannel(muxInPinArray[activeMux]);
    }

    //always set mux input
    setMuxInput();
}
