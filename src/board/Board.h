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

#ifdef LEDS_SUPPORTED
#include "interface/digital/output/leds/LEDs.h"
#endif
#include "dbms/src/LESSDB.h"
#include "midi/src/MIDI.h"
#include "common/Common.h"

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

    ///
    /// \brief Checks if firmware has been updated.
    /// Firmware file has written CRC in last two flash addresses. Application stores last read CRC in EEPROM.
    /// If EEPROM and flash CRC differ, firmware has been updated.
    /// \returns True if firmware has been updated, false otherwise.
    ///
    bool checkNewRevision();

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
        void init(uint8_t channel, uint32_t baudRate);

        ///
        /// \brief Deinitializes specified UART channel.
        /// @param [in] channel UART channel on MCU.
        ///
        void deInit(uint8_t channel);

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

    namespace io
    {
        ///
        /// \brief Flashes integrated LEDs on board on startup.
        /// Pattern differs depending on whether firmware is updated or not.
        /// @param[in] fwUpdated    If set to true, "Firmware updated" pattern will be
        ///                         used to flash the LEDs.
        ///
        void ledFlashStartup(bool fwUpdated);

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

#ifdef LEDS_SUPPORTED
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
        uint8_t getRGBaddress(uint8_t rgbID, Interface::digital::output::LEDs::rgbIndex_t index);

        ///
        /// \brief Calculates RGB LED index based on provided single-color LED index.
        /// @param [in] ledID   Index of single-color LED.
        /// \returns Calculated index of RGB LED.
        ///
        uint8_t getRGBID(uint8_t ledID);

        ///
        /// \brief Sets LED transition speed.
        /// @param [in] transitionSpeed Transition speed.
        /// \returns    True on success (transition speed is in range).
        ///             See board/common/constants/IO.h for range.
        ///
        bool setLEDfadeSpeed(uint8_t transitionSpeed);
#endif

        ///
        /// \brief Checks if data from multiplexers is available.
        /// Data is read in ISR and stored into samples array.
        /// Once all mux inputs are read, data is considered available.
        /// At this point, analogSamplingDone variable is reset
        /// to stop further data reading from ISR until continueAnalogReadout
        /// function is called.
        /// \returns True if data is available, false otherwise.
        ///
        bool isAnalogDataAvailable();

        ///
        /// brief Checks for current analog value for specified analog index.
        /// @param[in] analogID     Analog index for which ADC value is being checked.
        /// \returns ADC value for requested analog index.
        ///
        int16_t getAnalogValue(uint8_t analogID);

        ///
        /// \brief Resets the analog sampling procedure.
        ///
        void continueAnalogReadout();
    }    // namespace io

    namespace eeprom
    {
        ///
        /// \brief Initializes and prepares non-volatile storage on board.
        ///
        void init();

        ///
        /// \brief Used to read contents of memory provided by specific board,
        /// @param [in] address Memory address from which to read from.
        /// @param [in] type    Type of parameter which is being read. Defined in DBMS module.
        /// @param [in] value   Pointer to variable in which read value is being stored.
        /// \returns            True on success, false otherwise.
        ///
        bool read(uint32_t address, LESSDB::sectionParameterType_t type, int32_t& value);

        ///
        /// \brief Used to write value to memory provided by specific board.
        /// @param [in] address Memory address in which new value is being written.
        /// @param [in] value   Value to write.
        /// @param [in] type    Type of parameter which is being written. Defined in DBMS module.
        /// \returns            True on success, false otherwise.
        ///
        bool write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type);
    }    // namespace eeprom

    namespace bootloader
    {
        ///
        /// \brief Handler called upon receiving bootloader packet.
        ///
        void packetHandler(uint32_t data) __attribute__((weak));

        void checkPackets();
        void erasePage(uint32_t address);
        void fillPage(uint32_t address, uint16_t data);
        void writePage(uint32_t address);
        void applyFw();
    }    // namespace bootloader
};       // namespace Board