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
#include "interface/digital/output/leds/DataTypes.h"
#endif
#include "interface/digital/input/encoders/DataTypes.h"
#include "common/DataTypes.h"
#include "dbms/src/DataTypes.h"
#include "midi/src/DataTypes.h"
#include "board/common/uart/ODformat.h"

#ifdef __AVR__
#include "avr/variants/Common.h"
#endif

namespace Board
{
    ///
    /// \brief Perfoms initialization of MCU and all board peripherals.
    ///
    void init();

    ///
    /// \brief Checks if firmware has been updated.
    /// Firmware file has written CRC in last two flash addresses. Application stores last read CRC in EEPROM.
    /// If EEPROM and flash CRC differ, firmware has been updated.
    /// \returns True if firmware has been updated, false otherwise.
    ///
    bool checkNewRevision();

    ///
    /// \brief Flashes integrated LEDs on board on startup.
    /// Pattern differs depending on whether firmware is updated or not.
    /// @param[in] fwUpdated    If set to true, "Firmware updated" pattern will be
    ///                         used to flash the LEDs.
    ///
    void ledFlashStartup(bool fwUpdated);

    ///
    /// \brief Board-specific start-up animation function.
    /// \returns True if start-up animation is defined, false otherwise.
    ///
    bool startUpAnimation();

    ///
    /// \brief Checks if device has been successfully connected to host.
    ///
    bool isUSBconnected();

    ///
    /// \brief Deinitializes specified UART channel.
    /// @param [in] channel UART channel on MCU.
    ///
    void resetUART(uint8_t channel);

    ///
    /// \brief Initializes UART peripheral.
    /// @param [in] baudRate    UART speed (baudrate).
    /// @param [in] channel     UART channel on MCU.
    ///
    void initUART(uint32_t baudRate, uint8_t channel);

    ///
    /// \brief Used to read MIDI data from RX UART buffer.
    /// @param [in]     channel     UART channel on MCU.
    /// @param [in,out] data        Pointer to variable in which read data is being stored.
    /// \returns False if buffer is empty, true otherwise.
    ///
    bool uartRead(uint8_t channel, uint8_t &data);

    ///
    /// \brief Used to write MIDI data to UART TX buffer.
    /// @param [in] channel     UART channel on MCU.
    /// @param [in] data        Byte of data to write.
    /// \returns True on success. Since this function waits until
    /// outgoig buffer is full, result will always be success (1).
    ///
    bool uartWrite(uint8_t channel, uint8_t data);

    ///
    /// \brief Used to read MIDI data from USB interface.
    /// @param [in] USBMIDIpacket   Pointer to structure in which MIDI data is stored.
    /// \returns True if data is available, false otherwise.
    ///
    bool usbReadMIDI(USBMIDIpacket_t& USBMIDIpacket);

    ///
    /// \brief Used to write MIDI data to USB interface.
    /// @param [in] USBMIDIpacket   Pointer to structure holding MIDI data to write.
    /// \returns True if data is available, false otherwise.
    ///
    bool usbWriteMIDI(USBMIDIpacket_t& USBMIDIpacket);

    ///
    /// \brief Used to read data using custom OpenDeck format from UART interface.
    /// @param [in] channel         UART channel on MCU.
    /// @param [in] USBMIDIpacket   Pointer to structure holding MIDI data being read.
    /// @param [in] packetType      Pointer to variable in which read packet type is being stored.
    /// \returns True if data is available, false otherwise.
    ///
    bool uartReadOD(uint8_t channel, USBMIDIpacket_t& USBMIDIpacket, odPacketType_t& packetType);

    ///
    /// \brief Used to write data using custom OpenDeck format to UART interface.
    /// @param [in] channel         UART channel on MCU.
    /// @param [in] USBMIDIpacket   Pointer to structure holding MIDI data to write.
    /// @param [in] packetType      Type of OpenDeck packet to send.
    /// \returns True on success, false otherwise.
    ///
    bool uartWriteOD(uint8_t channel, USBMIDIpacket_t& USBMIDIpacket, odPacketType_t packetType);

