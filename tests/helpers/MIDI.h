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
#include <HWTestDefines.h>
#include <glog/logging.h>

class MIDIHelper
{
    public:
    MIDIHelper() = default;

    static std::vector<MIDI::USBMIDIpacket_t> rawSysExToUSBPackets(const std::vector<uint8_t>& raw)
    {
        class HWAFillMIDI : public MIDI::HWA
        {
            public:
            HWAFillMIDI(std::vector<MIDI::USBMIDIpacket_t>& buffer)
                : _buffer(buffer)
            {}

            bool init(MIDI::interface_t interface) override
            {
                return true;
            }

            bool deInit(MIDI::interface_t interface) override
            {
                return true;
            }

            bool dinRead(uint8_t& data) override
            {
                return false;
            }

            bool dinWrite(uint8_t data) override
            {
                return false;
            }

            bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
            {
                return false;
            }

            bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
            {
                _buffer.push_back(USBMIDIpacket);
                return true;
            }

            std::vector<MIDI::USBMIDIpacket_t>& _buffer;
        };

        // create temp midi object whose purpose is to convert provided raw sysex array into
        // a series of USB MIDI packets
        std::vector<MIDI::USBMIDIpacket_t> usbPackets;
        HWAFillMIDI                        hwaFillMIDI(usbPackets);
        MIDI                               fillMIDI(hwaFillMIDI);

        fillMIDI.init(MIDI::interface_t::usb);
        fillMIDI.sendSysEx(raw.size(), &raw[0], true);

        return usbPackets;
    }

