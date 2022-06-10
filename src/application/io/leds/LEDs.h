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

#include "database/Database.h"
#include "protocol/midi/MIDI.h"
#include "io/common/Common.h"
#include "system/Config.h"
#include "io/IOBase.h"

using namespace Protocol;

#if defined(DIGITAL_OUTPUTS_SUPPORTED) || defined(TOUCHSCREEN_SUPPORTED)
#define LEDS_SUPPORTED

namespace IO
{
    class LEDs : public IO::Base
    {
        public:
        class Collection : public Common::BaseCollection<NR_OF_DIGITAL_OUTPUTS,
                                                         NR_OF_TOUCHSCREEN_COMPONENTS>
        {
            public:
            Collection() = delete;
        };

        enum
        {
            GROUP_DIGITAL_OUTPUTS,
            GROUP_TOUCHSCREEN_COMPONENTS
        };

        enum class rgbComponent_t : uint8_t
        {
            R,
            G,
            B
        };

        enum class color_t : uint8_t
        {
            OFF,
            RED,
            GREEN,
            YELLOW,
            BLUE,
            MAGENTA,
            CYAN,
            WHITE,
            AMOUNT
        };

        enum class setting_t : uint8_t
        {
            BLINK_WITH_MIDI_CLOCK,
            UNUSED,
            USE_STARTUP_ANIMATION,
            USE_MIDI_PROGRAM_OFFSET,
            AMOUNT
        };

        enum class controlType_t : uint8_t
        {
            MIDI_IN_NOTE_SINGLE_VAL,
            LOCAL_NOTE_SINGLE_VAL,
            MIDI_IN_CC_SINGLE_VAL,
            LOCAL_CC_SINGLE_VAL,
            PC_SINGLE_VAL,
            PRESET,
            MIDI_IN_NOTE_MULTI_VAL,
            LOCAL_NOTE_MULTI_VAL,
            MIDI_IN_CC_MULTI_VAL,
            LOCAL_CC_MULTI_VAL,
            AMOUNT
        };

        enum class blinkSpeed_t : uint8_t
        {
            S1000MS,
            S500MS,
            S250MS,
            NO_BLINK
        };

        enum class blinkType_t : uint8_t
        {
            TIMER,
            MIDI_CLOCK
        };

        enum class brightness_t : uint8_t
        {
            OFF,
            B25,
            B50,
            B75,
            B100
        };

        class HWA
        {
            public:
            virtual void   setState(size_t index, brightness_t brightness)                   = 0;
            virtual size_t rgbFromOutput(size_t index)                                       = 0;
            virtual size_t rgbComponentFromRGB(size_t index, LEDs::rgbComponent_t component) = 0;
        };

        LEDs(HWA&                hwa,
             Database::Instance& database);

        bool         init() override;
        void         updateSingle(size_t index, bool forceRefresh = false) override;
        void         updateAll(bool forceRefresh = false) override;
        size_t       maxComponentUpdateIndex() override;
        color_t      color(uint8_t ledID);
        void         setColor(uint8_t ledID, color_t color, brightness_t brightness);
        blinkSpeed_t blinkSpeed(uint8_t ledID);
        void         setAllOff();

        private:
        enum class ledBit_t : uint8_t
        {
            ACTIVE,      ///< LED is active (either it blinks or it's constantly on), this bit is OR function between blinkOn and state
            BLINK_ON,    ///< LED blinks
            STATE,       ///< LED is in constant state
            RGB,         ///< RGB enabled
            RGB_R,       ///< R index of RGB LED
            RGB_G,       ///< G index of RGB LED
            RGB_B        ///< B index of RGB LED
        };

        void                   setAllOn();
        void                   refresh();
        void                   setBlinkSpeed(uint8_t ledID, blinkSpeed_t state, bool updateState = true);
        void                   setBlinkType(blinkType_t blinkType);
        void                   resetBlinking();
        void                   updateBit(uint8_t index, ledBit_t bit, bool state);
        bool                   bit(uint8_t index, ledBit_t bit);
        void                   resetState(uint8_t index);
        color_t                valueToColor(uint8_t value);
        blinkSpeed_t           valueToBlinkSpeed(uint8_t value);
        brightness_t           valueToBrightness(uint8_t value);
        void                   startUpAnimation();
        bool                   isControlTypeMatched(MIDI::messageType_t midiMessage, controlType_t controlType);
        void                   midiToState(const Messaging::event_t& event, Messaging::eventType_t source);
        void                   setState(size_t index, brightness_t brightness);
        std::optional<uint8_t> sysConfigGet(System::Config::Section::leds_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(System::Config::Section::leds_t section, size_t index, uint16_t value);

        HWA&                _hwa;
        Database::Instance& _database;

        static constexpr size_t  TOTAL_BLINK_SPEEDS              = 4;
        static constexpr size_t  TOTAL_BRIGHTNESS_VALUES         = 4;
        static constexpr uint8_t LED_BLINK_TIMER_TYPE_CHECK_TIME = 50;

        /// Array holding current LED status for all LEDs.
        uint8_t _ledState[Collection::size()] = {};

        /// Array holding current LED brightness for all LEDs.
        brightness_t _brightness[Collection::size()] = {};

        /// Array holding time after which LEDs should blink.
        uint8_t _blinkTimer[Collection::size()] = {};

        /// Holds currently active LED blink type.
        blinkType_t _ledBlinkType = blinkType_t::TIMER;

        /// Pointer to array used to check if blinking LEDs should toggle state.
        const uint8_t* _blinkResetArrayPtr = nullptr;

        /// Array holding MIDI clock pulses after which LED state is toggled for all possible blink rates.
        static constexpr uint8_t BLINK_RESET_MIDI_CLOCK[TOTAL_BLINK_SPEEDS] = {
            48,
            24,
            12,
            255,    // no blinking
        };

        /// Array holding time indexes (multipled by 50) after which LED state is toggled for all possible blink rates.
        static constexpr uint8_t BLINK_RESET_TIMER[TOTAL_BLINK_SPEEDS] = {
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

        static constexpr MIDI::messageType_t CONTROL_TYPE_TO_MIDI_MESSAGE[static_cast<uint8_t>(controlType_t::AMOUNT)] = {
            MIDI::messageType_t::NOTE_ON,           // midiInNoteSingleVal,
            MIDI::messageType_t::NOTE_ON,           // localNoteSingleVal,
            MIDI::messageType_t::CONTROL_CHANGE,    // midiInCCSingleVal,
            MIDI::messageType_t::CONTROL_CHANGE,    // localCCSingleVal,
            MIDI::messageType_t::PROGRAM_CHANGE,    // pcSingleVal,
            MIDI::messageType_t::PROGRAM_CHANGE,    // preset,
            MIDI::messageType_t::NOTE_ON,           // midiInNoteMultiVal,
            MIDI::messageType_t::NOTE_ON,           // localNoteMultiVal,
            MIDI::messageType_t::CONTROL_CHANGE,    // midiInCCMultiVal,
            MIDI::messageType_t::CONTROL_CHANGE,    // localCCMultiVal,
        };
    };
}    // namespace IO

#else
#include "stub/LEDs.h"
#endif