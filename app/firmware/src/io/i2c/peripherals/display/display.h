/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "element.h"
#include "strings.h"
#include "messaging/messaging.h"
#include "system/config.h"

#include <u8x8.h>

#include <bits/char_traits.h>
#include <optional>

namespace io::i2c::display
{
    /**
     * @brief OLED display peripheral that renders MIDI and preset status.
     */
    class Display : public Peripheral
    {
        public:
        /**
         * @brief Constructs a display peripheral bound to hardware and database services.
         *
         * @param hwa Hardware abstraction used to communicate with the display.
         * @param database Database interface used to read display configuration.
         */
        Display(Hwa&      hwa,
                Database& database);

        /**
         * @brief Detects, configures, and initializes the display controller.
         *
         * @return `true` if a supported display was found and initialized, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Refreshes display elements when the display is initialized.
         */
        void update() override;

        private:
        static constexpr uint8_t MAX_ROWS              = 4;
        static constexpr uint8_t MAX_COLUMNS           = 16;
        static constexpr size_t  U8X8_BUFFER_SIZE      = 32;
        static constexpr uint8_t MESSAGE_TEXT_LENGTH   = 12;
        static constexpr uint8_t PRESET_TEXT_LENGTH    = 3;
        static constexpr uint8_t INDICATOR_TEXT_LENGTH = 1;
        static constexpr uint8_t PRESET_COLUMN         = 13;

        using RowMapArray     = std::array<std::array<uint8_t, MAX_ROWS>, static_cast<uint8_t>(DisplayResolution::Count)>;
        using I2cAddressArray = std::array<uint8_t, 2>;

        static constexpr RowMapArray ROW_MAP = {
            {
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
            },
        };

        static constexpr I2cAddressArray I2C_ADDRESS = {
            0x3C,
            0x3D,
        };

        /**
         * @brief Groups the logical display elements rendered on the OLED.
         */
        class Elements
        {
            public:
            /**
             * @brief Constructs the display element collection for a display instance.
             *
             * @param display Owning display instance used for rendering.
             */
            explicit Elements(Display& display)
                : _display(display)
            {}

            /**
             * @brief Container type used to store pointers to display text controls.
             */
            using ElementsVec = std::vector<DisplayTextControl*>;

            /**
             * @brief Refresh period, in milliseconds, used when updating display elements.
             */
            static constexpr uint16_t REFRESH_TIME = 30;

            /**
             * @brief Formats MIDI events into human-readable display strings.
             */
            class MIDIUpdater
            {
                public:
                MIDIUpdater() = default;

                /**
                 * @brief Selects whether note events use alternate note-name formatting.
                 *
                 * @param state `true` to show note names, otherwise `false`.
                 */
                void use_alternate_note(bool state);

                /**
                 * @brief Updates a text element with the supplied MIDI event data.
                 *
                 * @param element Display text element to update.
                 * @param channel MIDI channel to show.
                 * @param index MIDI index or note number to show.
                 * @param value MIDI value to show.
                 * @param message MIDI message type that determines formatting.
                 */
                void update_midi_value(DisplayTextControl&         element,
                                       uint8_t                     channel,
                                       uint16_t                    index,
                                       uint16_t                    value,
                                       protocol::midi::MessageType message);

                private:
                bool _use_alternate_note = false;
            };

            /**
             * @brief Displays the most recent incoming MIDI message type.
             */
            class MessageTypeIn : public DisplayElement<MESSAGE_TEXT_LENGTH, 0, 1, true>
            {
                public:
                MessageTypeIn()
                {
                    messaging::subscribe<messaging::UmpSignal>(
                        [this](const messaging::UmpSignal& event)
                        {
                            if (event.direction != messaging::MidiDirection::In)
                            {
                                return;
                            }

                            const auto midi_message = protocol::midi::decode_message(event.packet);

                            if (midi_message.type != protocol::midi::MessageType::SysEx)
                            {
                                set_text("%s", Strings::midi_message(midi_message.type));
                            }
                        });
                }
            };

            /**
             * @brief Displays the most recent incoming MIDI message value.
             */
            class MessageValueIn : public DisplayElement<MESSAGE_TEXT_LENGTH, 1, 0, true>
            {
                public:
                explicit MessageValueIn(MIDIUpdater& midi_updater)
                    : _midi_updater(midi_updater)
                {
                    messaging::subscribe<messaging::UmpSignal>(
                        [this](const messaging::UmpSignal& event)
                        {
                            if (event.direction != messaging::MidiDirection::In)
                            {
                                return;
                            }

                            const auto midi_message = protocol::midi::decode_message(event.packet);

                            if (midi_message.type != protocol::midi::MessageType::SysEx)
                            {
                                _midi_updater.update_midi_value(*this,
                                                                midi_message.channel,
                                                                midi_message.data1,
                                                                midi_message.data2,
                                                                midi_message.type);
                            }
                        });
                }

                private:
                MIDIUpdater& _midi_updater;
            };

            /**
             * @brief Displays the most recent outgoing MIDI message type.
             */
            class MessageTypeOut : public DisplayElement<MESSAGE_TEXT_LENGTH, 2, 1, true>
            {
                public:
                MessageTypeOut()
                {
                    messaging::subscribe<messaging::MidiSignal>(
                        [this](const messaging::MidiSignal& signal)
                        {
                            switch (signal.source)
                            {
                            case messaging::MidiSource::Analog:
                            case messaging::MidiSource::Button:
                            case messaging::MidiSource::Encoder:
                                set_text("%s", Strings::midi_message(signal.message));
                                break;

                            default:
                                break;
                            }
                        });
                }
            };

