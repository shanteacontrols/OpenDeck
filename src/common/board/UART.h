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

#include <inttypes.h>
#include "DataTypes.h"

namespace Board
{
    namespace UART
    {
        ///
        /// \brief Deinitializes specified UART channel.
        /// @param [in] channel UART channel on MCU.
        ///
        void reset(uint8_t channel);

        ///
        /// \brief Initializes UART peripheral.
        /// @param [in] baudRate    UART speed (baudrate).
        /// @param [in] channel     UART channel on MCU.
        ///
        void init(uint32_t baudRate, uint8_t channel);

        ///
        /// \brief Used to read MIDI data from RX UART buffer.
        /// @param [in]     channel     UART channel on MCU.
        /// @param [in,out] data        Pointer to variable in which read data is being stored.
        /// \returns False if buffer is empty, true otherwise.
        ///
        bool read(uint8_t channel, uint8_t& data);

        ///
        /// \brief Used to write MIDI data to UART TX buffer.
        /// @param [in] channel     UART channel on MCU.
        /// @param [in] data        Byte of data to write.
        /// \returns True on success. Since this function waits until
        /// outgoig buffer is full, result will always be success (1).
        ///
        bool write(uint8_t channel, uint8_t data);

        ///
        /// \brief Used to enable or disable UART loopback functionality.
        /// Used to pass incoming UART data to TX channel immediately.
        /// @param [in] channel UART channel on MCU.
        /// @param [in] state   New state of loopback functionality (true/enabled, false/disabled).
        ///
        void setLoopbackState(uint8_t channel, bool state);

        ///
        /// \brief Checks whether or not UART loopback functionality is enabled.
        /// @param [in] channel UART channel on MCU.
        ///
        bool getLoopbackState(uint8_t channel);

        ///
        /// \brief Checks if all data on specified UART channel has been sent.
        /// @param [in] channel UART channel on MCU.
        /// \returns True if there is no more data to transmit, false otherwise.
        ///
        bool isTxEmpty(uint8_t channel);

        ///
        /// \brief Checks how many bytes are stored in incoming buffer.
        /// @param [in] channel UART channel on MCU.
        /// \returns Number of available bytes.
        ///
        uint8_t bytesAvailableRx(uint8_t channel);

        ///
        /// \brief Used to indicate whether or not UART event has occured (packet sent or received).
        ///
        extern trafficIndicator_t trafficIndicator;
    }    // namespace UART
}    // namespace Board