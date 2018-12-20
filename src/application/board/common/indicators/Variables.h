/*

Copyright 2015-2018 Igor Petrovic

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
    namespace detail
    {
        ///
        /// \brief Variables indicating whether MIDI in/out LED indicators should be turned on.
        /// External definition, must be implemented for specific board in order to use it.
        /// State of these variables is set to true externally when MIDI event happens.
        /// Handling should check if their state is true, and if it is, LED indicator should
        /// be turned on, variable state should be set to false and timeout countdown should be started.
        /// @{

        extern volatile bool        USBreceived;
        extern volatile bool        USBsent;
        extern volatile bool        UARTreceived;
        extern volatile bool        UARTsent;

        /// @}

        #ifndef USB_SUPPORTED
        ///
        /// \brief Flag signaling whether or not USB link is connected to host.
        ///
        extern bool                 usbLinkState;
        #endif
    }
}