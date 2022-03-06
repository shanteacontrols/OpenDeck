#pragma once

#include <cinttypes>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include "protocol/midi/MIDI.h"
#include "sysex/src/SysExConf.h"
#include "util/conversion/Conversion.h"
#include "system/System.h"
#include "Misc.h"
#include <glog/logging.h>

#ifdef HW_TESTS_SUPPORTED
#include <HWTestDefines.h>
#endif

using namespace Protocol;

class MIDIHelper
{
    public:
    MIDIHelper() = default;

    static std::vector<MIDI::usbMIDIPacket_t> rawSysExToUSBPackets(std::vector<uint8_t>& raw)
    {
        Messaging::event_t event;
        event.sysEx       = &raw[0];
        event.sysExLength = raw.size();
        event.message     = MIDI::messageType_t::systemExclusive;

        return MIDIHelper::midiToUsbPackets(event);
    }

    static std::vector<MIDI::usbMIDIPacket_t> midiToUsbPackets(Messaging::event_t event)
    {
        class HWAWriteToUSB : public MIDIlib::USBMIDI::HWA
        {
            public:
            HWAWriteToUSB() = default;

            bool init() override
            {
                return true;
            }

            bool deInit() override
            {
                return true;
            }

            bool read(MIDI::usbMIDIPacket_t& packet) override
            {
                return false;
            }

            bool write(MIDI::usbMIDIPacket_t packet) override
            {
                _buffer.push_back(packet);
                return true;
            }

            std::vector<MIDI::usbMIDIPacket_t> _buffer;
        } _hwaWriteToUSB;

        MIDIlib::USBMIDI _writeToUsb(_hwaWriteToUSB);
        _writeToUsb.init();

        switch (event.message)
        {
        case MIDI::messageType_t::noteOff:
        {
            _writeToUsb.sendNoteOff(event.midiIndex, event.midiValue, event.midiChannel);
        }
        break;

        case MIDI::messageType_t::noteOn:
        {
            _writeToUsb.sendNoteOff(event.midiIndex, event.midiValue, event.midiChannel);
        }
        break;

        case MIDI::messageType_t::controlChange:
        {
            _writeToUsb.sendControlChange(event.midiIndex, event.midiValue, event.midiChannel);
        }
        break;

        case MIDI::messageType_t::programChange:
        {
            _writeToUsb.sendProgramChange(event.midiIndex, event.midiChannel);
        }
        break;

        case MIDI::messageType_t::afterTouchChannel:
        {
            _writeToUsb.sendAfterTouch(event.midiValue, event.midiChannel);
        }
        break;

        case MIDI::messageType_t::afterTouchPoly:
        {
            _writeToUsb.sendAfterTouch(event.midiValue, event.midiChannel, event.midiIndex);
        }
        break;

        case MIDI::messageType_t::pitchBend:
        {
            _writeToUsb.sendPitchBend(event.midiValue, event.midiChannel);
        }
        break;

        case MIDI::messageType_t::sysRealTimeClock:
        {
            _writeToUsb.sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::sysRealTimeStart:
        {
            _writeToUsb.sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::sysRealTimeContinue:
        {
            _writeToUsb.sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::sysRealTimeStop:
        {
            _writeToUsb.sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::sysRealTimeActiveSensing:
        {
            _writeToUsb.sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::sysRealTimeSystemReset:
        {
            _writeToUsb.sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::mmcPlay:
        {
            _writeToUsb.sendMMC(event.midiIndex, event.message);
        }
        break;

        case MIDI::messageType_t::mmcStop:
        {
            _writeToUsb.sendMMC(event.midiIndex, event.message);
        }
        break;

        case MIDI::messageType_t::mmcPause:
        {
            _writeToUsb.sendMMC(event.midiIndex, event.message);
        }
        break;

        case MIDI::messageType_t::mmcRecordStart:
        {
            _writeToUsb.sendMMC(event.midiIndex, event.message);
        }
        break;

        case MIDI::messageType_t::mmcRecordStop:
        {
            _writeToUsb.sendMMC(event.midiIndex, event.message);
        }
        break;

        case MIDI::messageType_t::nrpn7bit:
        {
            _writeToUsb.sendNRPN(event.midiIndex, event.midiValue, event.midiChannel, false);
        }
        break;

        case MIDI::messageType_t::nrpn14bit:
        {
            _writeToUsb.sendNRPN(event.midiIndex, event.midiValue, event.midiChannel, true);
        }
        break;

        case MIDI::messageType_t::controlChange14bit:
        {
            _writeToUsb.sendControlChange14bit(event.midiIndex, event.midiValue, event.midiChannel);
        }
        break;

        case MIDI::messageType_t::systemExclusive:
        {
            _writeToUsb.sendSysEx(event.sysExLength, event.sysEx, true);
        }
        break;

        default:
            break;
        }

        return _hwaWriteToUSB._buffer;
    }

    template<typename T>
    static void generateSysExGetReq(T section, size_t index, std::vector<uint8_t>& request)
    {
        auto blockIndex = block(section);
        auto split      = Util::Conversion::Split14bit(index);

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
            split.high(),                                          // index high byte
            split.low(),                                           // index low byte
            0x00,                                                  // new value high byte
            0x00,                                                  // new value low byte
            0xF7
        };
    }

    template<typename S, typename I, typename V>
    static void generateSysExSetReq(S section, I index, V value, std::vector<uint8_t>& request)
    {
        auto blockIndex = block(section);
        auto splitIndex = Util::Conversion::Split14bit(static_cast<uint16_t>(index));
        auto splitValue = Util::Conversion::Split14bit(static_cast<uint16_t>(value));

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

#ifdef HW_TESTS_SUPPORTED
    template<typename T>
    static uint16_t readFromDevice(T section, size_t index)
    {
        std::vector<uint8_t> requestUint8;
        generateSysExGetReq(section, index, requestUint8);

        return sendRequest(requestUint8, SysExConf::wish_t::get);
    }

    template<typename S, typename I, typename V>
    static bool setSingleSysExReq(S section, I index, V value)
    {
        auto blockIndex = block(section);
        auto indexSplit = Util::Conversion::Split14bit(static_cast<uint16_t>(index));
        auto valueSplit = Util::Conversion::Split14bit(static_cast<uint16_t>(value));

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
        LOG(INFO) << "Flushing all incoming data from the OpenDeck device";
        std::string cmdResponse;

        std::string cmd = std::string("amidi -p ") + amidiPort(OPENDECK_MIDI_DEVICE_NAME) + std::string(" -d -t 3");
        test::wsystem(cmd, cmdResponse);

// do the same for din interface if present
#ifdef TEST_DIN_MIDI
        LOG(INFO) << "Flushing all incoming data from the DIN MIDI interface on device";
        cmd = std::string("amidi -p ") + amidiPort(OUT_DIN_MIDI_PORT) + std::string(" -d -t 3");
        test::wsystem(cmd, cmdResponse);
#endif
    }

    static std::string sendRawSysEx(std::string req, bool expectResponse = true)
    {
        std::string cmdResponse;
        std::string lastResponseFileLocation = "/tmp/midi_in_data.txt";

        test::wsystem("rm -f " + lastResponseFileLocation, cmdResponse);
        LOG(INFO) << "req: " << req;

        std::string cmd = std::string("stdbuf -i0 -o0 -e0 amidi -p ") + amidiPort(OPENDECK_MIDI_DEVICE_NAME) + std::string(" -S '") + req + "' -d | stdbuf -i0 -o0 -e0 tr -d '\\n' > " + lastResponseFileLocation + " &";
        test::wsystem(cmd, cmdResponse);

        if (test::wordsInString(req) < SysExConf::SPECIAL_REQ_MSG_SIZE)
        {
            LOG(ERROR) << "Invalid request";
            return "";
        }

        const uint32_t waitTimeMs    = 10;
        const uint32_t stopWaitAfter = 2000;
        uint32_t       totalWaitTime = 0;

        if (expectResponse)
        {
            // change status byte to ack
            req[13] = '1';

            // remove everything after request/wish byte
            std::string pattern = req.substr(0, 20) + ".*F7";

            cmd = "cat " + lastResponseFileLocation + " | xargs | sed 's/F7/F7\\n/g' | sed 's/F0/\\nF0/g' | grep -m 1 -E '" + pattern + "'";

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
            cmd         = "[ -s " + lastResponseFileLocation + " ]";
            cmdResponse = "";

            while (totalWaitTime < stopWaitAfter)
            {
                test::sleepMs(waitTimeMs);
                totalWaitTime += waitTimeMs;

                if (test::wsystem(cmd) == 0)
                {
                    LOG(ERROR) << "Got response while expecting none. Outputting response:";
                    test::wsystem("cat " + lastResponseFileLocation, cmdResponse);
                    LOG(INFO) << cmdResponse;
                    break;
                }
            }

            test::wsystem("killall amidi > /dev/null 2>&1");
            return cmdResponse;
        }
    }

    // check for opendeck device only here
    static bool devicePresent(bool bootloader = false)
    {
        LOG(INFO) << "Checking if OpenDeck MIDI device is available";

        std::string port;

        if (bootloader)
            port = amidiPort(OPENDECK_DFU_MIDI_DEVICE_NAME);
        else
            port = amidiPort(OPENDECK_MIDI_DEVICE_NAME);

        if (port == "")
        {
            LOG(ERROR) << "OpenDeck MIDI device not available";
            return false;
        }

        std::string cmdResponse;

        if (test::wsystem("amidi -l | grep \"" + port + "\"", cmdResponse) == 0)
        {
            LOG(INFO) << "Device found";
            return true;
        }
        else
        {
            LOG(ERROR) << "Device not found";
            return false;
        }
    }

    static std::string amidiPort(std::string midiDevice)
    {
        std::string cmd = "amidi -l | grep \"" + midiDevice + "\" | grep -Eo 'hw:\\S*'";
        std::string cmdResponse;

        test::wsystem(cmd, cmdResponse);
        return test::trimNewline(cmdResponse);
    }
#endif

    private:
#ifdef HW_TESTS_SUPPORTED
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
            auto merged = Util::Conversion::Merge14bit(responseUint8.at(responseUint8.size() - 3), responseUint8.at(responseUint8.size() - 2));
            return merged.value();
        }
        else
        {
            // read status byte
            return responseUint8.at(4);
        }
    }
#endif

    static System::Config::block_t block(System::Config::Section::global_t section)
    {
        return System::Config::block_t::global;
    }

    static System::Config::block_t block(System::Config::Section::button_t section)
    {
        return System::Config::block_t::buttons;
    }

    static System::Config::block_t block(System::Config::Section::encoder_t section)
    {
        return System::Config::block_t::encoders;
    }

    static System::Config::block_t block(System::Config::Section::analog_t section)
    {
        return System::Config::block_t::analog;
    }

    static System::Config::block_t block(System::Config::Section::leds_t section)
    {
        return System::Config::block_t::leds;
    }

    static System::Config::block_t block(System::Config::Section::i2c_t section)
    {
        return System::Config::block_t::i2c;
    }

    static System::Config::block_t block(System::Config::Section::touchscreen_t section)
    {
        return System::Config::block_t::touchscreen;
    }
};