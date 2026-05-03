/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "io/base.h"

#include <zephyr/kernel.h>

#include <string>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdint>
#include <time.h>
#include <errno.h>
#include <iomanip>

namespace opendeck::tests
{
    /**
     * @brief Resumes IO processing and waits for worker threads to observe it.
     */
    inline void resume_io()
    {
        io::Base::resume();
        k_msleep(20);
    }

    /**
     * @brief Polls until a condition becomes true or a timeout expires.
     *
     * @param [in] condition Predicate evaluated between sleep intervals.
     * @param [in] timeout_ms Maximum time to wait before returning.
     * @param [in] poll_ms Delay between condition checks.
     *
     * @return `true` if the condition became true within the timeout window,
     *         otherwise the final condition value after the timeout expires.
     */
    template<typename Condition>
    bool wait_until(Condition condition, const int32_t timeout_ms = 200, const int32_t poll_ms = 20)
    {
        for (int32_t elapsed_ms = 0; elapsed_ms < timeout_ms; elapsed_ms += poll_ms)
        {
            if (condition())
            {
                return true;
            }

            k_msleep(poll_ms);
        }

        return condition();
    }

    /**
     * @brief Executes a shell command and captures its standard output.
     *
     * @param in Shell command to execute.
     * @param out Output string filled with captured command output.
     *
     * @return Process exit status, or `-1` when the command could not be started.
     */
    inline int wsystem(std::string const& in, std::string& out)
    {
        FILE* fpipe = popen(in.c_str(), "r");

        if (fpipe == NULL)
        {
            out = "";
            return -1;
        }

        out = "";

        static constexpr size_t BUFFER_SIZE = 256;

        char buffer[BUFFER_SIZE];

        while (!feof(fpipe))
        {
            if (fgets(buffer, BUFFER_SIZE, fpipe) != NULL)
            {
                out += buffer;
            }
        }

        int status = pclose(fpipe);

        return status;
    }

    /**
     * @brief Executes a shell command without capturing its output.
     *
     * @param in Shell command to execute.
     *
     * @return Process exit status.
     */
    inline int wsystem(std::string const& in)
    {
        return system(in.c_str());
    }

    /**
     * @brief Removes all whitespace characters from a string copy.
     *
     * @param s Input string.
     *
     * @return Copy of the input string with whitespace removed.
     */
    inline std::string trim_whitespace(const std::string& s)
    {
        auto trimmed = s;

        trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(), [](char c)
                                     {
                                         return std::isspace(static_cast<unsigned char>(c));
                                     }),
                      trimmed.end());

        return trimmed;
    }

    /**
     * @brief Counts spaces in a mutable string and returns one plus that count.
     *
     * @param s String to inspect.
     *
     * @return Number of spaces plus one.
     */
    inline size_t words_in_string(std::string& s)
    {
        size_t words = 0;

        for (size_t i = 0; s[i] != '\0'; i++)
        {
            if (s[i] == ' ')
            {
                words++;
            }
        }

        return words + 1;
    }

    /**
     * @brief Sleeps the current thread for the requested number of milliseconds.
     *
     * @param msec Duration to sleep in milliseconds.
     *
     * @return `0` on success, or `-1` when the argument is invalid.
     */
    inline int sleep_ms(long msec)
    {
        struct timespec ts;
        int             res;

        if (msec < 0)
        {
            errno = EINVAL;
            return -1;
        }

        ts.tv_sec  = msec / 1000;
        ts.tv_nsec = (msec % 1000) * 1000000;

        do
        {
            res = nanosleep(&ts, &ts);
        } while (res && errno == EINTR);

        return res;
    }

    /**
     * @brief Returns the current wall-clock time in milliseconds.
     *
     * @return Current timestamp in milliseconds.
     */
    inline int64_t millis()
    {
        struct timespec now;
        timespec_get(&now, TIME_UTC);
        return (static_cast<int64_t>(now.tv_sec)) * 1000 + (static_cast<int64_t>(now.tv_nsec)) / 1000000;
    }

    /**
     * @brief Converts an even-length hexadecimal string into a byte vector.
     *
     * @param string Hexadecimal string to parse.
     *
     * @return Parsed byte vector.
     */
    inline std::vector<uint8_t> hex_string_to_vector(const std::string& string)
    {
        std::vector<unsigned char> vector;

        for (size_t i = 0; i < string.length(); i += 2)
        {
            std::string byte_string = string.substr(i, 2);
            auto        byte        = static_cast<unsigned char>(strtol(byte_string.c_str(), NULL, 16));
            vector.push_back(byte);
        }

        return vector;
    }

    /**
     * @brief Converts a byte vector into an uppercase hexadecimal string.
     *
     * @param request Byte vector to serialize.
     *
     * @return Uppercase hexadecimal string without separators.
     */
    inline std::string vector_to_hex_string(std::vector<uint8_t> request)
    {
        std::stringstream request_string;
        request_string << std::hex << std::setfill('0') << std::uppercase;

        auto first = std::begin(request);
        auto last  = std::end(request);

        while (first != last)
        {
            request_string << std::setw(2) << static_cast<int>(*first++);
        }

        return request_string.str();
    }
}    // namespace opendeck::tests
