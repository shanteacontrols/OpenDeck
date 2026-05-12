/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "io/touchscreen/deps.h"

#include <array>
#include <cstdio>
#include <string_view>

namespace opendeck::io::touchscreen
{
    /**
     * @brief Nextion touchscreen model implementation.
     */
    class Nextion : public Model
    {
        public:
        /**
         * @brief Constructs a Nextion model and registers it with the touchscreen controller.
         *
         * @param hwa Hardware abstraction used for UART communication with the display.
         */
        explicit Nextion(Hwa& hwa);

        /**
         * @brief Initializes the Nextion display and enables touch-coordinate reporting.
         *
         * @return `true` if the display was initialized successfully, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Deinitializes the Nextion hardware interface.
         *
         * @return `true` if the hardware deinitialized successfully, otherwise `false`.
         */
        bool deinit() override;

        /**
         * @brief Switches the display to the requested page.
         *
         * @param index Page index to show.
         *
         * @return `true` if the page-switch command was sent successfully, otherwise `false`.
         */
        bool set_screen(size_t index) override;

        /**
         * @brief Processes any pending bytes from the display and decodes one complete event.
         *
         * @param data Output storage populated when a supported event is decoded.
         *
         * @return Decoded touchscreen event type, or `TsEvent::None` when no complete supported event is available.
         */
        TsEvent update(Data& data) override;

        /**
         * @brief Draws the configured icon image for the requested on/off state.
         *
         * @param icon Icon descriptor containing coordinates, dimensions, and page resources.
         * @param state Requested icon state.
         */
        void set_icon_state(Icon& icon, bool state) override;

        /**
         * @brief Applies a logical brightness level using the Nextion dimming scale.
         *
         * @param brightness Brightness level to apply.
         *
         * @return `true` if the dimming command was sent successfully, otherwise `false`.
         */
        bool set_brightness(Brightness brightness) override;

        private:
        /**
         * @brief Identifiers for the Nextion response frames handled by this model.
         */
        enum class ResponseId : uint8_t
        {
            Switch,
            Count
        };

        /**
         * @brief Metadata describing one supported Nextion response frame.
         */
        struct ResponseDescriptor
        {
            uint8_t size        = 0;
            uint8_t response_id = 0;
        };

        static constexpr ResponseDescriptor RESPONSES[static_cast<size_t>(ResponseId::Count)] = {
            // switch
            {
                .size        = 6,
                .response_id = 0x65,
            },
        };

        static constexpr uint8_t BRIGHTNESS_MAPPING[7] = {
            10,
            25,
            50,
            75,
            80,
            90,
            100
        };

        static constexpr uint32_t INIT_DELAY_MS           = 1000;
        static constexpr uint8_t  COMMAND_TERMINATOR      = 0xFF;
        static constexpr uint8_t  COMMAND_TERMINATOR_SIZE = 3;

        Hwa&   _hwa;
        char   _command_buffer[Model::BUFFER_SIZE] = {};
        size_t _end_counter                        = 0;

        /**
         * @brief Formats and sends a command terminated with the Nextion end-of-command marker.
         *
         * @param line Plain command string with no format placeholders.
         *
         * @return `true` if the command and terminator were written successfully, otherwise `false`.
         */
        bool write_command(std::string_view line);

        /**
         * @brief Formats and sends a command terminated with the Nextion end-of-command marker.
         *
         * @param line `printf`-style command format string.
         *
         * @return `true` if the formatted command and terminator were written successfully, otherwise `false`.
         */
        template<typename... Args>
        bool write_formatted_command(std::string_view line, Args... args)
        {
            if (line.size() >= Model::BUFFER_SIZE)
            {
                return false;
            }

            std::array<char, Model::BUFFER_SIZE> format_buffer = {};

            for (size_t i = 0; i < line.size(); i++)
            {
                format_buffer[i] = line[i];
            }

            const int format_result = std::snprintf(_command_buffer, Model::BUFFER_SIZE, format_buffer.data(), args...);

            if ((format_result < 0) || (format_result >= static_cast<int>(Model::BUFFER_SIZE)))
            {
                return false;
            }

            for (int i = 0; i < format_result; i++)
            {
                if (!_hwa.write(_command_buffer[i]))
                {
                    return false;
                }
            }

            return end_command();
        }

        /**
         * @brief Appends the three-byte Nextion command terminator.
         *
         * @return `true` if all terminator bytes were written successfully, otherwise `false`.
         */
        bool end_command();

        /**
         * @brief Decodes the currently buffered Nextion response message.
         *
         * @param data Output storage populated when a supported response is recognized.
         *
         * @return Decoded touchscreen event type, or `TsEvent::None` when the buffered response is unsupported.
         */
        TsEvent response(Data& data);
    };
}    // namespace opendeck::io::touchscreen
