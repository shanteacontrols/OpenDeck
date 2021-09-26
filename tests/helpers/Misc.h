#pragma once

#include <string>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace test
{
    //simple wrapper for system call which takes string arg instead
    inline int wsystem(std::string const& in, std::string& out)
    {
        // std::cout << "[CMD]: " << in << std::endl;

        FILE* fpipe = popen(in.c_str(), "r");

        if (fpipe == NULL)
            throw std::runtime_error(std::string("Can't run ") + in);

        out = "";

        const size_t BUFFER_SIZE = 256;
        char         buffer[BUFFER_SIZE];

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

    inline std::string& trimNewline(std::string& s)
    {
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
        return s;
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

}    // namespace test
