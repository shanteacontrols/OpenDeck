/*

Copyright 2015-2022 Igor Petrovic

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
#include "board/common/constants/IO.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Atomic.h"
#include "core/src/general/Timing.h"
#include <Target.h>

#if defined(LED_INDICATORS) && defined(LED_INDICATORS_CTL)

namespace
{
    /// Variables used to control the time MIDI in/out LED indicators on board are active.
    /// When these LEDs need to be turned on, variables are set to value representing time in
    /// milliseconds during which they should be on. ISR decreases variable value by 1 every 1 millsecond.
    /// Once the variables have value 0, specific LED indicator is turned off.

    volatile uint8_t _midiInDINtimeout;
    volatile uint8_t _midiOutDINtimeout;

    volatile uint8_t _midiInUSBtimeout;
    volatile uint8_t _midiOutUSBtimeout;

    ///
}    // namespace

namespace Board
{
    namespace detail::io
    {
        void checkIndicators()
        {
            if (_midiInDINtimeout)
            {
                _midiInDINtimeout--;

                if (!_midiInDINtimeout)
                {
                    INT_LED_OFF(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                }
            }

            if (_midiOutDINtimeout)
            {
                _midiOutDINtimeout--;

                if (!_midiOutDINtimeout)
                {
                    INT_LED_OFF(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
                }
            }

            if (_midiInUSBtimeout)
            {
                _midiInUSBtimeout--;

                if (!_midiInUSBtimeout)
                {
                    INT_LED_OFF(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);
                }
            }

            if (_midiOutUSBtimeout)
            {
                _midiOutUSBtimeout--;

                if (!_midiOutUSBtimeout)
                {
                    INT_LED_OFF(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
                }
            }
        }
    }    // namespace detail::io

    namespace io
    {
        void indicateTraffic(dataSource_t source, dataDirection_t direction)
        {
            switch (source)
            {
            case dataSource_t::UART:
            {
                if (direction == dataDirection_t::INCOMING)
                {
                    INT_LED_ON(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                    _midiInDINtimeout = MIDI_INDICATOR_TIMEOUT;
                }
                else
                {
                    INT_LED_ON(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
                    _midiOutDINtimeout = MIDI_INDICATOR_TIMEOUT;
                }
            }
            break;

            case dataSource_t::USB:
            {
                if (direction == dataDirection_t::INCOMING)
                {
                    INT_LED_ON(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);
                    _midiInUSBtimeout = MIDI_INDICATOR_TIMEOUT;
                }
                else
                {
                    INT_LED_ON(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
                    _midiOutUSBtimeout = MIDI_INDICATOR_TIMEOUT;
                }
            }
            break;
            }
        }
    }    // namespace io
}    // namespace Board

#endif

#ifdef LED_INDICATORS

namespace Board::detail::io
{
    void ledFlashStartup()
    {
        for (int i = 0; i < 3; i++)
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

        INT_LED_OFF(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
        INT_LED_OFF(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
        INT_LED_OFF(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
        INT_LED_OFF(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);
    }
}    // namespace Board::detail::io

#endif