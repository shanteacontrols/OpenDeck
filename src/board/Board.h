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

#pragma once

#include <inttypes.h>
#include "midi/src/MIDI.h"

namespace Board
{
    ///
    /// \brief List of all possible reboot types.
    ///
    enum class rebootType_t : uint8_t
    {
        rebootApp,     ///< Reboot to application.
        rebootBtldr    ///< Reboot to bootloader.
    };

#ifdef UID_BITS
    ///
    /// \brief Structure holding unique ID for MCU.
    ///
    typedef struct
    {
        uint8_t uid[UID_BITS / 8];
    } uniqueID_t;

    ///
    /// \brief Retrieves MCU-specific unique ID number.
    /// @param [in] uid Reference to structure in which retrieved UID will be stored.
    ///
    void uniqueID(uniqueID_t& uid);
#endif

    ///
    /// \brief Perfoms initialization of MCU and all board peripherals.
    ///
    void init();

    ///
    /// \brief Performs software MCU reboot.
    /// @param [in] type    Type of reset to perform. See rebootType_t enumeration.
    ///
    void reboot(rebootType_t type);

    namespace USB
    {
        ///
        /// \brief Used to read MIDI data from USB interface.
        /// @param [in] USBMIDIpacket   Pointer to structure in which MIDI data is stored.
        /// \returns True if data is available, false otherwise.
        ///
        bool readMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket);

        ///
        /// \brief Used to write MIDI data to USB interface.
        /// @param [in] USBMIDIpacket   Pointer to structure holding MIDI data to write.
        /// \returns True if data is available, false otherwise.
        ///
        bool writeMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket);
    }    // namespace USB

    namespace UART
    {
        ///
        /// \brief Initializes UART peripheral.
        /// @param [in] channel     UART channel on MCU.
        /// @param [in] baudRate    UART speed (baudrate).
        ///
        bool init(uint8_t channel, uint32_t baudRate);

        ///
        /// \brief Deinitializes specified UART channel.
        /// @param [in] channel UART channel on MCU.
        ///
        bool deInit(uint8_t channel);

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
        /// \brief Checks if all data on specified UART channel has been sent.
        /// @param [in] channel UART channel on MCU.
        /// \returns True if there is no more data to transmit, false otherwise.
        ///
        bool isTxEmpty(uint8_t channel);
    }    // namespace UART

    namespace I2C
    {
        enum class clockSpeed_t : uint32_t
        {
            _1kHz = 100000
        };

        ///
        /// \brief Initializes I2C peripheral on the MCU.
        /// @param [in] clockSpeed  I2C interface speed.
        /// \returns True on success, false otherwise.
        ///
        bool init(clockSpeed_t clockSpeed);

        ///
        /// \brief Denitializes I2C peripheral on the MCU.
        /// \returns True on success, false otherwise.
        ///
        bool deInit();

        ///
        /// \brief Write data to I2C slave on specified address.
        /// @param [in] address 7-bit slave address without R/W bit.
        /// @param [in] buffer  Buffer containing data to send.
        /// @param [in] size    Number of bytes to send.
        /// \returns True on success, false otherwise.
        ///
        bool write(uint8_t address, uint8_t* data, size_t size);
    }    // namespace I2C

    namespace io
    {
        enum class rgbIndex_t : uint8_t
        {
            r,
            g,
            b
        };

        ///
        /// \brief Checks if digital input data is available (encoder and button data).
        /// Digital input data is read in ISR and stored into ring buffer.
        /// \returns True if data is available, false otherwise.
        ///
        bool isInputDataAvailable();

        ///
        /// \brief Returns last read button state for requested button index.
        /// @param [in] buttonIndex Index of button which should be read.
        /// \returns True if button is pressed, false otherwise.
        ///
        bool getButtonState(uint8_t buttonIndex);

        ///
        /// \brief Calculates encoder pair number based on provided button ID.
        /// @param [in] buttonID   Button index from which encoder pair is being calculated.
        /// \returns Calculated encoder pair number.
        ///
        uint8_t getEncoderPair(uint8_t buttonID);

        ///
        /// \brief Checks state of requested encoder.
        /// @param [in] encoderID       Encoder which is being checked.
        /// \returns Pair state of the specified encoder (A and B signals stored in bits 0 and 1).
        ///
        uint8_t getEncoderPairState(uint8_t encoderID);

        ///
        /// \brief Used to turn LED connected to the board on or off.
        /// @param [in] ledID   LED for which to change state.
        /// @param [in] state   New LED state (true/on, false/off).
        ///
        void writeLEDstate(uint8_t ledID, bool state);

        ///
        /// \brief Used to calculate index of R, G or B component of RGB LED.
        /// @param [in] rgbID   Index of RGB LED.
        /// @param [in] index   R, G or B component (enumerated type, see rgbIndex_t).
        /// \returns Calculated index of R, G or B component of RGB LED.
        ///
        uint8_t getRGBaddress(uint8_t rgbID, rgbIndex_t index);

        ///
        /// \brief Calculates RGB LED index based on provided single-color LED index.
        /// @param [in] ledID   Index of single-color LED.
        /// \returns Calculated index of RGB LED.
        ///
        uint8_t getRGBID(uint8_t ledID);

        ///
        /// \brief Sets LED transition speed.
        /// @param [in] transitionSpeed Transition speed.
        ///
        void setLEDfadeSpeed(uint8_t transitionSpeed);

        ///
        /// \brief Checks if data from ADC is available.
        /// Data is read in ISR and stored into samples array.
        /// Once all inputs are read, data is considered available.
        /// \returns True if data is available, false otherwise.
        ///
        bool isAnalogDataAvailable();

        ///
        /// brief Checks for current analog value for specified analog index.
        /// @param[in] analogID     Analog index for which ADC value is being checked.
        /// \returns ADC value for requested analog index.
        ///
        uint16_t getAnalogValue(uint8_t analogID);
    }    // namespace io

    namespace NVM
    {
        //NVM: non-volatile memory

        enum class parameterType_t : uint8_t
        {
            byte,
            word,
            dword
        };

        ///
        /// \brief Returns total available bytes to store in non-volatile memory.
        ///
        uint32_t size();

        ///
        /// \brief Initializes and prepares non-volatile storage on board.
        ///
        void init();

        ///
        /// \brief Used to wipe non-volatile memory on specified range.
        /// @param [in] start   Starting address from which to erase.
        /// @param [in] end     Last address to erase.
        ///
        void clear(uint32_t start, uint32_t end);

        ///
        /// \brief Returns amount of actual memory it takes to store provided parameter type.
        ///
        size_t paramUsage(parameterType_t type);

        ///
        /// \brief Used to read contents of memory provided by specific board,
        /// @param [in] address Memory address from which to read from.
        /// @param [in] value   Pointer to variable in which read value is being stored.
        /// @param [in] type    Type of parameter which is being read.
        /// \returns            True on success, false otherwise.
        ///
        bool read(uint32_t address, int32_t& value, parameterType_t type);

        ///
        /// \brief Used to write value to memory provided by specific board.
        /// @param [in] address Memory address in which new value is being written.
        /// @param [in] value   Value to write.
        /// @param [in] type    Type of parameter which is being written.
        /// \returns            True on success, false otherwise.
        ///
        bool write(uint32_t address, int32_t value, parameterType_t type);
    }    // namespace NVM

    namespace bootloader
    {
        uint32_t pageSize(size_t index);
        void     erasePage(size_t index);
        void     fillPage(size_t index, uint32_t address, uint16_t data);
        void     writePage(size_t index);
        void     applyFw();
    }    // namespace bootloader
};       // namespace Board