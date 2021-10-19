/*

Copyright 2015-2021 Igor Petrovic

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

#include <stddef.h>
#include <inttypes.h>
#include <array>
#include "midi/src/MIDI.h"

namespace Board
{
    using uniqueID_t = std::array<uint8_t, UID_BITS / 8>;

    /// Retrieves MCU-specific unique ID number.
    /// param [in]: uid Reference to array in which retrieved UID will be stored.
    void uniqueID(uniqueID_t& uid);

    /// Perfoms initialization of MCU and all board peripherals.
    void init();

    /// Performs software MCU reboot.
    void reboot();

    namespace USB
    {
        /// Checks if USB has been enumerated on host machine.
        bool isUSBconnected();

        /// Used to read MIDI data from USB interface.
        /// param [in]: USBMIDIpacket   Reference to structure in which read data will be stored if available.
        /// returns: True if data is available, false otherwise.
        bool readMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket);

        /// Used to write MIDI data to USB interface.
        /// param [in]: USBMIDIpacket   Reference to structure holding data to write.
        /// returns: True if transfer has succeded, false otherwise.
        bool writeMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket);

        /// Used to read CDC data from USB interface.
        /// param [in]: buffer  Pointer to array in which read data will be stored if available.
        /// param [in]: size    Reference to variable in which amount of read bytes will be stored.
        /// param [in]: maxSize Maximum amount of bytes which can be stored in provided buffer.
        /// returns: True if data is available, false otherwise.
        bool readCDC(uint8_t* buffer, size_t& size, const size_t maxSize);

        /// Used to read single CDC value from USB interface.
        /// param [in]: value  Reference to variable in which read data will be stored if available.
        /// returns: True if data is available, false otherwise.
        bool readCDC(uint8_t& value);

        /// Used to write CDC data to USB interface.
        /// param [in]: buffer  Pointer to array holding data to send.
        /// param [in]: size    Amount of bytes in provided buffer.
        /// returns: True if transfer has succeded, false otherwise.
        bool writeCDC(uint8_t* buffer, size_t size);

        /// Used to write single CDC value to USB interface.
        /// param [in]: value   Value being sent.
        /// returns: True if transfer has succeded, false otherwise.
        bool writeCDC(uint8_t value);

        /// Function called once set line encoding request has been received.
        /// Should be overriden by user application for proper functionality.
        /// param [in]: baudRate    Baudrate value specified in USB request.
        void onCDCsetLineEncoding(uint32_t baudRate);
    }    // namespace USB

    namespace UART
    {
        enum class initStatus_t : uint8_t
        {
            ok,
            error,
            alreadyInit
        };

        enum parity_t : uint8_t
        {
            no,
            even,
            odd
        };

        enum stopBits_t : uint8_t
        {
            one,
            two
        };

        enum type_t : uint8_t
        {
            rxTx,
            rx,
            tx
        };

        struct config_t
        {
            uint32_t   baudRate = 9600;
            parity_t   parity   = parity_t::no;
            stopBits_t stopBits = stopBits_t::one;
            type_t     type     = type_t::rxTx;
            bool       dmxMode  = false;

            config_t(uint32_t   baudRate,
                     parity_t   parity,
                     stopBits_t stopBits,
                     type_t     type,
                     bool       dmxMode)
                : baudRate(baudRate)
                , parity(parity)
                , stopBits(stopBits)
                , type(type)
                , dmxMode(dmxMode)
            {}

            config_t(uint32_t   baudRate,
                     parity_t   parity,
                     stopBits_t stopBits,
                     type_t     type)
                : baudRate(baudRate)
                , parity(parity)
                , stopBits(stopBits)
                , type(type)
            {}

            config_t() = default;
        };

        /// Initializes UART peripheral.
        /// param [in]: channel     UART channel on MCU.
        /// param [in]: config      Structure containing configuration for given UART channel.
        /// param [in]: force       Forces the enabling of UART channel
        ///                         even if UART is already enabled.
        /// returns: see initStatus_t.
        initStatus_t init(uint8_t channel, config_t& config, bool force = false);

        /// Deinitializes specified UART channel.
        /// param [in]: channel UART channel on MCU.
        bool deInit(uint8_t channel);

        /// Checks if specified UART channel is initialized.
        /// param [in]: channel UART channel on MCU.
        bool isInitialized(uint8_t channel);

        /// Used to read data from RX UART buffer.
        /// param [in]: channel     UART channel on MCU.
        /// param [in]: buffer      Pointer to array in which read data will be stored if available.
        /// param [in]: size        Reference to variable in which amount of read bytes will be stored.
        /// param [in]: maxSize     Maximum amount of bytes which can be stored in provided buffer.
        /// returns: True if data is available, false otherwise.
        bool read(uint8_t channel, uint8_t* buffer, size_t& size, const size_t maxSize);

        /// Used to read single value from RX UART buffer.
        /// param [in]: channel     UART channel on MCU.
        /// param [in]: value       Reference to variable in which read data will be stored if available.
        /// returns: True if data is available, false otherwise.
        bool read(uint8_t channel, uint8_t& value);

        /// Used to write data to UART TX buffer.
        /// param [in]: channel     UART channel on MCU.
        /// param [in]: buffer      Pointer to array holding data to send.
        /// param [in]: size        Amount of bytes in provided buffer.
        /// returns: True on success. Since this function waits until
        /// outgoig buffer is full, result will always be success (1).
        bool write(uint8_t channel, uint8_t* buffer, size_t size);

        /// Used to write single value to UART TX buffer.
        /// param [in]: channel     UART channel on MCU.
        /// param [in]: value       Value being sent.
        /// returns: True on success. Since this function waits until
        /// outgoig buffer is full, result will always be success (1).
        bool write(uint8_t channel, uint8_t value);

        /// Used to enable or disable UART loopback functionality.
        /// Used to pass incoming UART data to TX channel immediately.
        /// param [in]: channel UART channel on MCU.
        /// param [in]: state   New state of loopback functionality (true/enabled, false/disabled).
        void setLoopbackState(uint8_t channel, bool state);

        /// Checks if all data on specified UART channel has been sent.
        /// param [in]: channel UART channel on MCU.
        /// returns: True if there is no more data to transmit, false otherwise.
        bool isTxEmpty(uint8_t channel);

        /// Used to set value for a particular DMX channel.
        /// Note: DMX channels start from 1.
        /// param [in]: channel Channel for which to set new value.
        /// param [in]: value   New value for the specified channel.
        void setDMXChannelValue(uint16_t channel, uint8_t value);
    }    // namespace UART

    namespace I2C
    {
        enum class clockSpeed_t : uint32_t
        {
            _1kHz = 100000
        };

        /// Initializes I2C peripheral on the MCU.
        /// param [in]: channel     I2C interface channel on MCU.
        /// param [in]: clockSpeed  I2C interface speed.
        /// returns: True on success, false otherwise.
        bool init(uint8_t channel, clockSpeed_t clockSpeed);

        /// Denitializes I2C peripheral on the MCU.
        /// param [in]: channel     I2C interface channel on MCU.
        /// returns: True on success, false otherwise.
        bool deInit(uint8_t channel);

        /// Write data to I2C slave on specified address.
        /// param [in]: channel     I2C interface channel on MCU.
        /// param [in]: address     7-bit slave address without R/W bit.
        /// param [in]: buffer      Pointer to array holding data to send.
        /// param [in]: size        Amount of bytes in provided buffer.
        /// returns: True on success, false otherwise.
        bool write(uint8_t channel, uint8_t address, uint8_t* buffer, size_t size);
    }    // namespace I2C

    namespace io
    {
        enum class rgbIndex_t : uint8_t
        {
            r,
            g,
            b
        };

        enum class ledBrightness_t : uint8_t
        {
            bOff = 0,
            b25  = 1,
            b50  = 2,
            b75  = 3,
            b100 = 4
        };

        enum class encoderIndex_t : uint8_t
        {
            a,
            b
        };

        /// Structure containing digital input readings for a given input.
        /// Count represents total amount of readings stored in readings variable.
        /// Readings variable contains up to last 32 readings where LSB bit is the
        /// newest reading, and MSB bit is the last.
        typedef struct
        {
            uint8_t  count;
            uint32_t readings;
        } dInReadings_t;

        enum class dataSource_t : uint8_t
        {
            usb,
            uart
        };

        enum class dataDirection_t : uint8_t
        {
            incoming,
            outgoing
        };

        /// Returns last read digital input states for requested digital input index.
        /// param [in]:     digitalInIndex  Index of digital input which should be read.
        /// param [in,out]: dInReadings     Reference to variable in which new digital input readings are stored.
        /// returns: True if there are new readings for specified digital input index.
        bool digitalInState(size_t digitalInIndex, dInReadings_t& dInReadings);

        /// Calculates encoder index based on provided button index.
        /// param [in]: buttonID   Button index from which encoder is being calculated.
        /// returns: Calculated encoder index.
        size_t encoderIndex(size_t buttonID);

        /// Used to calculate index of A or B signal of encoder.
        /// param [in]: encoderID   Encoder which is being checked.
        /// param [in]: index       A or B signal (enumerated type, see encoderIndex_t).
        /// returns: Calculated index of A or B signal of encoder.
        size_t encoderSignalIndex(size_t encoderID, encoderIndex_t index);

        /// Used to turn LED connected to the board on or off.
        /// param [in]: ledID           LED for which to change state.
        /// param [in]: brightnessLevel See ledBrightness_t enum.
        void writeLEDstate(size_t ledID, ledBrightness_t ledBrightness);

        /// Calculates RGB LED index based on provided single-color LED index.
        /// param [in]: ledID   Index of single-color LED.
        /// returns: Calculated index of RGB LED.
        size_t rgbIndex(size_t ledID);

        /// Used to calculate index of R, G or B component of RGB LED.
        /// param [in]: rgbID   Index of RGB LED.
        /// param [in]: index   R, G or B component (enumerated type, see rgbIndex_t).
        /// returns: Calculated index of R, G or B component of RGB LED.
        size_t rgbSignalIndex(size_t rgbID, rgbIndex_t index);

        /// brief Checks for current analog value for specified analog index.
        /// @param[in] analogID     Analog index for which ADC value is being checked.
        /// param [in,out]:         Reference to variable in which new ADC reading is stored.
        /// returns: True if there is a new reading for specified analog index.
        bool analogValue(size_t analogID, uint16_t& value);

        /// Used to indicate that the data event (DIN MIDI, USB MIDI, CDC etc.) has occured using built-in LEDs on board.
        /// param [source]     Source of data. Depending on the source (USB/UART, corresponding LEDs will be turned on).
        /// param [direction]  Direction of data.
        void indicateTraffic(dataSource_t source, dataDirection_t direction);
    }    // namespace io

    namespace NVM
    {
        // NVM: non-volatile memory

        enum class parameterType_t : uint8_t
        {
            byte,
            word,
            dword
        };

        /// Initializes and prepares non-volatile storage on board.
        bool init();

        /// Returns total available bytes to store in non-volatile memory.
        uint32_t size();

        /// Used to wipe non-volatile memory on specified range.
        /// param [in]: start   Starting address from which to erase.
        /// param [in]: end     Last address to erase.
        bool clear(uint32_t start, uint32_t end);

        /// Returns amount of actual memory it takes to store provided parameter type.
        size_t paramUsage(parameterType_t type);

        /// Used to read contents of memory provided by specific board,
        /// param [in]: address Memory address from which to read from.
        /// param [in]: value   Reference to variable in which read value is being stored.
        /// param [in]: type    Type of parameter which is being read.
        /// returns: True on success, false otherwise.
        bool read(uint32_t address, int32_t& value, parameterType_t type);

        /// Used to write value to memory provided by specific board.
        /// param [in]: address Memory address in which new value is being written.
        /// param [in]: value   Value to write.
        /// param [in]: type    Type of parameter which is being written.
        /// returns: True on success, false otherwise.
        bool write(uint32_t address, int32_t value, parameterType_t type);
    }    // namespace NVM

    namespace bootloader
    {
        uint8_t  magicBootValue();
        void     setMagicBootValue(uint8_t value);
        void     runBootloader();
        void     runApplication();
        void     appAddrBoundary(uint32_t& first, uint32_t& last);
        bool     isHWtriggerActive();
        uint32_t pageSize(size_t index);
        void     erasePage(size_t index);
        void     fillPage(size_t index, uint32_t address, uint16_t value);
        void     writePage(size_t index);
#ifdef FW_BOOT
        // don't allow this API from application
        uint8_t readFlash(uint32_t address);
#endif
    }    // namespace bootloader
};       // namespace Board