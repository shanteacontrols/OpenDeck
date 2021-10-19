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

#include "database/Database.h"
#include "util/messaging/Messaging.h"

#ifndef LEDS_SUPPORTED
#include "stub/LEDs.h"
#else

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
            unused,
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

        class HWA
        {
            public:
            HWA() = default;

            virtual void   setState(size_t index, brightness_t brightness)                = 0;
            virtual size_t rgbIndex(size_t singleLEDindex)                                = 0;
            virtual size_t rgbSignalIndex(size_t rgbIndex, LEDs::rgbIndex_t rgbComponent) = 0;
        };

        LEDs(HWA&                     hwa,
             Database&                database,
             Util::MessageDispatcher& dispatcher);

        void         init(bool startUp = true);
        void         update(bool forceChange = false);
        void         setAllOn();
        void         setAllOff();
        void         refresh();
        void         setColor(uint8_t ledID, color_t color, brightness_t brightness);
        color_t      color(uint8_t ledID);
        void         setBlinkSpeed(uint8_t ledID, blinkSpeed_t value);
        blinkSpeed_t blinkSpeed(uint8_t ledID);
        size_t       rgbSignalIndex(size_t rgbIndex, LEDs::rgbIndex_t rgbComponent);
        size_t       rgbIndex(size_t singleLEDindex);
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
        bool         isControlTypeMatched(MIDI::messageType_t midiMessage, controlType_t controlType);
        void         midiToState(MIDI::messageType_t messageType, uint8_t value1, uint8_t value2, uint8_t channel, Util::MessageDispatcher::messageSource_t source);

        HWA&      _hwa;
        Database& _database;

        static constexpr size_t  MAX_LEDS                        = MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS;
        static constexpr size_t  TOTAL_BLINK_SPEEDS              = 4;
        static constexpr size_t  TOTAL_BRIGHTNESS_VALUES         = 4;
        static constexpr uint8_t LED_BLINK_TIMER_TYPE_CHECK_TIME = 50;

        /// Array holding current LED status for all LEDs.
        uint8_t _ledState[MAX_LEDS] = {};

        /// Array holding current LED brightness for all LEDs.
        brightness_t _brightness[MAX_LEDS] = {};

        /// Array holding time after which LEDs should blink.
        uint8_t _blinkTimer[MAX_LEDS] = {};

        /// Holds currently active LED blink type.
        blinkType_t _ledBlinkType = blinkType_t::timer;

        /// Pointer to array used to check if blinking LEDs should toggle state.
        const uint8_t* _blinkResetArrayPtr = nullptr;

        /// Array holding MIDI clock pulses after which LED state is toggled for all possible blink rates.
        const uint8_t _blinkReset_midiClock[TOTAL_BLINK_SPEEDS] = {
            48,
            24,
            12,
            255,    // no blinking
        };

        /// Array holding time indexes (multipled by 50) after which LED state is toggled for all possible blink rates.
        const uint8_t _blinkReset_timer[TOTAL_BLINK_SPEEDS] = {
            20,
            10,
            5,
            0,
        };

        /// Array used to determine when the blink state for specific blink rate should be changed.
        uint8_t _blinkCounter[TOTAL_BLINK_SPEEDS] = {};

        /// Holds blink state for each blink speed so that leds are in sync.
        bool _blinkState[TOTAL_BLINK_SPEEDS] = {};

        /// Holds last time in miliseconds when LED blinking has been updated.
        uint32_t _lastLEDblinkUpdateTime = 0;

        const MIDI::messageType_t controlTypeToMIDImessage[static_cast<uint8_t>(controlType_t::AMOUNT)] = {
            MIDI::messageType_t::noteOn,           // midiInNoteSingleVal,
            MIDI::messageType_t::noteOn,           // localNoteSingleVal,
            MIDI::messageType_t::controlChange,    // midiInCCSingleVal,
            MIDI::messageType_t::controlChange,    // localCCSingleVal,
            MIDI::messageType_t::programChange,    // midiInPCSingleVal,
            MIDI::messageType_t::programChange,    // localPCSingleVal,
            MIDI::messageType_t::noteOn,           // midiInNoteMultiVal,
            MIDI::messageType_t::noteOn,           // localNoteMultiVal,
            MIDI::messageType_t::controlChange,    // midiInCCMultiVal,
            MIDI::messageType_t::controlChange,    // localCCMultiVal,
        };
    };
}    // namespace IO

#endif