/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/misc/bit.h"

#include <zephyr/kernel.h>

#include <cstdio>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>

namespace opendeck::io::i2c::display
{
    /**
     * @brief Common interface implemented by display text elements.
     */
    class DisplayTextControl
    {
        public:
        virtual ~DisplayTextControl() = default;

        /**
         * @brief Returns the maximum text length supported by the element.
         *
         * @return Maximum text length in characters.
         */
        virtual uint8_t max_length() = 0;
        /**
         * @brief Returns the target display row.
         *
         * @return Row index.
         */
        virtual uint8_t row() = 0;
        /**
         * @brief Returns the target display column.
         *
         * @return Column index.
         */
        virtual uint8_t column() = 0;
        /**
         * @brief Returns whether the element content should be retained temporarily.
         *
         * @return `true` when retention is enabled, otherwise `false`.
         */
        virtual bool use_retention() = 0;
        /**
         * @brief Returns the current element text.
         *
         * @return Pointer to the current null-terminated text buffer.
         */
        virtual const char* text() = 0;
        /**
         * @brief Replaces the element text with a preformatted string.
         *
         * @param text New text to copy into the element buffer.
         */
        virtual void set_text_raw(const char* text) = 0;
        /**
         * @brief Returns the bitmask of characters changed since the last clear.
         *
         * @return Change bitmask.
         */
        virtual uint32_t change() = 0;
        /**
         * @brief Clears the accumulated change bitmask.
         */
        virtual void clear_change() = 0;
        /**
         * @brief Returns the timestamp of the last text update.
         *
         * @return Last update time in milliseconds.
         */
        virtual uint32_t last_update_time() = 0;

        /**
         * @brief Formats and stores new element text.
         *
         * @tparam Args Argument types forwarded to `snprintf`.
         *
         * @param text Format string or plain text.
         * @param args Optional format arguments.
         */
        template<typename... Args>
        void set_text(const char* text, Args... args)
        {
            static constexpr uint8_t TEXT_BUFFER_SIZE = 32;

            char temp_buff[TEXT_BUFFER_SIZE] = {};

            if constexpr (sizeof...(args) == 0)
            {
                [[maybe_unused]] auto ret = snprintf(temp_buff, sizeof(temp_buff), "%s", text);
            }
            else
            {
                [[maybe_unused]] auto ret = snprintf(temp_buff, sizeof(temp_buff), text, args...);
            }

            set_text_raw(temp_buff);
        }
    };

    /**
     * @brief Fixed-layout display element implementation.
     *
     * @tparam MaxLength Maximum supported text length.
     * @tparam RowIndex Fixed row index.
     * @tparam ColumnIndex Fixed column index.
     * @tparam UseRetention Whether temporary retention behavior is enabled.
     */
    template<uint8_t MaxLength, uint8_t RowIndex, uint8_t ColumnIndex, bool UseRetention>
    class DisplayElement : public DisplayTextControl
    {
        public:
        DisplayElement() = default;

        /**
         * @brief Returns the fixed maximum text length for the element.
         *
         * @return Maximum text length in characters.
         */
        uint8_t max_length() override
        {
            return MaxLength;
        }

        /**
         * @brief Returns the fixed row index for the element.
         *
         * @return Row index.
         */
        uint8_t row() override
        {
            return RowIndex;
        }

        /**
         * @brief Returns the fixed column index for the element.
         *
         * @return Column index.
         */
        uint8_t column() override
        {
            return ColumnIndex;
        }

        /**
         * @brief Returns whether retention behavior is enabled for the element.
         *
         * @return `true` when retention is enabled, otherwise `false`.
         */
        bool use_retention() override
        {
            return UseRetention;
        }

        /**
         * @brief Returns the current element text buffer.
         *
         * @return Pointer to the current null-terminated text buffer.
         */
        const char* text() override
        {
            return _text;
        }

        /**
         * @brief Updates the text buffer and tracks which character positions changed.
         *
         * @param text New text to store.
         */
        void set_text_raw(const char* text) override
        {
            static constexpr uint8_t TEXT_BUFFER_SIZE = 32;

            static_assert((MaxLength + 1) <= TEXT_BUFFER_SIZE, "Provided element size too large");
            static char temp_buff[TEXT_BUFFER_SIZE] = {};
            int         len                         = snprintf(temp_buff, sizeof(temp_buff), "%s", text);

            if (len >= static_cast<int>(sizeof(temp_buff) - 1))
            {
                // overflow
                return;
            }

            memset(&temp_buff[strlen(temp_buff)], ' ', sizeof(temp_buff) - len - 1);
            temp_buff[sizeof(temp_buff) - 1] = '\0';

            for (size_t i = 0; i < MaxLength; i++)
            {
                if (temp_buff[i] != _text[i])
                {
                    zlibs::utils::misc::bit_write(_text_change, i, true);
                }
            }

            strncpy(_text, temp_buff, MaxLength);
            _last_update_time = k_uptime_get_32();
        }

        /**
         * @brief Returns the accumulated change bitmask.
         *
         * @return Change bitmask.
         */
        uint32_t change() override
        {
            return _text_change;
        }

        /**
         * @brief Clears the accumulated change bitmask.
         */
        void clear_change() override
        {
            _text_change = 0;
        }

        /**
         * @brief Returns the timestamp of the last text update.
         *
         * @return Last update time in milliseconds.
         */
        uint32_t last_update_time() override
        {
            return _last_update_time;
        }

        private:
        char     _text[MaxLength + 1] = {};
        uint32_t _text_change         = 0;
        uint32_t _last_update_time    = 0;
    };
}    // namespace opendeck::io::i2c::display