    static std::vector<uint8_t> usbSysExToRawBytes(std::vector<MIDI::USBMIDIpacket_t>& raw)
    {
        class HWAParseMIDI : public MIDI::HWA
        {
            public:
            HWAParseMIDI(std::vector<MIDI::USBMIDIpacket_t>& buffer)
                : _buffer(buffer)
            {}

            bool init(MIDI::interface_t interface) override
            {
                return true;
            }

            bool deInit(MIDI::interface_t interface) override
            {
                return true;
            }

            bool dinRead(uint8_t& data) override
            {
                return false;
            }

            bool dinWrite(uint8_t data) override
            {
                _parsed.push_back(data);
                return true;
            }

            bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
            {
                if (!_buffer.size())
                    return false;

                USBMIDIpacket = _buffer.at(0);
                _buffer.erase(_buffer.begin());

                return true;
            }

            bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
            {
                return true;
            }

            std::vector<MIDI::USBMIDIpacket_t>& _buffer;
            std::vector<uint8_t>                _parsed;
        };

        HWAParseMIDI hwaParseMIDI(raw);
        MIDI         parseMIDI(hwaParseMIDI);

        parseMIDI.init(MIDI::interface_t::all);
        parseMIDI.setInputChannel(MIDI::MIDI_CHANNEL_OMNI);

        auto packetSize = raw.size();

        for (size_t i = 0; i < packetSize; i++)
            parseMIDI.read(MIDI::interface_t::usb, MIDI::filterMode_t::fullDIN);

        return hwaParseMIDI._parsed;
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
            static_cast<uint8_t>(SysExConf::status_t::request),    // status
            0,                                                     // part
            static_cast<uint8_t>(SysExConf::wish_t::get),          // wish
            static_cast<uint8_t>(SysExConf::amount_t::single),     // amount
            static_cast<uint8_t>(blockIndex),                      // block
            static_cast<uint8_t>(section),                         // section
            splitIndex.high(),                                     // index high byte
            splitIndex.low(),                                      // index low byte
            0x00,                                                  // new value high byte
            0x00,                                                  // new value low byte
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

#ifdef HW_TESTING
    template<typename T>
    static uint16_t readFromBoard(T section, size_t index)
    {
        std::vector<uint8_t> requestUint8;
        generateSysExGetReq(section, index, requestUint8);

        return sendRequest(requestUint8, SysExConf::wish_t::get);
    }

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

    static void flush()
    {
        LOG(INFO) << "Flushing all incoming data from the board";
        std::string cmdResponse;

        std::string deviceNameSearch = "$(amidi -l | grep \"OpenDeck | " + std::string(BOARD_STRING) + "\"";

        std::string cmd = std::string("stdbuf -i0 -o0 -e0 amidi -p ") + deviceNameSearch + std::string(" | grep -Eo 'hw:\\S*') -d -t 3");
        test::wsystem(cmd, cmdResponse);

// do the same for din interface if present
#ifdef TEST_DIN_MIDI_PORT
        LOG(INFO) << "Flushing all incoming data from the DIN MIDI interface";
        cmd = std::string("amidi -p $(amidi -l | grep -E '") + std::string(DIN_MIDI_PORT) + std::string("' | grep -Eo 'hw:\\S*') -d -t 3");
        test::wsystem(cmd, cmdResponse);
#endif
    }

    static std::string sendRawSysEx(std::string req, bool verifyResponse = true)
    {
        std::string cmdResponse;
        std::string lastResponseFileLocation = "/tmp/midi_in_data.txt";

        test::wsystem("rm -f " + lastResponseFileLocation, cmdResponse);
        LOG(INFO) << "req: " << req;

        std::string deviceNameSearch = "$(amidi -l | grep \"OpenDeck | " + std::string(BOARD_STRING) + "\"";
        std::string cmd              = std::string("stdbuf -i0 -o0 -e0 amidi -p ") + deviceNameSearch + std::string(" | grep -Eo 'hw:\\S*') -S '") + req + "' -d | stdbuf -i0 -o0 -e0 tr -d '\\n' > " + lastResponseFileLocation + " &";
        test::wsystem(cmd, cmdResponse);

        if (test::wordsInString(req) < SysExConf::SPECIAL_REQ_MSG_SIZE)
        {
            LOG(ERROR) << "Invalid request";
            return "";
        }

        if (verifyResponse)
        {
            // change status byte to ack
            req[13] = '1';

            // remove everything after request/wish byte
            std::string pattern = req.substr(0, 20) + ".*F7";

            cmd                          = "cat " + lastResponseFileLocation + " | xargs | sed 's/F7/F7\\n/g' | sed 's/F0/\\nF0/g' | grep -m 1 -E '" + pattern + "'";
            const uint32_t waitTimeMs    = 10;
            const uint32_t stopWaitAfter = 2000;
            uint32_t       totalWaitTime = 0;

            while (test::wsystem(cmd, cmdResponse))
            {
                test::sleepMs(waitTimeMs);
                totalWaitTime += waitTimeMs;

                if (totalWaitTime == stopWaitAfter)
                {
                    LOG(ERROR) << "Failed to find valid response to request. Outputting response:";
                    test::wsystem("cat " + lastResponseFileLocation, cmdResponse);
                    LOG(INFO) << cmdResponse << "\n"
                                                "Search pattern was: "
                              << pattern;

                    test::wsystem("killall amidi > /dev/null 2>&1");
                    return "";
                }
            }

            test::wsystem("killall amidi > /dev/null 2>&1");
            test::trimNewline(cmdResponse);
            LOG(INFO) << "res: " << cmdResponse;

            return cmdResponse;
        }
        else
        {
            test::sleepMs(100);
            test::wsystem("killall amidi > /dev/null 2>&1");

            return "";
        }
    }

    static bool devicePresent(bool bootloader = false)
    {
        std::string port = amidiPort(bootloader);
        std::string cmdResponse;

        if (port == "")
            return false;

        return (test::wsystem("amidi -l | grep \"" + amidiPort(bootloader) + "\"", cmdResponse) == 0);
    }

    static std::string amidiPort(bool bootloader = false)
    {
        std::string cmd;
        std::string cmdResponse;

        if (bootloader)
        {
            std::string baseString = "amidi -l | grep \"OpenDeck DFU | ";
            cmd                    = baseString + "\"" + std::string("| grep ") + std::string(BOARD_STRING) + std::string(" | grep -Eo 'hw:\\S*'");
        }
        else
        {
            std::string baseString = "amidi -l | grep \"OpenDeck | ";
            cmd                    = baseString + std::string(BOARD_STRING) + "\"" + std::string(" | grep -Eo 'hw:\\S*'");
        }

        test::wsystem(cmd, cmdResponse);

        return test::trimNewline(cmdResponse);
    }
#endif

    private:
#ifdef HW_TESTING
    static uint16_t sendRequest(const std::vector<uint8_t>& requestUint8, SysExConf::wish_t wish)
    {
        // convert uint8_t vector to string so it can be passed as command line argument
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

        std::string responseString = sendRawSysEx(requestString.str());

        if (responseString == "")
            return 0;    // invalid response

        // convert response back to uint8 vector
        std::vector<uint8_t> responseUint8;

        for (size_t i = 0; i < responseString.length(); i += 3)
        {
            std::string byteString = responseString.substr(i, 2);
            char        byte       = (char)strtol(byteString.c_str(), NULL, 16);
            responseUint8.push_back(byte);
        }

        if (wish == SysExConf::wish_t::get)
        {
            // last two bytes are result
            MIDI::Merge14bit merge14bit;
            merge14bit.merge(responseUint8.at(responseUint8.size() - 3), responseUint8.at(responseUint8.size() - 2));
            return merge14bit.value();
        }
        else
        {
            // read status byte
            return responseUint8.at(4);
        }
    }
#endif

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