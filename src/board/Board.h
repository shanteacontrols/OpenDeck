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

#pragma once

#include <stddef.h>
#include <inttypes.h>
#include <array>

namespace Board
{
    enum class initStatus_t : uint8_t
    {
        OK,
        ERROR,
        ALREADY_INIT
    };

    /// Perfoms initialization of MCU and all board peripherals.
    void init();

    /// Performs software MCU reboot.
    void reboot();

    /// Used to continuously perform board-specific tasks if needed.
    void update();

    namespace USB
    {
        using midiPacket_t = std::array<uint8_t, 4>;

        /// Initializes USB interface.
        /// Note: explicit call needed only for application.
        /// For bootloader, USB is initialized automatically once runBootloader is called.
        initStatus_t init();

        /// Checks if USB has been enumerated on host machine.
        bool isUSBconnected();

        /// Used to read MIDI data from USB interface.
        /// param [in]: packet   Reference to structure in which read data will be stored if available.
        /// returns: True if data is available, false otherwise.
        bool readMIDI(midiPacket_t& packet);

        /// Used to write MIDI data to USB interface.
        /// param [in]: packet   Reference to structure holding data to write.
        /// returns: True if transfer has succeded, false otherwise.
        bool writeMIDI(midiPacket_t& packet);

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
        /// Initializes UART peripheral.
        /// param [in]: channel     UART channel on MCU.
        /// param [in]: baudRate    Baud rate for specified channel.
        /// param [in]: force       Forces the enabling of UART channel
        ///                         even if UART is already enabled.
        /// returns: see initStatus_t.
        initStatus_t init(uint8_t channel, uint32_t baudRate, bool force = false);

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
    }    // namespace UART

    namespace I2C
    {
        enum class clockSpeed_t : uint32_t
        {
            S100K = 100000,
            S400K = 400000
        };

        /// Initializes I2C peripheral on the MCU.
        /// param [in]: channel     I2C interface channel on MCU.
        /// param [in]: clockSpeed  I2C interface speed.
        /// returns: see initStatus_t.
        initStatus_t init(uint8_t channel, clockSpeed_t speed);

        /// Checks if specified I2C channel is initialized.
        /// param [in]: channel I2C channel on MCU.
        bool isInitialized(uint8_t channel);

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

        /// Verifies if device with specified address is present on the bus.
        /// param [in]: channel     I2C interface channel on MCU.
        /// param [in]: address     7-bit slave address without R/W bit.
        /// returns: True if device is present, false otherwise.
        bool deviceAvailable(uint8_t channel, uint8_t address);
    }    // namespace I2C

    namespace IO
    {
        void init();

        namespace digitalIn
        {
            enum class encoderComponent_t : uint8_t
            {
                A,
                B
            };

            /// Structure containing digital input readings for a given input.
            /// Count represents total amount of readings stored in readings variable.
            /// Readings variable contains up to 16 last readings where LSB bit is the
            /// newest reading, and MSB bit is the last.
            struct readings_t
            {
                uint8_t  count;
                uint16_t readings;
            };

            /// Returns last read digital input states for requested digital input index.
            /// param [in]:     index           Index of digital input which should be read.
            /// param [in,out]: readings        Reference to variable in which new digital input readings are stored.
            /// returns: True if there are new readings for specified digital input index.
            bool state(size_t index, readings_t& readings);

            /// Calculates encoder index based on provided digital input index.
            /// param [in]: index   Digital input index from which encoder is being calculated.
            /// returns: Calculated encoder index.
            size_t encoderFromInput(size_t index);

            /// Used to calculate index of A or B component index of encoder.
            /// param [in]: index       Encoder which is being checked.
            /// param [in]: component   A or B encoder component (enumerated type, see encoderComponent_t).
            /// returns: Calculated index of A or B signal of encoder.
            size_t encoderComponentFromEncoder(size_t index, encoderComponent_t component);
        }    // namespace digitalIn

        namespace digitalOut
        {
            enum class rgbComponent_t : uint8_t
            {
                R,
                G,
                B
            };

            enum class ledBrightness_t : uint8_t
            {
                OFF  = 0,
                B25  = 1,
                B50  = 2,
                B75  = 3,
                B100 = 4
            };

            /// Used to turn LED connected to the board on or off.
            /// param [in]: index           LED for which to change state.
            /// param [in]: brightnessLevel See ledBrightness_t enum.
            void writeLEDstate(size_t index, ledBrightness_t ledBrightness);

            /// Calculates RGB LED index based on provided single-color LED index.
            /// param [in]: index   Index of single-color LED.
            /// returns: Calculated index of RGB LED.
            size_t rgbFromOutput(size_t index);

