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

#include <u8x8.h>
#include "Element.h"
#include "core/util/Util.h"
#include "application/database/Database.h"
#include "application/messaging/Messaging.h"
#include "application/system/Config.h"
#include "application/io/i2c/I2C.h"
#include <bits/char_traits.h>
#include "Strings.h"

namespace io
{
    class Display : public io::I2C::Peripheral
    {
        public:
        enum class setting_t : uint8_t
        {
            DEVICE_INFO_MSG,
            CONTROLLER,
            RESOLUTION,
            EVENT_TIME,
            MIDI_NOTES_ALTERNATE,
            OCTAVE_NORMALIZATION,
            ENABLE,
            AMOUNT
        };

        enum class displayController_t : uint8_t
        {
            INVALID,
            SSD1306,
            AMOUNT
        };

        enum displayResolution_t : uint8_t
        {
            INVALID,
            R128X64,
            R128X32,
            AMOUNT
        };

        using Database = database::User<database::Config::Section::i2c_t>;

        Display(I2C::Peripheral::HWA& hwa,
                Database&             database);

        bool init() override;
        void update() override;

        private:
        bool                   initU8X8(uint8_t i2cAddress, displayController_t controller, displayResolution_t resolution);
        bool                   deInit();
        void                   displayWelcomeMessage();
        uint8_t                getTextCenter(uint8_t textSize);
        std::optional<uint8_t> sysConfigGet(sys::Config::Section::i2c_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(sys::Config::Section::i2c_t section, size_t index, uint16_t value);

        I2C::Peripheral::HWA& _hwa;
        Database&             _database;
        u8x8_t                _u8x8;

        static constexpr uint8_t MAX_ROWS         = 4;
        static constexpr uint8_t MAX_COLUMNS      = 16;
        static constexpr size_t  U8X8_BUFFER_SIZE = 32;

        using rowMapArray_t     = std::array<std::array<uint8_t, MAX_ROWS>, static_cast<uint8_t>(displayResolution_t::AMOUNT)>;
        using i2cAddressArray_t = std::array<uint8_t, 2>;

        static constexpr rowMapArray_t ROW_MAP = { {
            /// Array holding remapped values of LCD rows.
            /// Used to increase readability.
            /// Matched with displayResolution_t enum.

            // 128x32
            {
                0,
                2,
                3,
                4,
            },
            // 128x64
            {
                0,
                2,
                4,
                6,
            },
        } };

        static constexpr i2cAddressArray_t I2C_ADDRESS = {
            0x3C,
            0x3D
        };

        class Elements
        {
            public:
            Elements(Display& display)
                : _display(display)
            {}

            using elementsVec_t = std::vector<DisplayTextControl*>;

            void update();
            void setRetentionTime(uint32_t retentionTime);

            class MIDIUpdater
            {
                public:
                MIDIUpdater() = default;

                void useAlternateNote(bool state);
                void updateMIDIValue(DisplayTextControl& element, const messaging::event_t& event);

                private:
                bool _useAlternateNote = false;
            };

            class MessageTypeIn : public DisplayElement<12, 0, 1, true>
            {
                public:
                MessageTypeIn()
                {
                    MIDIDispatcher.listen(messaging::eventType_t::MIDI_IN,
                                          [this](const messaging::event_t& event)
                                          {
                                              if (event.message != MIDI::messageType_t::SYS_EX)
                                              {
                                                  setText("%s", Strings::MIDI_MESSAGE(event.message));
                                              }
                                          });
                }
            };

            class MessageValueIn : public DisplayElement<12, 1, 0, true>
            {
                public:
                MessageValueIn(MIDIUpdater& midiUpdater)
                    : _midiUpdater(midiUpdater)
                {
                    MIDIDispatcher.listen(messaging::eventType_t::MIDI_IN,
                                          [this](const messaging::event_t& event)
                                          {
                                              if (event.message != MIDI::messageType_t::SYS_EX)
                                              {
                                                  _midiUpdater.updateMIDIValue(*this, event);
                                              }
                                          });
                }

                private:
                MIDIUpdater& _midiUpdater;
            };

            class MessageTypeOut : public DisplayElement<12, 2, 1, true>
            {
                public:
                MessageTypeOut()
                {
                    MIDIDispatcher.listen(messaging::eventType_t::ANALOG,
                                          [this](const messaging::event_t& event)
                                          {
                                              setText("%s", Strings::MIDI_MESSAGE(event.message));
                                          });

                    MIDIDispatcher.listen(messaging::eventType_t::BUTTON,
                                          [this](const messaging::event_t& event)
                                          {
                                              setText("%s", Strings::MIDI_MESSAGE(event.message));
                                          });

                    MIDIDispatcher.listen(messaging::eventType_t::ENCODER,
                                          [this](const messaging::event_t& event)
                                          {
                                              setText("%s", Strings::MIDI_MESSAGE(event.message));
                                          });
                }
            };

            class MessageValueOut : public DisplayElement<12, 3, 0, true>
            {
                public:
                MessageValueOut(MIDIUpdater& midiUpdater)
                    : _midiUpdater(midiUpdater)
                {
                    MIDIDispatcher.listen(messaging::eventType_t::ANALOG,
                                          [this](const messaging::event_t& event)
                                          {
                                              _midiUpdater.updateMIDIValue(*this, event);
                                          });

                    MIDIDispatcher.listen(messaging::eventType_t::BUTTON,
                                          [this](const messaging::event_t& event)
                                          {
                                              _midiUpdater.updateMIDIValue(*this, event);
                                          });

                    MIDIDispatcher.listen(messaging::eventType_t::ENCODER,
                                          [this](const messaging::event_t& event)
                                          {
                                              _midiUpdater.updateMIDIValue(*this, event);
                                          });
                }

                private:
                MIDIUpdater& _midiUpdater;
            };

            class Preset : public DisplayElement<3, 0, 13, false>
            {
                public:
                Preset()
                {
                    MIDIDispatcher.listen(messaging::eventType_t::SYSTEM,
                                          [this](const messaging::event_t& event)
                                          {
                                              if (event.systemMessage == messaging::systemMessage_t::PRESET_CHANGED)
                                              {
                                                  setPreset(event.index + 1);
                                              }
                                          });
                }

                void setPreset(uint8_t preset)
                {
                    setText("P%d", preset);
                }
            };

            class InMessageIndicator : public DisplayElement<1, 0, 0, false>
            {
                public:
                InMessageIndicator()
                {
                    setText("%s", Strings::IN_EVENT_STRING);
                }
            };

            class OutMessageIndicator : public DisplayElement<1, 2, 0, false>
            {
                public:
                OutMessageIndicator()
                {
                    setText("%s", Strings::OUT_EVENT_STRING);
                }
            };

            Display&            _display;
            MIDIUpdater         _midiUpdater;
            MessageTypeIn       _messageTypeIn;
            MessageValueIn      _messageValueIn = MessageValueIn(_midiUpdater);
            MessageTypeOut      _messageTypeOut;
            MessageValueOut     _messageValueOut = MessageValueOut(_midiUpdater);
            Preset              _preset;
            InMessageIndicator  _inMessageIndicator;
            OutMessageIndicator _outMessageIndicator;

            uint32_t _messageRetentionTime = 0;
            bool     _messageDisplayedIn   = false;
            bool     _messageDisplayedOut  = false;

            elementsVec_t _elements = {
                &_messageTypeIn,
                &_messageValueIn,
                &_messageTypeOut,
                &_messageValueOut,
                &_preset,
                &_inMessageIndicator,
                &_outMessageIndicator,
            };

            static constexpr uint16_t REFRESH_TIME     = 30;
            uint32_t                  _lastRefreshTime = 0;
        };

        friend class Elements;
        Elements _elements = Elements(*this);

        uint8_t _u8x8Buffer[U8X8_BUFFER_SIZE] = {};
        size_t  _u8x8Counter                  = 0;

        /// Holds resolution of configured screen.
        displayResolution_t _resolution = displayResolution_t::AMOUNT;

        /// Holds true if display has been initialized.
        bool _initialized = false;

        bool    _startupInfoShown   = false;
        uint8_t _selectedI2Caddress = 0;
        size_t  _rows               = 0;
    };
}    // namespace io