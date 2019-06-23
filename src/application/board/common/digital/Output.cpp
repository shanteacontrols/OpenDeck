/*

Copyright 2015-2019 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "board/Board.h"
#include "board/common/constants/LEDs.h"
#include "board/common/digital/Output.h"
#include "board/common/Map.h"
#include "core/src/general/BitManipulation.h"
#include "Pins.h"
#include "interface/digital/output/leds/Helpers.h"
#ifndef BOARD_A_xu2
#include "interface/digital/output/leds/LEDs.h"
#endif

#ifdef LED_INDICATORS
namespace
{
    ///
    /// \brief Variables used to control the time MIDI in/out LED indicators on board are active.
    /// When these LEDs need to be turned on, variables are set to value representing time in
    /// milliseconds during which they should be on. ISR decreases variable value by 1 every 1 millsecond.
    /// Once the variables have value 0, specific LED indicator is turned off.
    /// @{

    volatile uint8_t midiIn_timeout;
    volatile uint8_t midiOut_timeout;

    /// @}
}    // namespace
#endif

#ifndef BOARD_A_xu2
namespace
{
    volatile uint8_t pwmSteps;
    volatile int8_t  transitionCounter[MAX_NUMBER_OF_LEDS];

    ///
    /// \brief Array holding values needed to achieve more natural LED transition between states.
    ///
    const uint8_t ledTransitionScale[NUMBER_OF_LED_TRANSITIONS] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        1,
        1,
        1,
        1,
        2,
        2,
        2,
        2,
        3,
        3,
        4,
        4,
        5,
        5,
        6,
        6,
        7,
        8,
        9,
        10,
        11,
        12,
        13,
        14,
        16,
        17,
        19,
        21,
        23,
        25,
        28,
        30,
        33,
        36,
        40,
        44,
        48,
        52,
        57,
        62,
        68,
        74,
        81,
        89,
        97,
        106,
        115,
        126,
        138,
        150,
        164,
        179,
        195,
        213,
        255
    };

#ifdef NUMBER_OF_LED_COLUMNS
    ///
    /// \brief Holds value of currently active output matrix column.
    ///
    volatile uint8_t activeOutColumn;
#endif
}    // namespace
#endif

namespace Board
{
    namespace interface
    {
        namespace digital
        {
            namespace output
            {
#ifndef BOARD_A_xu2
#ifdef LEDS_SUPPORTED
                bool setLEDfadeSpeed(uint8_t transitionSpeed)
                {
                    if (transitionSpeed > FADE_TIME_MAX)
                    {
                        return false;
                    }

//reset transition counter
#ifdef __AVR__
                    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
#endif
                    {
                        for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
                            transitionCounter[i] = 0;

                        pwmSteps = transitionSpeed;
                    }

                    return true;
                }

                uint8_t getRGBaddress(uint8_t rgbID, Interface::digital::output::LEDs::rgbIndex_t index)
                {
#ifdef NUMBER_OF_LED_COLUMNS
                    uint8_t column = rgbID % NUMBER_OF_LED_COLUMNS;
                    uint8_t row = (rgbID / NUMBER_OF_LED_COLUMNS) * 3;
                    uint8_t address = column + NUMBER_OF_LED_COLUMNS * row;

                    switch (index)
                    {
                    case Interface::digital::output::LEDs::rgbIndex_t::r:
                        return address;

                    case Interface::digital::output::LEDs::rgbIndex_t::g:
                        return address + NUMBER_OF_LED_COLUMNS * 1;

                    case Interface::digital::output::LEDs::rgbIndex_t::b:
                        return address + NUMBER_OF_LED_COLUMNS * 2;
                    }

                    return 0;
#else
                    return rgbID * 3 + static_cast<uint8_t>(index);
#endif
                }

                uint8_t getRGBID(uint8_t ledID)
                {
#ifdef NUMBER_OF_LED_COLUMNS
                    uint8_t row = ledID / NUMBER_OF_LED_COLUMNS;

                    uint8_t mod = row % 3;
                    row -= mod;

                    uint8_t column = ledID % NUMBER_OF_LED_COLUMNS;

                    uint8_t result = (row * NUMBER_OF_LED_COLUMNS) / 3 + column;

                    if (result >= MAX_NUMBER_OF_RGB_LEDS)
                        return MAX_NUMBER_OF_RGB_LEDS - 1;
                    else
                        return result;
#else
                    uint8_t result = ledID / 3;

                    if (result >= MAX_NUMBER_OF_RGB_LEDS)
                        return MAX_NUMBER_OF_RGB_LEDS - 1;
                    else
                        return result;
#endif
                }
#endif
#endif

                namespace detail
                {
#ifdef LED_INDICATORS
                    void checkIndicators()
                    {
                        if (Board::USB::trafficIndicator.received || Board::UART::trafficIndicator.received)
                        {
                            INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
                            Board::USB::trafficIndicator.received = false;
                            Board::UART::trafficIndicator.received = false;
                            midiIn_timeout = MIDI_INDICATOR_TIMEOUT;
                        }

                        if (midiIn_timeout)
                            midiIn_timeout--;
                        else
                            INT_LED_OFF(LED_IN_PORT, LED_IN_PIN);

                        if (Board::USB::trafficIndicator.sent || Board::UART::trafficIndicator.sent)
                        {
                            INT_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
                            Board::USB::trafficIndicator.sent = false;
                            Board::UART::trafficIndicator.sent = false;
                            midiOut_timeout = MIDI_INDICATOR_TIMEOUT;
                        }

                        if (midiOut_timeout)
                            midiOut_timeout--;
                        else
                            INT_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
                    }
#endif

#ifndef BOARD_A_xu2
#ifdef NUMBER_OF_LED_COLUMNS
                    ///
                    /// \brief Switches to next LED matrix column.
                    ///
                    inline void activateOutputColumn()
                    {
                        BIT_READ(activeOutColumn, 0) ? setHigh(DEC_LM_A0_PORT, DEC_LM_A0_PIN) : setLow(DEC_LM_A0_PORT, DEC_LM_A0_PIN);
                        BIT_READ(activeOutColumn, 1) ? setHigh(DEC_LM_A1_PORT, DEC_LM_A1_PIN) : setLow(DEC_LM_A1_PORT, DEC_LM_A1_PIN);
                        BIT_READ(activeOutColumn, 2) ? setHigh(DEC_LM_A2_PORT, DEC_LM_A2_PIN) : setLow(DEC_LM_A2_PORT, DEC_LM_A2_PIN);
                    }

                    namespace
                    {
                        Board::mcuPin_t pin;
                        uint8_t         ledNumber;
                        uint8_t         ledStateSingle;
                    }    // namespace

                    void checkDigitalOutputs()
                    {
                        for (int i = 0; i < NUMBER_OF_LED_ROWS; i++)
                            Board::map::ledRowOff(i);

                        activateOutputColumn();

                        //if there is an active LED in current column, turn on LED row
                        //do fancy transitions here
                        for (int i = 0; i < NUMBER_OF_LED_ROWS; i++)
                        {
                            ledNumber = activeOutColumn + i * NUMBER_OF_LED_COLUMNS;
                            ledStateSingle = LED_ON(Interface::digital::output::LEDs::getLEDstate(ledNumber));

                            ledStateSingle *= (NUMBER_OF_LED_TRANSITIONS - 1);

                            //don't bother with pwm if it's disabled
                            if (!pwmSteps && ledStateSingle)
                            {
                                Board::map::ledRowOn(i
#ifdef LED_FADING
                                                     ,
                                                     255
#endif
                                );
                            }
                            else
                            {
                                if (ledTransitionScale[transitionCounter[ledNumber]])
                                    Board::map::ledRowOn(i
#ifdef LED_FADING
                                                         ,
                                                         ledTransitionScale[transitionCounter[ledNumber]]
#endif
                                    );

                                if (transitionCounter[ledNumber] != ledStateSingle)
                                {
                                    //fade up
                                    if (transitionCounter[ledNumber] < ledStateSingle)
                                    {
                                        transitionCounter[ledNumber] += pwmSteps;

                                        if (transitionCounter[ledNumber] > ledStateSingle)
                                            transitionCounter[ledNumber] = ledStateSingle;
                                    }
                                    else
                                    {
                                        //fade down
                                        transitionCounter[ledNumber] -= pwmSteps;

                                        if (transitionCounter[ledNumber] < 0)
                                            transitionCounter[ledNumber] = 0;
                                    }
                                }
                            }
                        }

                        if (++activeOutColumn == NUMBER_OF_LED_COLUMNS)
                            activeOutColumn = 0;
                    }
#else
                    namespace
                    {
                        uint8_t lastLEDstate[MAX_NUMBER_OF_LEDS];
                        uint8_t ledStateSingle;
                    }    // namespace

#ifdef NUMBER_OF_OUT_SR
                    ///
                    /// \brief Checks if any LED state has been changed and writes changed state to output shift registers.
                    ///
                    void checkDigitalOutputs()
                    {
                        bool updateSR = false;

                        for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
                        {
                            ledStateSingle = LED_ON(Interface::digital::output::LEDs::getLEDstate(i));

                            if (ledStateSingle != lastLEDstate[i])
                            {
                                lastLEDstate[i] = ledStateSingle;
                                updateSR = true;
                            }
                        }

                        if (updateSR)
                        {
                            setLow(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

                            for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
                            {
                                LED_ON(Interface::digital::output::LEDs::getLEDstate(i)) ? EXT_LED_ON(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN) : EXT_LED_OFF(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
                                pulseHighToLow(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                            }

                            setHigh(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);
                        }
                    }
#else
                    namespace
                    {
                        Board::mcuPin_t pin;
                    }

                    void checkDigitalOutputs()
                    {
                        for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
                        {
                            ledStateSingle = LED_ON(Interface::digital::output::LEDs::getLEDstate(i));
                            pin = Board::map::led(i);

                            if (ledStateSingle != lastLEDstate[i])
                            {
                                if (ledStateSingle)
                                    EXT_LED_ON(*pin.port, pin.pin);
                                else
                                    EXT_LED_OFF(*pin.port, pin.pin);

                                lastLEDstate[i] = ledStateSingle;
                            }
                        }
                    }
#endif
#endif
#endif
                }    // namespace detail
            }        // namespace output
        }            // namespace digital
    }                // namespace interface
}    // namespace Board