            /**
             * @brief Displays the most recent outgoing MIDI message value.
             */
            class MessageValueOut : public DisplayElement<MESSAGE_TEXT_LENGTH, 3, 0, true>
            {
                public:
                explicit MessageValueOut(MIDIUpdater& midi_updater)
                    : _midi_updater(midi_updater)
                {
                    messaging::subscribe<messaging::MidiSignal>(
                        [this](const messaging::MidiSignal& signal)
                        {
                            switch (signal.source)
                            {
                            case messaging::MidiSource::Analog:
                            case messaging::MidiSource::Button:
                            case messaging::MidiSource::Encoder:
                                _midi_updater.update_midi_value(*this,
                                                                signal.channel,
                                                                signal.index,
                                                                signal.value,
                                                                signal.message);
                                break;

                            default:
                                break;
                            }
                        });
                }

                private:
                MIDIUpdater& _midi_updater;
            };

            /**
             * @brief Displays the active preset number.
             */
            class Preset : public DisplayElement<PRESET_TEXT_LENGTH, 0, PRESET_COLUMN, false>
            {
                public:
                Preset()
                {
                    messaging::subscribe<messaging::SystemSignal>(
                        [this](const messaging::SystemSignal& event)
                        {
                            if (event.system_message == messaging::SystemMessage::PresetChanged)
                            {
                                set_preset(event.value + 1);
                            }
                        });
                }

                /**
                 * @brief Updates the displayed preset number.
                 *
                 * @param preset One-based preset number to display.
                 */
                void set_preset(uint8_t preset)
                {
                    set_text("P%d", preset);
                }
            };

            /**
             * @brief Displays the incoming-MIDI activity marker.
             */
            class InMessageIndicator : public DisplayElement<INDICATOR_TEXT_LENGTH, 0, 0, false>
            {
                public:
                InMessageIndicator()
                {
                    set_text("%s", Strings::IN_EVENT_STRING);
                }
            };

            /**
             * @brief Displays the outgoing-MIDI activity marker.
             */
            class OutMessageIndicator : public DisplayElement<INDICATOR_TEXT_LENGTH, 2, 0, false>
            {
                public:
                OutMessageIndicator()
                {
                    set_text("%s", Strings::OUT_EVENT_STRING);
                }
            };

            /**
             * @brief Renders changed elements to the display when the refresh interval expires.
             */
            void update();

            /**
             * @brief Sets how long transient message elements stay visible.
             *
             * @param retention_time Retention time in milliseconds.
             */
            void set_retention_time(uint32_t retention_time);

            /**
             * @brief Selects whether note events use alternate note-name formatting.
             *
             * @param state `true` to show note names, otherwise `false`.
             */
            void use_alternate_note(bool state);

            /**
             * @brief Updates the displayed preset number.
             *
             * @param preset One-based preset number to display.
             */
            void set_preset(uint8_t preset);

            private:
            Display&            _display;
            MIDIUpdater         _midi_updater;
            MessageTypeIn       _message_type_in;
            MessageValueIn      _message_value_in = MessageValueIn(_midi_updater);
            MessageTypeOut      _message_type_out;
            MessageValueOut     _message_value_out = MessageValueOut(_midi_updater);
            Preset              _preset;
            InMessageIndicator  _in_message_indicator;
            OutMessageIndicator _out_message_indicator;
            uint32_t            _last_refresh_time      = 0;
            uint32_t            _message_retention_time = 0;
            ElementsVec         _elements               = {
                &_message_type_in,
                &_message_value_in,
                &_message_type_out,
                &_message_value_out,
                &_preset,
                &_in_message_indicator,
                &_out_message_indicator,
            };
        };

        friend class Elements;

        Hwa&              _hwa;
        Database&         _database;
        u8x8_t            _u8x8                          = {};
        Elements          _elements                      = Elements(*this);
        uint8_t           _u8x8_buffer[U8X8_BUFFER_SIZE] = {};
        size_t            _u8x8_counter                  = 0;
        DisplayResolution _resolution                    = DisplayResolution::Count;
        bool              _initialized                   = false;
        bool              _startup_info_shown            = false;
        uint8_t           _selected_i2c_address          = 0;
        size_t            _rows                          = 0;

        /**
         * @brief Initializes the selected U8x8 display backend.
         *
         * @param i2c_address I2C address of the detected display.
         * @param controller Display controller type to initialize.
         * @param resolution Display resolution to initialize.
         *
         * @return `true` if the selected backend was initialized successfully, otherwise `false`.
         */
        bool init_u8x8(uint8_t i2c_address, DisplayController controller, DisplayResolution resolution);

        /**
         * @brief Deinitializes the display runtime state.
         *
         * @return `true` if the display was initialized and is now deinitialized, otherwise `false`.
         */
        bool deinit();

        /**
         * @brief Shows the startup welcome message.
         */
        void display_welcome_message();

        /**
         * @brief Returns the centered column position for a text string.
         *
         * @param text_size Character count of the text to center.
         *
         * @return Column index that centers the text on the display.
         */
        uint8_t get_text_center(uint8_t text_size);

        /**
         * @brief Serves SysEx configuration reads for the display section.
         *
         * @param section I2C configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::I2c section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the display section.
         *
         * @param section I2C configuration section being written.
         * @param index Parameter index within the section.
         * @param value Value to store.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::I2c section, size_t index, uint16_t value);
    };
}    // namespace io::i2c::display
