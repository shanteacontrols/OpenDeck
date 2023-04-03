#pragma once

#include <string>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <time.h>
#include <errno.h>

namespace test
{
    // simple wrapper for system call which takes string arg instead
    inline int wsystem(std::string const& in, std::string& out)
    {
        // std::cout << "[CMD]: " << in << std::endl;

        FILE* fpipe = popen(in.c_str(), "r");

        if (fpipe == NULL)
        {
            throw std::runtime_error(std::string("Can't run ") + in);
        }

        out = "";

        static constexpr size_t BUFFER_SIZE = 256;
        char                    buffer[BUFFER_SIZE];

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

    inline int wsystem(std::string const& in)
    {
        // std::cout << "[CMD]: " << in << std::endl;
        return system(in.c_str());
    }

    inline std::string trimWhitespace(const std::string& s)
    {
        auto trimmed = s;

        trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(), [](char c)
                                     {
                                         return std::isspace(static_cast<unsigned char>(c));
                                     }),
                      trimmed.end());

        return trimmed;
    }

    inline size_t wordsInString(std::string& s)
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

    int sleepMs(long msec)
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

    int64_t millis()
    {
        struct timespec now;
        timespec_get(&now, TIME_UTC);
        return (static_cast<int64_t>(now.tv_sec)) * 1000 + (static_cast<int64_t>(now.tv_nsec)) / 1000000;
    }

    std::vector<uint8_t> hexStringToVector(const std::string& string)
    {
        std::vector<unsigned char> vector;

        for (size_t i = 0; i < string.length(); i += 2)
        {
            std::string byteString = string.substr(i, 2);
            auto        byte       = static_cast<unsigned char>(strtol(byteString.c_str(), NULL, 16));
            vector.push_back(byte);
        }

        return vector;
    }

    std::string vectorToHexString(std::vector<uint8_t> request)
    {
        // convert uint8_t vector to string so it can be passed as command line argument
        std::stringstream requestString;
        requestString << std::hex << std::setfill('0') << std::uppercase;

        auto first = std::begin(request);
        auto last  = std::end(request);

        while (first != last)
        {
            requestString << std::setw(2) << static_cast<int>(*first++);
        }

        return requestString.str();
    }
}    // namespace test
