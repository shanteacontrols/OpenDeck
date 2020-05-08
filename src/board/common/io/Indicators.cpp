/*

Copyright 2015-2020 Igor Petrovic

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
#include "board/common/io/Helpers.h"
#include "board/Internal.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Atomic.h"
#include "core/src/general/Timing.h"
#include "Pins.h"

#ifdef LED_INDICATORS

namespace
{
    ///
    /// \brief Variables used to control the time MIDI in/out LED indicators on board are active.
    /// When these LEDs need to be turned on, variables are set to value representing time in
    /// milliseconds during which they should be on. ISR decreases variable value by 1 every 1 millsecond.
    /// Once the variables have value 0, specific LED indicator is turned off.
    /// @{

    volatile uint8_t midiInDINtimeout;
    volatile uint8_t midiOutDINtimeout;

    volatile uint8_t midiInUSBtimeout;
    volatile uint8_t midiOutUSBtimeout;

    /// @}

    bool indicatorsDisabled;
}    // namespace

namespace Board
{
    namespace detail
    {
        namespace io
        {
            void enableIndicators()
            {
                ATOMIC_SECTION
                {
                    midiInDINtimeout   = 0;
                    midiOutDINtimeout  = 0;
                    midiInUSBtimeout   = 0;
                    midiOutUSBtimeout  = 0;
                    indicatorsDisabled = false;
                }
            }

            void disableIndicators()
            {
                indicatorsDisabled = true;
            }

            void indicateMIDItraffic(MIDI::interface_t source, midiTrafficDirection_t direction)
            {
                switch (source)
                {
                case MIDI::interface_t::din:
                    if (direction == midiTrafficDirection_t::incoming)
                    {
                        INT_LED_ON(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                        midiInDINtimeout = MIDI_INDICATOR_TIMEOUT;
                    }
                    else
                    {
                        INT_LED_ON(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
                        midiOutDINtimeout = MIDI_INDICATOR_TIMEOUT;
                    }
                    break;

                case MIDI::interface_t::usb:
                    if (direction == midiTrafficDirection_t::incoming)
                    {
                        INT_LED_ON(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);
                        midiInUSBtimeout = MIDI_INDICATOR_TIMEOUT;
                    }
                    else
                    {
                        INT_LED_ON(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
                        midiOutUSBtimeout = MIDI_INDICATOR_TIMEOUT;
                    }
                    break;
                }
            }

            void checkIndicators()
            {
                if (indicatorsDisabled)
                    return;

                if (midiInDINtimeout)
                    midiInDINtimeout--;
                else
                    INT_LED_OFF(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);

                if (midiOutDINtimeout)
                    midiOutDINtimeout--;
                else
                    INT_LED_OFF(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);

                if (midiInUSBtimeout)
                    midiInUSBtimeout--;
                else
                    INT_LED_OFF(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);

                if (midiOutUSBtimeout)
                    midiOutUSBtimeout--;
                else
                    INT_LED_OFF(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
            }

            void ledFlashStartup(bool fwUpdated)
            {
                Board::detail::io::disableIndicators();

                for (int i = 0; i < 3; i++)
                {
                    if (fwUpdated)
                    {
                        INT_LED_ON(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
                        INT_LED_ON(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
                        INT_LED_OFF(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                        INT_LED_OFF(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);
                        core::timing::waitMs(LED_INDICATOR_STARTUP_DELAY);
                        INT_LED_OFF(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
                        INT_LED_OFF(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
                        INT_LED_ON(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                        INT_LED_ON(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);
                        core::timing::waitMs(LED_INDICATOR_STARTUP_DELAY);
                    }
                    else
                    {
                        INT_LED_ON(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
                        INT_LED_ON(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
                        INT_LED_ON(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                        INT_LED_ON(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);
                        core::timing::waitMs(LED_INDICATOR_STARTUP_DELAY);
                        INT_LED_OFF(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
                        INT_LED_OFF(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
                        INT_LED_OFF(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                        INT_LED_OFF(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);
                        core::timing::waitMs(LED_INDICATOR_STARTUP_DELAY);
                    }
                }

                INT_LED_OFF(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
                INT_LED_OFF(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                INT_LED_OFF(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
                INT_LED_OFF(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);

                Board::detail::io::enableIndicators();
            }
        }    // namespace io
    }        // namespace detail
}    // namespace Board

#endif