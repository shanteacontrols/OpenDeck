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

#ifndef _BOARD_
#define _BOARD_

#include "interface/digital/output/leds/DataTypes.h"
#include "interface/digital/input/encoders/DataTypes.h"
#include "common/DataTypes.h"
#include "dbms/src/DataTypes.h"
#include "midi/src/DataTypes.h"

#ifdef __AVR__
#include "avr/variants/Common.h"
#endif

class Board
{
    public:
    ///
    /// \brief Default constructor.
    ///
    Board() {}

    ///
    /// \brief Perfoms initialization of MCU and all board peripherals.
    ///
    void init();

    ///
    /// \brief Used to initialize board specific data, peripherals or anything else.
    ///
    void initCustom();

    ///
    /// \brief Checks if firmware has been updated.
    /// Firmware file has written CRC in last two flash addresses. Application stores last read CRC in EEPROM.
    /// If EEPROM and flash CRC differ, firmware has been updated.
    /// \returns True if firmware has been updated, false otherwise.
    ///
    static bool checkNewRevision();

    ///
    /// \brief Flashes integrated LEDs on board on startup.
    /// Pattern differs depending on whether firmware is updated or not.
    /// @param[in] fwUpdated    If set to true, "Firmware updated" pattern will be
    ///                         used to flash the LEDs.
    ///
    static void ledFlashStartup(bool fwUpdated);

    ///
    /// \brief Pointer to board-specific start-up animation function.
    /// Disabled by default.
    ///
    void (*startUpAnimation)() = NULL;

    ///
    /// \brief Initializes USB peripheral and configures it as MIDI device.
    ///
    static void initMIDI_USB();

    ///
    /// \brief Checks if device has been successfully connected to host.
    ///
    static bool isUSBconnected();

    ///
    /// \brief Deinitializes specified UART channel.
    /// @param [in] channel UART channel on MCU.
    ///
    static void resetUART(uint8_t channel);

    ///
    /// \brief Initializes UART peripheral.
    /// @param [in] baudRate    UART speed (baudrate).
    /// @param [in] channel     UART channel on MCU.
    ///
    static void initUART(uint32_t baudRate, uint8_t channel);

    ///
    /// \brief Used to read MIDI data from RX UART buffer.
    /// @param [in] channel     UART channel on MCU.
    /// \returns Single byte on success, -1 is buffer is empty.
    ///
    static int16_t uartRead(uint8_t channel);

    ///
    /// \brief Used to write MIDI data to UART TX buffer.
    /// This function calls uartTransmitStart() internally so there is no
    /// need to call externally.
    /// @param [in] channel     UART channel on MCU.
    /// @param [in] data        Byte of data to write.
    /// \returns Positive value on success. Since this function waits until
    /// outgoig buffer is full, result will always be success (1).
    ///
    static int8_t uartWrite(uint8_t channel, uint8_t data);

    ///
    /// \brief Starts the process of transmitting the data from UART TX buffer to UART interface.
    /// @param [in] channel     UART channel on MCU.
    ///
    static void uartTransmitStart(uint8_t channel);

    ///
    /// \brief Used to read MIDI data from USB interface.
    /// @param [in] USBMIDIpacket   Pointer to structure in which MIDI data is stored.
    /// \returns True if data is available, false otherwise.
    ///
    static bool usbReadMIDI(USBMIDIpacket_t& USBMIDIpacket);

    ///
    /// \brief Used to write MIDI data to USB interface.
    /// @param [in] USBMIDIpacket   Pointer to structure holding MIDI data to write.
    /// \returns True if data is available, false otherwise.
    ///
    static bool usbWriteMIDI(USBMIDIpacket_t& USBMIDIpacket);

    ///
    /// \brief Used to enable or disable UART loopback functionality.
    /// Used to pass incoming UART data to TX channel immediately.
    /// @param [in] channel UART channel on MCU.
    /// @param [in] state   New state of loopback functionality (true/enabled, false/disabled).
    ///
    static void setUARTloopbackState(uint8_t channel, bool state);

    ///
    /// \brief Checks whether or not UART loopback functionality is enabled.
    /// @param [in] channel UART channel on MCU.
    ///
    static bool getUARTloopbackState(uint8_t channel);

    ///
    /// \brief Checks if digital input data is available (encoder and button data).
    /// Digital input data is read in ISR and stored into ring buffer.
    /// \returns True if data is available, false otherwise.
    ///
    static bool digitalInputDataAvailable();