            /// Used to calculate index of R, G or B component of RGB LED.
            /// param [in]: index       Index of RGB LED.
            /// param [in]: component   R, G or B component (enumerated type, see rgbComponent_t).
            /// returns: Calculated index of R, G or B component of RGB LED.
            size_t rgbComponentFromRGB(size_t index, rgbComponent_t component);
        }    // namespace digitalOut

        namespace analog
        {
            /// brief Checks for current analog value for specified analog index.
            /// @param[in] index        Analog index for which ADC value is being checked.
            /// param [in,out]:         Reference to variable in which new ADC reading is stored.
            /// returns: True if there is a new reading for specified analog index.
            bool value(size_t index, uint16_t& value);
        }    // namespace analog

        namespace indicators
        {
            enum class source_t : uint8_t
            {
                USB,
                UART,
                BLE
            };

            enum class direction_t : uint8_t
            {
                INCOMING,
                OUTGOING
            };

            /// Time in milliseconds during which LED indicators are on when indicateTraffic() is called.
            constexpr inline uint32_t LED_TRAFFIC_INDICATOR_TIMEOUT = 50;

            /// Blinking time in milliseconds for indicator LEDs to indicate that the firmware update is in progress.
            /// Blinking starts once indicateFirmwareUpdateStart() is called.
            constexpr inline uint32_t LED_DFU_INDICATOR_TIMEOUT = 500;

            /// Used to indicate that the data event (UART, USB etc.) has occured using built-in LEDs on board.
            /// param [source]      Source of data. Depending on the source corresponding LEDs will be turned on.
            /// param [direction]   Direction of data.
            void indicateTraffic(source_t source, direction_t direction);

            /// Used to indicate that the firmware update is in progress.
            /// This will blink all the internal LEDs continuously until update is done.
            void indicateFirmwareUpdateStart();
        }    // namespace indicators
    }        // namespace IO

    namespace NVM
    {
        // NVM: non-volatile memory

        enum class parameterType_t : uint8_t
        {
            BYTE,
            WORD,
            DWORD
        };

        /// Initializes and prepares non-volatile storage on board.
        bool init();

        /// Returns total available bytes to store in non-volatile memory.
        uint32_t size();

        /// Used to wipe non-volatile memory on specified range.
        /// param [in]: start   Starting address from which to erase.
        /// param [in]: end     Last address to erase.
        bool clear(uint32_t start, uint32_t end);

        /// Used to read contents of memory provided by specific board,
        /// param [in]: address Memory address from which to read from.
        /// param [in]: value   Reference to variable in which read value is being stored.
        /// param [in]: type    Type of parameter which is being read.
        /// returns: True on success, false otherwise.
        bool read(uint32_t address, uint32_t& value, parameterType_t type);

        /// Used to write value to memory provided by specific board.
        /// param [in]: address Memory address in which new value is being written.
        /// param [in]: value   Value to write.
        /// param [in]: type    Type of parameter which is being written.
        /// returns: True on success, false otherwise.
        bool write(uint32_t address, uint32_t value, parameterType_t type);
    }    // namespace NVM

    namespace BLE
    {
        /// Initializes and prepares BLE stack on board as well as all registered services.
        // Advertising will be started as well.
        bool init();

        /// Deinitializes BLE stack, stops advertising and terminates all active BLE connections.
        bool deInit();

        /// Checks if the device is connected to a central.
        bool isConnected();

        namespace MIDI
        {
            /// Used to read MIDI data from BLE interface.
            /// param [in]: buffer  Pointer to array in which read data will be stored if available.
            /// param [in]: size    Reference to variable in which amount of read bytes will be stored.
            /// param [in]: maxSize Maximum amount of bytes which can be stored in provided buffer.
            /// returns: True if data is available, false otherwise.
            bool read(uint8_t* buffer, size_t& size, const size_t maxSize);

            /// Used to write MIDI data to BLE interface.
            /// param [in]: buffer  Pointer to array holding data to send.
            /// param [in]: size    Amount of bytes in provided buffer.
            /// returns: True if transfer has succeded, false otherwise.
            bool write(uint8_t* buffer, size_t size);
        }    // namespace MIDI
    }        // namespace BLE

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
        void     fillPage(size_t index, uint32_t address, uint32_t value);
        void     commitPage(size_t index);
#ifdef FW_BOOT
        // don't allow this API from application
        uint8_t readFlash(uint32_t address);
#endif
    }    // namespace bootloader
};       // namespace Board