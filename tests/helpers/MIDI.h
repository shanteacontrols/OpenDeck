#pragma once

#include <cinttypes>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include "midi/src/MIDI.h"
#include "sysex/src/SysExConf.h"
#include "system/System.h"
#include "Misc.h"

class MIDIHelper
{
    public:
    MIDIHelper() = default;

    template<typename T>
    static bool setSingleSysExReq(T section, size_t index, uint16_t value)
    {
        auto             blockIndex = block(section);
        MIDI::Split14bit indexSplit;
        MIDI::Split14bit valueSplit;

        indexSplit.split(index);
        valueSplit.split(value);

        const std::vector<uint8_t> requestUint8 = {
            0xF0,
            SYSEX_MANUFACTURER_ID_0,
            SYSEX_MANUFACTURER_ID_1,
            SYSEX_MANUFACTURER_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::request),
            0,
            static_cast<uint8_t>(SysExConf::wish_t::set),
            static_cast<uint8_t>(SysExConf::amount_t::single),
            static_cast<uint8_t>(blockIndex),
            static_cast<uint8_t>(section),
            indexSplit.high(),
            indexSplit.low(),
            valueSplit.high(),
            valueSplit.low(),
            0xF7
        };

        return sendRequest(requestUint8, SysExConf::wish_t::set);
    }

    template<typename T>
    static void generateSysExGetReq(T section, size_t index, std::vector<uint8_t>& request)
    {
        auto             blockIndex = block(section);
        MIDI::Split14bit splitIndex;

        splitIndex.split(index);
        request.clear();

        request = {
            0xF0,
            SYSEX_MANUFACTURER_ID_0,
            SYSEX_MANUFACTURER_ID_1,
            SYSEX_MANUFACTURER_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::request),    //status
            0,                                                     //part
            static_cast<uint8_t>(SysExConf::wish_t::get),          //wish
            static_cast<uint8_t>(SysExConf::amount_t::single),     //amount
            static_cast<uint8_t>(blockIndex),                      //block
            static_cast<uint8_t>(section),                         //section
            splitIndex.high(),                                     //index high byte
            splitIndex.low(),                                      //index low byte
            0x00,                                                  //new value high byte
            0x00,                                                  //new value low byte
            0xF7
        };
    }

    template<typename T>
    static void generateSysExSetReq(T section, size_t index, int16_t value, std::vector<uint8_t>& request)
    {
        auto             blockIndex = block(section);
        MIDI::Split14bit splitIndex;
        MIDI::Split14bit splitValue;

        splitIndex.split(index);
        splitValue.split(value);
        request.clear();

        request = {
            0xF0,
            SYSEX_MANUFACTURER_ID_0,
            SYSEX_MANUFACTURER_ID_1,
            SYSEX_MANUFACTURER_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::request),
            0,
            static_cast<uint8_t>(SysExConf::wish_t::set),
            static_cast<uint8_t>(SysExConf::amount_t::single),
            static_cast<uint8_t>(blockIndex),
            static_cast<uint8_t>(section),
            splitIndex.high(),
            splitIndex.low(),
            splitValue.high(),
            splitValue.low(),
            0xF7
        };
    }

    template<typename T>
    static uint16_t readFromBoard(T section, size_t index)
    {
        std::vector<uint8_t> requestUint8;
        generateSysExGetReq(section, index, requestUint8);

        return sendRequest(requestUint8, SysExConf::wish_t::get);
    }

    static std::string sendRawSysEx(std::string req, bool setReq = true)
    {
        std::string cmdResponse;
        std::string lastResponseFileLocation = "/tmp/midi_in_data.txt";

        test::wsystem("rm -f " + lastResponseFileLocation, cmdResponse);
        std::cout << "req: " << req << std::endl;
        std::string cmd = std::string("stdbuf -i0 -o0 -e0 amidi -p $(amidi -l | grep -E 'OpenDeck'") + std::string(" | grep -Eo 'hw:\\S*') -S '") + req + "' -d | stdbuf -i0 -o0 -e0 tr -d '\n' > " + lastResponseFileLocation + " &";
        test::wsystem(cmd, cmdResponse);

        size_t responseRetryCounter = 0;

        std::string pattern = setReq ? "F0 00 53 43 01 00 01.*F7$" : "F0 00 53 43 01.*F7$";

        while (test::wsystem("grep '" + pattern + "' " + lastResponseFileLocation, cmdResponse))
        {
            test::wsystem("sleep 0.01", cmdResponse);
            responseRetryCounter++;

            //allow 1 second of response time
            if (responseRetryCounter == 100)
            {
                test::wsystem("killall amidi > /dev/null 2>&1");
                return "";
            }
        }

        test::wsystem("killall amidi > /dev/null 2>&1", cmdResponse);

        //cut everything before the last F0
        test::wsystem("sed -i 's/^.*F0/F0/' " + lastResponseFileLocation, cmdResponse);
        //and also everything after F7
        test::wsystem("sed -i 's/F7.*/F7/' " + lastResponseFileLocation, cmdResponse);

        auto response = lastResponse(lastResponseFileLocation);

        std::cout << "res: " << response << std::endl;

        return response;
    }

    private:
    static std::string lastResponse(const std::string& location)
    {
        //last response is in last line in provided file path
        std::ifstream file;
        std::string   lastline = "";

        file.open(location);

        if (file.is_open())
        {
            char ch = ' ';
            file.seekg(0, std::ios_base::end);

            while (ch != '\n')
            {
                file.seekg(-2, std::ios_base::cur);

                if ((int)file.tellg() <= 0)
                {                     //If passed the start of the file,
                    file.seekg(0);    //this is the start of the line
                    break;
                }

                file.get(ch);
            }

            std::getline(file, lastline);
            file.close();
        }

        return lastline;
    }

    static uint16_t sendRequest(const std::vector<uint8_t>& requestUint8, SysExConf::wish_t wish)
    {
        //convert uint8_t vector to string so it can be passed as command line argument
        std::stringstream requestString;
        requestString << std::hex << std::setfill('0') << std::uppercase;

        auto first = std::begin(requestUint8);
        auto last  = std::end(requestUint8);

        while (first != last)
        {
            requestString << std::setw(2) << static_cast<int>(*first++);

            if (first != last)
                requestString << " ";
        }

        std::string responseString = sendRawSysEx(requestString.str(), wish == SysExConf::wish_t::set);

        if (responseString == "")
            return 0;    //invalid response

        //convert response back to uint8 vector
        std::vector<uint8_t> responseUint8;

        for (size_t i = 0; i < responseString.length(); i += 3)
        {
            std::string byteString = responseString.substr(i, 2);
            char        byte       = (char)strtol(byteString.c_str(), NULL, 16);
            responseUint8.push_back(byte);
        }

        if (wish == SysExConf::wish_t::get)
        {
            //last two bytes are result
            MIDI::Merge14bit merge14bit;
            merge14bit.merge(responseUint8.at(responseUint8.size() - 3), responseUint8.at(responseUint8.size() - 2));
            return merge14bit.value();
        }
        else
        {
            //read status byte
            return responseUint8.at(4);
        }
    }

    static System::block_t block(System::Section::global_t section)
    {
        return System::block_t::global;
    }

    static System::block_t block(System::Section::button_t section)
    {
        return System::block_t::buttons;
    }

    static System::block_t block(System::Section::encoder_t section)
    {
        return System::block_t::encoders;
    }

    static System::block_t block(System::Section::analog_t section)
    {
        return System::block_t::analog;
    }

    static System::block_t block(System::Section::leds_t section)
    {
        return System::block_t::leds;
    }

    static System::block_t block(System::Section::display_t section)
    {
        return System::block_t::display;
    }

    static System::block_t block(System::Section::touchscreen_t section)
    {
        return System::block_t::touchscreen;
    }
};