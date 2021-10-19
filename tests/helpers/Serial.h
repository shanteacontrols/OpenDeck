#pragma once

#include <cinttypes>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include "Misc.h"

class SerialHelper
{
    public:
    SerialHelper() = default;

    static std::vector<uint8_t> sendToBoard(const std::string& port, std::vector<uint8_t> req)
    {
        std::cout << "[cdc] req: ";

        for (uint8_t i : req)
            std::cout << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(i) << ' ';

        std::cout << std::endl;

        std::string rawString;
        std::string cmdResponse;

        for (size_t i = 0; i < req.size(); i++)
        {
            std::stringstream stream;
            stream << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(req.at(i));

            rawString += stream.str();
        }

        std::string responseFileLocation = "/tmp/cdc_in_data.txt";
        std::string cmd =
            std::string("stdbuf -i0 -o0 printf ") +
            rawString +
            std::string(" | stdbuf -i0 -o0 -e0 ssterm -b ") +
            std::to_string(UART_BAUDRATE_USB) +
            std::string(" -o hex -i hex ") + port + " > " + responseFileLocation + " &";

        test::wsystem(cmd, cmdResponse);
        test::wsystem("sleep 1", cmdResponse);
        test::wsystem("killall ssterm > /dev/null 2>&1");

        test::wsystem("cat " + responseFileLocation + " | xargs", cmdResponse);
        std::cout << "[cdc] res: " << cmdResponse << std::endl;

        auto HexCharToInt = [](char Input) {
            return ((Input >= 'a') && (Input <= 'f'))
                       ? (Input - 87)
                   : ((Input >= 'A') && (Input <= 'F'))
                       ? (Input - 55)
                   : ((Input >= '0') && (Input <= '9'))
                       ? (Input - 48)
                       : throw std::exception{};
        };

        std::vector<uint8_t> response;

        for (size_t i = 0; cmdResponse[i] != '\0'; i++)
        {
            if (cmdResponse[i] == ' ' || cmdResponse[i] == '\n')
                continue;

            uint8_t number = HexCharToInt(cmdResponse[i]);
            number <<= 4;
            number |= HexCharToInt(cmdResponse[i + 1]);
            response.push_back(number);

            ++i;
        }

        test::wsystem("rm -f " + responseFileLocation);

        return response;
    }
};