    ///
    /// \brief Returns last read button state for requested button index.
    /// @param [in] buttonIndex Index of button which should be read.
    /// \returns True if button is pressed, false otherwise.
    ///
    static bool getButtonState(uint8_t buttonIndex);

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
    static int16_t getAnalogValue(uint8_t analogID);

    ///
    /// \brief Resets active pad index and starts data acquisition from pads again.
    ///
    static void continueADCreadout();

    ///
    /// brief Scales specified ADC value from minimum of 0 to maximum value specified.
    /// @param[in] value    ADC value which is being scaled.
    /// @param[in] maxValue Maximum value to which ADC value should be scaled.
    ///
    static uint16_t scaleADC(uint16_t value, uint16_t maxValue);

    ///
    /// brief Checks if specified hysteresis is active for requested analog index.
    /// @param[in] type     Hysteresis type. Enumerated type (see hysteresisType_t).
    /// @param[in] analogID Analog index for which hysteresis state is being checked.
    /// \returns True if hysteresis is currently active, false otherwise.
    ///
    static bool isHysteresisActive(hysteresisType_t type, uint8_t analogID);

    ///
    /// brief Enables or disables specific hysteresis type for specified analog index.
    /// @param[in] type     Hysteresis type. Enumerated type (see hysteresisType_t).
    /// @param[in] analogID Analog index for which hysteresis state is being changed.
    /// @param[in] state    New hystersis state (true/enabled, false/disabled).
    ///
    static void updateHysteresisState(hysteresisType_t type, uint8_t analogID, bool state);

    ///
    /// \brief Calculates encoder pair number based on provided button ID.
    /// @param [in] buttonID   Button index from which encoder pair is being calculated.
    /// \returns Calculated encoder pair number.
    ///
    static uint8_t getEncoderPair(uint8_t buttonID);

    ///
    /// \brief Checks state of requested encoder.
    /// @param [in] encoderID       Encoder which is being checked.
    /// @param [in] pulsesPerStep   Amount of pulses per encoder step.
    /// \returns Encoder direction. See encoderPosition_t.
    ///
    static encoderPosition_t getEncoderState(uint8_t encoderID, uint8_t pulsesPerStep);

    ///
    /// \brief Used to calculate index of R, G or B component of RGB LED.
    /// @param [in] rgbID   Index of RGB LED.
    /// @param [in] index   R, G or B component (enumerated type, see rgbIndex_t).
    /// \returns Calculated index of R, G or B component of RGB LED.
    ///
    static uint8_t getRGBaddress(uint8_t rgbID, rgbIndex_t index);

    ///
    /// \brief Calculates RGB LED index based on provided single-color LED index.
    /// @param [in] ledID   Index of single-color LED.
    /// \returns Calculated index of RGB LED.
    ///
    static uint8_t getRGBID(uint8_t ledID);

    ///
    /// \brief Performs software MCU reboot.
    ///
    void reboot(rebootType_t type);

    ///
    /// \brief Used to read contents of memory provided by specific board.
    /// @param [in] address Memory address from which to read from.
    /// @param [in] type    Type of parameter which is being read. Defined in DBMS module.
    /// @param [in] value   Pointer to variable in which read value is being stored.
    /// \returns            True on success, false otherwise.
    ///
    static bool memoryRead(uint32_t address, sectionParameterType_t type, int32_t &value);

    ///
    /// \brief Used to write value to memory provided by specific board.
    /// @param [in] address Memory address in which new value is being written.
    /// @param [in] value   Value to write.
    /// @param [in] type    Type of parameter which is being written. Defined in DBMS module.
    /// \returns            True on success, false otherwise.
    ///
    static bool memoryWrite(uint32_t address, int32_t value, sectionParameterType_t type);

    private:
    ///
    /// \brief Initializes all pins to correct states.
    ///
    static void initPins();

    ///
    /// \brief Initializes analog variables and ADC peripheral.
    ///
    static void initAnalog();

    ///
    /// \brief Initializes main and PWM timers.
    ///
    static void configureTimers();

    ///
    /// \brief Checks state of requested encoder.
    /// Internal function.
    /// @param [in] encoderID       Encoder which is being checked.
    /// @param [in] pairState       A and B signal readings from encoder placed into bits 0 and 1.
    /// @param [in] pulsesPerStep   Amount of pulses per encoder step.
    /// \returns Encoder direction. See encoderPosition_t.
    ///
    static encoderPosition_t readEncoder(uint8_t encoderID, uint8_t pairState, uint8_t pulsesPerStep);
};

///
/// \brief External definition of Board class instance.
///
extern Board board;

#endif