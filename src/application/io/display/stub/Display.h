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

namespace IO
{
    class Display : public IO::Base
    {
        public:
        enum eventType_t : uint8_t
        {
            in,
            out,
        };

        enum class event_t : uint8_t
        {
            noteOff,
            noteOn,
            controlChange,
            programChange,
            afterTouchChannel,
            afterTouchPoly,
            pitchBend,
            systemExclusive,
            sysCommonTimeCodeQuarterFrame,
            sysCommonSongPosition,
            sysCommonSongSelect,
            sysCommonTuneRequest,
            sysRealTimeClock,
            sysRealTimeStart,
            sysRealTimeContinue,
            sysRealTimeStop,
            sysRealTimeActiveSensing,
            sysRealTimeSystemReset,
            mmcPlay,
            mmcStop,
            mmcPause,
            mmcRecordOn,
            mmcRecordOff,
            nrpn,
            presetChange
        };

        enum class setting_t : uint8_t
        {
            controller,
            resolution,
            MIDIeventTime,
            octaveNormalization,
            i2cAddress,
            AMOUNT
        };

        enum class feature_t : uint8_t
        {
            enable,
            welcomeMsg,
            vInfoMsg,
            MIDInotesAlternate,
            AMOUNT
        };

        Display(IO::U8X8& u8x8,
                Database& database)
        {}

        bool init() override
        {
            return false;
        }

        void update(bool forceRefresh = false) override
        {
        }

        void setAlternateNoteDisplay(bool state)
        {
        }

        void setOctaveNormalization(int8_t value)
        {
        }

        void setRetentionTime(uint32_t time)
        {
        }

        void setPreset(uint8_t preset)
        {
        }
    };
}    // namespace IO