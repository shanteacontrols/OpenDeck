/*

Copyright Igor Petrovic

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

#include "board/board.h"

#include "core/mcu.h"

#include <stddef.h>
#include <inttypes.h>

namespace board::detail
{
    // some boards/SDKs might require periodic calls to certain APIs:
    // enable only if needed
    using updateHook_t = void (*)();

    void registerUpdateHook(updateHook_t hook);

    namespace setup
    {
        void application();
        void bootloader();
    }    // namespace setup

    namespace usb
    {
        /// Used to indicate current state of TX (data in in USB terminology) transfers.
        enum class txState_t : uint32_t
        {
            DONE,
            SENDING,
            WAITING
        };

        void init();
        void deInit();
        void update();

        const void* cfgDescriptor(uint16_t* size);
        const void* deviceDescriptor(uint16_t* size);
        const void* languageString(uint16_t* size);
        const void* manufacturerString(uint16_t* size);
        const void* productString(uint16_t* size);
        const void* serialIDString(uint16_t* size, uint8_t* uid);
    }    // namespace usb

    namespace io
    {
        void init();

        /// MCU-specific delay routine used to generate correct SPI timings in bit-banging mode.
        void spiWait();

        namespace digital_in
        {
            // constant used to easily access maximum amount of previous readings for a given digital input
            constexpr inline size_t MAX_READING_COUNT = (8 * sizeof(((board::io::digital_in::Readings*)0)->readings));

            void init();

            /// Continuously reads all digital inputs.
            void update();

            /// Removes all readings from digital inputs.
            void flush();
        }    // namespace digital_in

        namespace digital_out
        {
            void init();

            /// Checks if digital outputs need to be updated.
            void update();
        }    // namespace digital_out

        namespace analog
        {
            /// Used to indicate that the new analog reading has been made
            constexpr inline uint16_t ADC_NEW_READING_FLAG = 0x8000;

            constexpr inline uint8_t ISR_PRIORITY = 5;

            void init();
        }    // namespace analog

        namespace indicators
        {
            void init();

            /// Checks if indicator LEDs need to be turned on or off.
            void update();

            /// Flashes integrated LEDs on board on startup to indicate that application is about to be loaded.
            void indicateApplicationLoad();

            /// Flashes integrated LEDs on board on startup to indicate that bootloader is about to be loaded.
            void indicateBootloaderLoad();
        }    // namespace indicators

        namespace unused
        {
            /// Used as an descriptor for unused pins.
            struct UnusedIo
            {
                core::mcu::io::pin_t pin   = {};
                bool                 state = false;
            };

            void init();
        }    // namespace unused

        namespace bootloader
        {
            void init();
        }    // namespace bootloader
    }    // namespace io
}    // namespace board::detail