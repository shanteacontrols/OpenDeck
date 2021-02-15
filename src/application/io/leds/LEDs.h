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

#ifndef LEDS_SUPPORTED
#include "Stub.h"
#else

#include "database/Database.h"
#include "midi/src/MIDI.h"

namespace IO
{
    class LEDs
    {
        public:
        enum class rgbIndex_t : uint8_t
        {
            r,
            g,
            b
        };

        enum class color_t : uint8_t
        {
            off,
            red,
            green,
            yellow,
            blue,
            magenta,
            cyan,
            white,
            AMOUNT
        };

        enum class setting_t : uint8_t
        {
            blinkWithMIDIclock,
            fadeSpeed,
            useStartupAnimation,
            AMOUNT
        };

        enum class controlType_t : uint8_t
        {
            midiInNoteSingleVal,
            localNoteSingleVal,
            midiInCCSingleVal,
            localCCSingleVal,
            midiInPCSingleVal,
            localPCSingleVal,
            midiInNoteMultiVal,
            localNoteMultiVal,
            midiInCCMultiVal,
            localCCMultiVal,
            AMOUNT
        };

        enum class blinkSpeed_t : uint8_t
        {
            s1000ms,
            s500ms,
            s250ms,
            noBlink
        };

        enum class blinkType_t : uint8_t
        {
            timer,
            midiClock
        };

        enum class brightness_t : uint8_t
        {
            bOff,
            b25,
            b50,
            b75,
            b100
        };

        enum class dataSource_t : uint8_t
        {
            external,    //data from midi in
            internal     //data from local source (buttons, encoders...)
        };

        class HWA
        {
            public:
            HWA() = default;

            virtual void   setState(size_t index, brightness_t brightness)                         = 0;
            virtual size_t rgbSingleComponentIndex(size_t rgbIndex, LEDs::rgbIndex_t rgbComponent) = 0;
            virtual size_t rgbIndex(size_t singleLEDindex)                                         = 0;
            virtual void   setFadeSpeed(size_t transitionSpeed)                                    = 0;
        };

        LEDs(HWA& hwa, Database& database)
            : hwa(hwa)
            , database(database)
        {}

        void         init(bool startUp = true);
        void         checkBlinking(bool forceChange = false);
        void         setAllOn();
        void         setAllOff();
        void         refresh();
        void         setColor(uint8_t ledID, color_t color, brightness_t brightness);
        color_t      color(uint8_t ledID);
        void         setBlinkSpeed(uint8_t ledID, blinkSpeed_t value);
        blinkSpeed_t blinkSpeed(uint8_t ledID);
        size_t       rgbSingleComponentIndex(size_t rgbIndex, LEDs::rgbIndex_t rgbComponent);
        size_t       rgbIndex(size_t singleLEDindex);
        bool         setFadeSpeed(uint8_t transitionSpeed);
        void         midiToState(MIDI::messageType_t messageType, uint8_t data1, uint8_t data2, uint8_t channel, dataSource_t dataSource);
        void         setBlinkType(blinkType_t blinkType);
        void         resetBlinking();

        private:
        enum class ledBit_t : uint8_t
        {
            active,     ///< LED is active (either it blinks or it's constantly on), this bit is OR function between blinkOn and state
            blinkOn,    ///< LED blinks
            state,      ///< LED is in constant state
            rgb,        ///< RGB enabled
            rgb_r,      ///< R index of RGB LED
            rgb_g,      ///< G index of RGB LED
            rgb_b       ///< B index of RGB LED
        };

        void         updateBit(uint8_t index, ledBit_t bit, bool state);
        bool         bit(uint8_t index, ledBit_t bit);
        void         resetState(uint8_t index);
        color_t      valueToColor(uint8_t receivedVelocity);
        blinkSpeed_t valueToBlinkSpeed(uint8_t value);
        brightness_t valueToBrightness(uint8_t value);
        void         startUpAnimation();

        HWA&                    hwa;
        Database&               database;
        static constexpr size_t maxLEDs = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS;

        /// Array holding current LED status for all LEDs.
        uint8_t ledState[maxLEDs] = {};

        /// Array holding current LED brightness for all LEDs.
        brightness_t brightness[maxLEDs] = {};

        /// Array holding time after which LEDs should blink.
        uint8_t blinkTimer[maxLEDs] = {};

        /// Holds currently active LED blink type.
        blinkType_t ledBlinkType = blinkType_t::timer;

        /// Pointer to array used to check if blinking LEDs should toggle state.
        const uint8_t* blinkResetArrayPtr = nullptr;

        static constexpr size_t totalBlinkSpeeds      = 4;
        static constexpr size_t totalBrightnessValues = 4;

        /// Array holding MIDI clock pulses after which LED state is toggled for all possible blink rates.
        const uint8_t blinkReset_midiClock[totalBlinkSpeeds] = {
            255,    //no blinking
            12,
            24,
            48
        };

        /// Array holding time indexes (multipled by 50) after which LED state is toggled for all possible blink rates.
        const uint8_t blinkReset_timer[totalBlinkSpeeds] = {
            0,
            5,
            10,
            20,
        };

        /// Array used to determine when the blink state for specific blink rate should be changed.
        uint8_t blinkCounter[totalBlinkSpeeds] = {};

        /// Holds last time in miliseconds when LED blinking has been updated.
        uint32_t lastLEDblinkUpdateTime = 0;

        // \brief Holds blink state for each blink speed so that leds are in sync.
        bool blinkState[totalBlinkSpeeds] = {};
    };
}    // namespace IO

#endif