    ///
    /// \brief Used to enable or disable UART loopback functionality.
    /// Used to pass incoming UART data to TX channel immediately.
    /// @param [in] channel UART channel on MCU.
    /// @param [in] state   New state of loopback functionality (true/enabled, false/disabled).
    ///
    void setUARTloopbackState(uint8_t channel, bool state);

    ///
    /// \brief Checks whether or not UART loopback functionality is enabled.
    /// @param [in] channel UART channel on MCU.
    ///
    bool getUARTloopbackState(uint8_t channel);

    ///
    /// \brief Checks if all data on specified UART channel has been sent.
    /// @param [in] channel UART channel on MCU.
    /// \returns True if there is no more data to transmit, false otherwise.
    ///
    bool isUARTtxEmpty(uint8_t channel);

    ///
    /// \brief Checks if digital input data is available (encoder and button data).
    /// Digital input data is read in ISR and stored into ring buffer.
    /// \returns True if data is available, false otherwise.
    ///
    bool digitalInputDataAvailable();

    ///
    /// \brief Returns last read button state for requested button index.
    /// @param [in] buttonIndex Index of button which should be read.
    /// \returns True if button is pressed, false otherwise.
    ///
    bool getButtonState(uint8_t buttonIndex);

    ///
    /// \brief Checks if data from multiplexers is available.
    /// Data is read in ISR and stored into samples array.
    /// Once all mux inputs are read, data is considered available.
    /// At this point, analogSampleCounter variable is set to invalid value
    /// to stop further data reading from ISR until continueADCreadout
    /// function is called.
    /// \returns True if data is available, false otherwise.
    ///
    bool analogDataAvailable();

    ///
    /// brief Checks for current analog value for specified analog index.
    /// @param[in] analogID     Analog index for which ADC value is being checked.
    /// \returns ADC value for requested analog index.
    ///
    int16_t getAnalogValue(uint8_t analogID);

    ///
    /// \brief Resets active pad index and starts data acquisition from pads again.
    ///
    void continueADCreadout();

    ///
    /// \brief Calculates encoder pair number based on provided button ID.
    /// @param [in] buttonID   Button index from which encoder pair is being calculated.
    /// \returns Calculated encoder pair number.
    ///
    uint8_t getEncoderPair(uint8_t buttonID);

    ///
    /// \brief Checks state of requested encoder.
    /// @param [in] encoderID       Encoder which is being checked.
    /// @param [in] pulsesPerStep   Amount of pulses per encoder step.
    /// \returns Encoder direction. See encoderPosition_t.
    ///
    encoderPosition_t getEncoderState(uint8_t encoderID, uint8_t pulsesPerStep);

    #ifdef LEDS_SUPPORTED
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
    /// \returns    True on success (transition speed is in range).
    ///             See board/common/constants/LEDs.h for range.
    ///
    bool setLEDfadeSpeed(uint8_t transitionSpeed);
    #endif

    ///
    /// \brief Performs software MCU reboot.
    ///
    void reboot(rebootType_t type);

    ///
    /// \brief Used to read contents of memory provided by specific Board::
    /// @param [in] address Memory address from which to read from.
    /// @param [in] type    Type of parameter which is being read. Defined in DBMS module.
    /// @param [in] value   Pointer to variable in which read value is being stored.
    /// \returns            True on success, false otherwise.
    ///
    bool memoryRead(uint32_t address, sectionParameterType_t type, int32_t &value);

    ///
    /// \brief Used to write value to memory provided by specific Board::
    /// @param [in] address Memory address in which new value is being written.
    /// @param [in] value   Value to write.
    /// @param [in] type    Type of parameter which is being written. Defined in DBMS module.
    /// \returns            True on success, false otherwise.
    ///
    bool memoryWrite(uint32_t address, int32_t value, sectionParameterType_t type);
};