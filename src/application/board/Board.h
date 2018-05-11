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
#include "common/DataTypes.h"

///
/// \brief List of all supported boards.
///
typedef enum
{
    BOARD_OPEN_DECK_ID,
    BOARD_A_LEO_ID,
    BOARD_A_MEGA_ID,
    BOARD_A_PRO_MICRO_ID,
    BOARD_A_UNO_ID,
    BOARD_T_2PP_ID,
    BOARD_KODAMA_ID
} boardID_t;

#ifdef __AVR__
#include "avr/variants/Common.h"
#endif

#ifdef BOARD_OPEN_DECK
#define BOARD_ID    BOARD_OPEN_DECK_ID
#include "avr/variants/opendeck/Hardware.h"
#include "avr/variants/opendeck/Version.h"
#elif defined(BOARD_A_LEO)
#define BOARD_ID    BOARD_A_LEO_ID
#include "avr/variants/leonardo/Hardware.h"
#include "avr/variants/leonardo/Version.h"
#elif defined(BOARD_A_PRO_MICRO)
#define BOARD_ID    BOARD_A_PRO_MICRO_ID
#include "avr/variants/leonardo/Hardware.h"
#include "avr/variants/leonardo/Version.h"
#elif defined(BOARD_KODAMA)
#define BOARD_ID    BOARD_KODAMA_ID
#include "avr/variants/kodama/Hardware.h"
#include "avr/variants/kodama/Version.h"
#elif defined(BOARD_A_MEGA)
#define BOARD_ID    BOARD_A_MEGA_ID
#include "avr/variants/mega/Hardware.h"
#include "avr/variants/mega/Version.h"
#elif defined(BOARD_A_UNO)
#define BOARD_ID    BOARD_A_UNO_ID
#include "avr/variants/uno/Hardware.h"
#include "avr/variants/uno/Version.h"
#elif defined(BOARD_T_2PP)
#define BOARD_ID    BOARD_T_2PP_ID
#include "avr/variants/teensy2pp/Hardware.h"
#include "avr/variants/teensy2pp/Version.h"
#elif defined(BOARD_A_xu2)
//no id needed
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
    static void init();

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
    /// \brief Initializes USB peripheral and configures it as MIDI device.
    ///
    static void initUSB_MIDI();

    ///
    /// \brief Initializes UART peripheral.
    /// @param[in] baudRate UART speed.
    ///
    static void initUART_MIDI(uint32_t baudRate);

    ///
    /// \brief Checks if digital input data is avaiable (encoder and button data).
    /// On boards which use input matrix data is read in ISR and stored into digitalInBuffer array.
    /// Once all columns are read, data is considered available.
    /// At this point, input matrix column variable is set to invalid value
    /// to stop further data reading from ISR until continueDigitalInReadout
    /// function is called.
    /// \returns True if data is available, false otherwise.
    ///
    static bool digitalInputDataAvailable();

    ///
    /// \brief Resets active input matrix column so that readings in ISR can continue.
    ///
    static void continueDigitalInReadout();

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
    /// @param [in] encoderID   Encoder which is being checked.
    /// \returns 0 if encoder hasn't been moved, 1 if it's moving in positive and -1 if it's
    /// moving in negative direction.
    ///
    static int8_t getEncoderState(uint8_t encoderID);

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
    /// \brief Initializes encoder values to default state.
    ///
    static void initEncoders();

    ///
    /// \brief Initializes main and PWM timers.
    ///
    static void configureTimers();

    ///
    /// \brief Checks state of requested encoder.
    /// Internal function.
    /// @param [in] encoderID   Encoder which is being checked.
    /// @param [in] pairState   A and B signal readings from encoder placed into bits 0 and 1.
    /// \returns 0 if encoder hasn't been moved, 1 if it's moving in positive and -1 if it's
    /// moving in negative direction.
    ///
    static int8_t readEncoder(uint8_t encoderID, uint8_t pairState);
};

///
/// \brief External definition of Board class instance.
///
extern Board board;

#endif