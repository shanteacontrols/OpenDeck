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

#include "deps.h"
#include "application/database/database.h"
#include "application/protocol/midi/midi.h"
#include "application/io/common/common.h"
#include "application/system/config.h"
#include "application/io/base.h"

#include <optional>

namespace io::leds
{
    class Leds : public io::Base
    {
        public:
        Leds(Hwa&      hwa,
             Database& database);

        bool         init() override;
        void         updateSingle(size_t index, bool forceRefresh = false) override;
        void         updateAll(bool forceRefresh = false) override;
        size_t       maxComponentUpdateIndex() override;
        color_t      color(uint8_t index);
        void         setColor(uint8_t index, color_t color, brightness_t brightness);
        blinkSpeed_t blinkSpeed(uint8_t index);
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

        static constexpr size_t  TOTAL_BLINK_SPEEDS              = 4;
        static constexpr size_t  TOTAL_BRIGHTNESS_VALUES         = 4;
        static constexpr uint8_t LED_BLINK_TIMER_TYPE_CHECK_TIME = 50;

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

        static constexpr protocol::midi::messageType_t CONTROL_TYPE_TO_MIDI_MESSAGE[static_cast<uint8_t>(controlType_t::AMOUNT)] = {
            protocol::midi::messageType_t::NOTE_ON,           // MIDI_IN_NOTE_SINGLE_VAL,
            protocol::midi::messageType_t::NOTE_ON,           // LOCAL_NOTE_SINGLE_VAL,
            protocol::midi::messageType_t::CONTROL_CHANGE,    // MIDI_IN_CC_SINGLE_VAL,
            protocol::midi::messageType_t::CONTROL_CHANGE,    // LOCAL_CC_SINGLE_VAL,
            protocol::midi::messageType_t::PROGRAM_CHANGE,    // PC_SINGLE_VAL,
            protocol::midi::messageType_t::PROGRAM_CHANGE,    // PRESET,
            protocol::midi::messageType_t::NOTE_ON,           // MIDI_IN_NOTE_MULTI_VAL,
            protocol::midi::messageType_t::NOTE_ON,           // LOCAL_NOTE_MULTI_VAL,
            protocol::midi::messageType_t::CONTROL_CHANGE,    // MIDI_IN_CC_MULTI_VAL,
            protocol::midi::messageType_t::CONTROL_CHANGE,    // LOCAL_CC_MULTI_VAL,
            protocol::midi::messageType_t::INVALID,           // STATIC
        };

        Hwa&      _hwa;
        Database& _database;

        /// Array holding current LED status for all LEDs.
        uint8_t _ledState[Collection::SIZE()] = {};

        /// Array holding current LED brightness for all LEDs.
        brightness_t _brightness[Collection::SIZE()] = {};

        /// Array holding time after which LEDs should blink.
        uint8_t _blinkTimer[Collection::SIZE()] = {};

        /// Holds currently active LED blink type.
        blinkType_t _ledBlinkType = blinkType_t::TIMER;

        /// Pointer to array used to check if blinking LEDs should toggle state.
        const uint8_t* _blinkResetArrayPtr = nullptr;

        /// Array used to determine when the blink state for specific blink rate should be changed.
        uint8_t _blinkCounter[TOTAL_BLINK_SPEEDS] = {};

        /// Holds blink state for each blink speed so that leds are in sync.
        bool _blinkState[TOTAL_BLINK_SPEEDS] = {};

        /// Holds last time in miliseconds when LED blinking has been updated.
        uint32_t _lastLEDblinkUpdateTime = 0;

        void                   setAllOn();
        void                   setAllStaticOn();
        void                   refresh();
        void                   setBlinkSpeed(uint8_t index, blinkSpeed_t state, bool updateState = true);
        void                   setBlinkType(blinkType_t blinkType);
        void                   resetBlinking();
        void                   updateBit(uint8_t index, ledBit_t bit, bool state);
        bool                   bit(uint8_t index, ledBit_t bit);
        void                   resetState(uint8_t index);
        color_t                valueToColor(uint8_t value);
        blinkSpeed_t           valueToBlinkSpeed(uint8_t value);
        brightness_t           valueToBrightness(uint8_t value);
        void                   startUpAnimation();
        bool                   isControlTypeMatched(protocol::midi::messageType_t midiMessage, controlType_t controlType);
        void                   midiToState(const messaging::Event& event, messaging::eventType_t source);
        void                   setState(size_t index, brightness_t brightness);
        std::optional<uint8_t> sysConfigGet(sys::Config::Section::leds_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(sys::Config::Section::leds_t section, size_t index, uint16_t value);
    };
}    // namespace io::leds