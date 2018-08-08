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

#include "board/Board.h"
#include "Common.h"
#include "HardwareControl.cpp"
#include "board/common/analog/input/Variables.h"
#include "board/common/indicators/Variables.h"


///
/// \brief Implementation of core variable used to keep track of run time in milliseconds.
///
volatile uint32_t rTime_ms;

#ifdef LED_INDICATORS
volatile uint8_t    midiIn_timeout;
volatile uint8_t    midiOut_timeout;
#endif

volatile bool       USBreceived;
volatile bool       USBsent;
volatile bool       UARTreceived;
volatile bool       UARTsent;


///
/// \brief Main interrupt service routine.
/// Used to control I/O on board and to update current run time.
///
ISR(CORE_ISR)
{
    static bool _1ms = true;
    _1ms = !_1ms;

    if (_1ms)
    {
        checkLEDs();

        rTime_ms++;

        #ifdef LED_INDICATORS
        if (USBreceived || UARTreceived)
        {
            INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
            USBreceived = false;
            UARTreceived = false;
            midiIn_timeout = MIDI_INDICATOR_TIMEOUT;
        }

        if (midiIn_timeout)
            midiIn_timeout--;
        else
            INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);

        if (USBsent || UARTsent)
        {
            INT_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
            USBsent = false;
            UARTsent = false;
            midiOut_timeout = MIDI_INDICATOR_TIMEOUT;
        }

        if (midiOut_timeout)
            midiOut_timeout--;
        else
            INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
        #endif
    }

    if (dIn_count < DIGITAL_IN_BUFFER_SIZE)
    {
        if (++dIn_head == DIGITAL_IN_BUFFER_SIZE)
            dIn_head = 0;

        storeDigitalIn();

        dIn_count++;
    }
}

///
/// \brief ADC ISR used to read values from multiplexers.
///
ISR(ADC_ISR)
{
    //always ignore first reading
    static bool ignoreFirst = true;

    if (!ignoreFirst)
    {
        analogBuffer[analogIndex] += ADC;
        analogIndex++;
        #ifdef USE_MUX
        activeMuxInput++;

        bool switchMux = (activeMuxInput == NUMBER_OF_MUX_INPUTS);

        if (switchMux)
        #else
        if (analogIndex == MAX_NUMBER_OF_ANALOG)
        #endif
        {
            #ifdef USE_MUX
            activeMuxInput = 0;
            activeMux++;

            if (activeMux == NUMBER_OF_MUX)
            {
                activeMux = 0;
            #endif
                analogIndex = 0;
                analogSampleCounter++;
            #ifdef USE_MUX
            }
            #endif

            #ifdef USE_MUX
            //switch to next mux once all mux inputs are read
            setADCchannel(adcChannelArray[activeMux]);
            #endif
        }

        //always switch to next read pin
        #ifdef USE_MUX
        setMuxInput();
        #else
        setADCchannel(adcChannelArray[analogIndex]);
        #endif
    }

    ignoreFirst = !ignoreFirst;

    if (analogSampleCounter != NUMBER_OF_ANALOG_SAMPLES)
        startADCconversion();
}
