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
#include "stubs/System.h"
#include "system/Config.h"
#include <glog/logging.h>

#ifdef HW_TESTS_SUPPORTED
#include <HWTestDefines.h>
#endif

using namespace Protocol;

class MIDIHelper
{
    public:
    MIDIHelper()
        : USE_HARDWARE(false)
    {}

    MIDIHelper(bool useHardware)
        : USE_HARDWARE(useHardware)
    {}

    MIDIHelper(TestSystem& system)
        : _system(&system)
        , USE_HARDWARE(false)
    {}

    std::vector<MIDI::usbMIDIPacket_t> rawSysExToUSBPackets(std::vector<uint8_t>& raw)
    {
        Messaging::event_t event;
        event.sysEx       = &raw[0];
        event.sysExLength = raw.size();
        event.message     = MIDI::messageType_t::SYS_EX;

        return midiToUsbPackets(event);
    }

    std::vector<MIDI::usbMIDIPacket_t> midiToUsbPackets(Messaging::event_t event)
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

            bool write(MIDI::usbMIDIPacket_t& packet) override
            {
                _buffer.push_back(packet);
                return true;
            }

            std::vector<MIDI::usbMIDIPacket_t> _buffer;
        } hwaWriteToUSB;

        MIDIlib::USBMIDI writeToUsb(hwaWriteToUSB);
        writeToUsb.init();

        switch (event.message)
        {
        case MIDI::messageType_t::NOTE_OFF:
        {
            writeToUsb.sendNoteOff(event.index, event.value, event.channel);
        }
        break;

        case MIDI::messageType_t::NOTE_ON:
        {
            writeToUsb.sendNoteOff(event.index, event.value, event.channel);
        }
        break;

        case MIDI::messageType_t::CONTROL_CHANGE:
        {
            writeToUsb.sendControlChange(event.index, event.value, event.channel);
        }
        break;

        case MIDI::messageType_t::PROGRAM_CHANGE:
        {
            writeToUsb.sendProgramChange(event.index, event.channel);
        }
        break;

        case MIDI::messageType_t::AFTER_TOUCH_CHANNEL:
        {
            writeToUsb.sendAfterTouch(event.value, event.channel);
        }
        break;

        case MIDI::messageType_t::AFTER_TOUCH_POLY:
        {
            writeToUsb.sendAfterTouch(event.value, event.channel, event.index);
        }
        break;

        case MIDI::messageType_t::PITCH_BEND:
        {
            writeToUsb.sendPitchBend(event.value, event.channel);
        }
        break;

        case MIDI::messageType_t::SYS_REAL_TIME_CLOCK:
        {
            writeToUsb.sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::SYS_REAL_TIME_START:
        {
            writeToUsb.sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::SYS_REAL_TIME_CONTINUE:
        {
            writeToUsb.sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::SYS_REAL_TIME_STOP:
        {
            writeToUsb.sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::SYS_REAL_TIME_ACTIVE_SENSING:
        {
            writeToUsb.sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::SYS_REAL_TIME_SYSTEM_RESET:
        {
            writeToUsb.sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::MMC_PLAY:
        {
            writeToUsb.sendMMC(event.index, event.message);
        }
        break;

        case MIDI::messageType_t::MMC_STOP:
        {
            writeToUsb.sendMMC(event.index, event.message);
        }
        break;

        case MIDI::messageType_t::MMC_PAUSE:
        {
            writeToUsb.sendMMC(event.index, event.message);
        }
        break;

        case MIDI::messageType_t::MMC_RECORD_START:
        {
            writeToUsb.sendMMC(event.index, event.message);
        }
        break;

        case MIDI::messageType_t::MMC_RECORD_STOP:
        {
            writeToUsb.sendMMC(event.index, event.message);
        }
        break;

        case MIDI::messageType_t::NRPN_7BIT:
        {
            writeToUsb.sendNRPN(event.index, event.value, event.channel, false);
        }
        break;

        case MIDI::messageType_t::NRPN_14BIT:
        {
            writeToUsb.sendNRPN(event.index, event.value, event.channel, true);
        }
        break;

        case MIDI::messageType_t::CONTROL_CHANGE_14BIT:
        {
            writeToUsb.sendControlChange14bit(event.index, event.value, event.channel);
        }
        break;

        case MIDI::messageType_t::SYS_EX:
        {
            writeToUsb.sendSysEx(event.sysExLength, event.sysEx, true);
        }
        break;

        default:
            break;
        }

        return hwaWriteToUSB._buffer;
    }

    template<typename T>
    std::vector<uint8_t> generateSysExGetReq(T section, size_t index)
    {
        using namespace System;

        auto blockIndex = block(section);
        auto split      = Util::Conversion::Split14bit(index);

        std::vector<uint8_t> request = {
            0xF0,
            Config::SYSEX_MANUFACTURER_ID_0,
            Config::SYSEX_MANUFACTURER_ID_1,
            Config::SYSEX_MANUFACTURER_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),    // status
            0,                                                     // part
            static_cast<uint8_t>(SysExConf::wish_t::GET),          // wish
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),     // amount
            static_cast<uint8_t>(blockIndex),                      // block
            static_cast<uint8_t>(section),                         // section
            split.high(),                                          // index high byte
            split.low(),                                           // index low byte
            0x00,                                                  // new value high byte
            0x00,                                                  // new value low byte
            0xF7
        };

        return request;
    }

    template<typename S, typename I, typename V>
    std::vector<uint8_t> generateSysExSetReq(S section, I index, V value)
    {
        using namespace System;

        auto blockIndex = block(section);
        auto splitIndex = Util::Conversion::Split14bit(static_cast<uint16_t>(index));
        auto splitValue = Util::Conversion::Split14bit(static_cast<uint16_t>(value));

        std::vector<uint8_t> request = {
            0xF0,
            Config::SYSEX_MANUFACTURER_ID_0,
            Config::SYSEX_MANUFACTURER_ID_1,
            Config::SYSEX_MANUFACTURER_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            0,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            static_cast<uint8_t>(blockIndex),
            static_cast<uint8_t>(section),
            splitIndex.high(),
            splitIndex.low(),
            splitValue.high(),
            splitValue.low(),
            0xF7
        };

        return request;
    }

    template<typename T>
    uint16_t readFromSystem(T section, size_t index)
    {
        auto request = generateSysExGetReq(section, index);

#ifdef HW_TESTS_SUPPORTED
        if (USE_HARDWARE)
        {
            return sendRequestToDevice(request, SysExConf::wish_t::GET);
        }
#endif

        return sendRequestToStub(request, SysExConf::wish_t::GET);
    }

    template<typename S, typename I, typename V>
    bool writeToSystem(S section, I index, V value)
    {
        auto request = generateSysExSetReq(section, index, value);

#ifdef HW_TESTS_SUPPORTED
        if (USE_HARDWARE)
        {
            return sendRequestToDevice(request, SysExConf::wish_t::SET);
        }
#endif

        return sendRequestToStub(request, SysExConf::wish_t::SET);
    }

#ifdef HW_TESTS_SUPPORTED
    void flush()
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

    std::string sendRawSysEx(std::string req, bool expectResponse = true)
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

        static constexpr uint32_t WAIT_TIME_MS       = 10;
        static constexpr uint32_t STOP_WAIT_AFTER_MS = 2000;
        uint32_t                  totalWaitTime      = 0;

        if (expectResponse)
        {
            // change status byte to ack
            req[13] = '1';

            // remove everything after request/wish byte
            std::string pattern = req.substr(0, 20) + ".*F7";

            cmd = "cat " + lastResponseFileLocation + " | xargs | sed 's/F7/F7\\n/g' | sed 's/F0/\\nF0/g' | grep -m 1 -E '" + pattern + "'";

            while (test::wsystem(cmd, cmdResponse))
            {
                test::sleepMs(WAIT_TIME_MS);
                totalWaitTime += WAIT_TIME_MS;

                if (totalWaitTime == STOP_WAIT_AFTER_MS)
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

        cmd         = "[ -s " + lastResponseFileLocation + " ]";
        cmdResponse = "";

        while (totalWaitTime < STOP_WAIT_AFTER_MS)
        {
            test::sleepMs(WAIT_TIME_MS);
            totalWaitTime += WAIT_TIME_MS;

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

    bool devicePresent(bool silent = false)
    {
        if (!silent)
        {
            LOG(INFO) << "Checking if OpenDeck MIDI device is available";
        }

        auto port = amidiPort(OPENDECK_MIDI_DEVICE_NAME);

        if (port == "")
        {
            if (!silent)
            {
                LOG(ERROR) << "OpenDeck MIDI device not available";
            }

            return false;
        }

        std::string cmdResponse;

        if (test::wsystem("amidi -l | grep \"" + port + "\"", cmdResponse) == 0)
        {
            if (!silent)
            {
                LOG(INFO) << "Device found";
            }

            return true;
        }

        if (!silent)
        {
            LOG(ERROR) << "Device not found";
        }

        return false;
    }

    std::string amidiPort(std::string midiDevice)
    {
        std::string cmd = "amidi -l | grep \"" + midiDevice + "\" | grep -Eo 'hw:\\S*'";
        std::string cmdResponse;

        test::wsystem(cmd, cmdResponse);
        return test::trimNewline(cmdResponse);
    }
#endif

    std::vector<uint8_t> sendRawSysEx(std::vector<uint8_t> request)
    {
        LOG(INFO) << "Sending request to system: ";

        for (size_t i = 0; i < request.size(); i++)
        {
            std::cout
                << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
                << static_cast<int>(request.at(i)) << " ";
        }

        std::cout << std::endl;

        Messaging::event_t event;
        event.sysEx       = &request[0];
        event.sysExLength = request.size();
        event.message     = MIDI::messageType_t::SYS_EX;

        processIncoming(event);

        auto response     = _system->_hwaMIDIUSB._writeParser.writtenMessages().at(0).sysexArray;
        auto responseSize = _system->_hwaMIDIUSB._writeParser.writtenMessages().at(0).length;

        std::vector<uint8_t> responseVec(&response[0], &response[responseSize]);

        LOG(INFO) << "Received response: ";

        for (size_t i = 0; i < responseVec.size(); i++)
        {
            std::cout
                << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
                << static_cast<int>(responseVec.at(i)) << " ";
        }

        std::cout << std::endl;

        return responseVec;
    }

    void processIncoming(Messaging::event_t event)
    {
        if (_system == nullptr)
        {
            return;
        }

        LOG(INFO) << "Processing incoming messages to system";

        _system->_hwaMIDIUSB.clear();
        _system->_hwaMIDIDIN.clear();

        _system->_hwaMIDIUSB._readPackets = midiToUsbPackets(event);

        // don't care about hwa calls here
        EXPECT_CALL(_system->_hwaButtons, state(_, _, _))
            .Times(AnyNumber());

        EXPECT_CALL(_system->_hwaAnalog, value(_, _))
            .Times(AnyNumber());

        // now just call system which will call midi.read which in turn will read the filled packets
        while (_system->_hwaMIDIUSB._readPackets.size())
        {
            _system->_instance.run();
        }
    }

    private:
#ifdef HW_TESTS_SUPPORTED
    uint16_t sendRequestToDevice(std::vector<uint8_t>& requestUint8, SysExConf::wish_t wish)
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
            {
                requestString << " ";
            }
        }

        std::string responseString = sendRawSysEx(requestString.str());

        if (responseString == "")
        {
            return 0;    // invalid response
        }

        // convert response back to uint8 vector
        std::vector<uint8_t> responseUint8;

        for (size_t i = 0; i < responseString.length(); i += 3)
        {
            std::string byteString = responseString.substr(i, 2);
            char        byte       = (char)strtol(byteString.c_str(), NULL, 16);
            responseUint8.push_back(byte);
        }

        if (wish == SysExConf::wish_t::GET)
        {
            // last two bytes are result
            auto merged = Util::Conversion::Merge14bit(responseUint8.at(responseUint8.size() - 3), responseUint8.at(responseUint8.size() - 2));
            return merged.value();
        }

        // read status byte
        return responseUint8.at(4);
    }
#endif

    uint16_t sendRequestToStub(std::vector<uint8_t>& request, SysExConf::wish_t wish, bool customReq = false)
    {
        auto response = sendRawSysEx(request);

        if (wish == SysExConf::wish_t::GET)
        {
            // last two bytes are result
            auto merged = Util::Conversion::Merge14bit(response.at(response.size() - 3), response.at(response.size() - 2));
            return merged.value();
        }

        // read status byte
        return response.at(4);
    }

    System::Config::block_t block(System::Config::Section::global_t section)
    {
        return System::Config::block_t::GLOBAL;
    }

    System::Config::block_t block(System::Config::Section::button_t section)
    {
        return System::Config::block_t::BUTTONS;
    }

    System::Config::block_t block(System::Config::Section::encoder_t section)
    {
        return System::Config::block_t::ENCODERS;
    }

    System::Config::block_t block(System::Config::Section::analog_t section)
    {
        return System::Config::block_t::ANALOG;
    }

    System::Config::block_t block(System::Config::Section::leds_t section)
    {
        return System::Config::block_t::LEDS;
    }

    System::Config::block_t block(System::Config::Section::i2c_t section)
    {
        return System::Config::block_t::I2C;
    }

    System::Config::block_t block(System::Config::Section::touchscreen_t section)
    {
        return System::Config::block_t::TOUCHSCREEN;
    }

    TestSystem* _system = nullptr;

    [[maybe_unused]] const bool USE_HARDWARE;
};