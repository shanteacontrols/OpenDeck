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

#pragma once

namespace Board
{
    namespace setup
    {
        ///
        /// \brief Initializes all pins to correct states.
        ///
        void pins();

#ifndef BOARD_A_xu2
        ///
        /// \brief Initializes analog variables and ADC peripheral.
        ///
        void adc();
#endif

#ifdef USB_MIDI_SUPPORTED
        ///
        /// \brief Initializes USB peripheral and configures it as MIDI device.
        ///
        void usb();
#endif

        ///
        /// \brief Initializes main and PWM timers.
        ///
        void timers();
    }    // namespace setup
}    // namespace Board