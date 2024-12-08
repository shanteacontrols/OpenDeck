/*

Copyright Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#pragma once

#include "misc.h"
#include "application/system/builder_test.h"
#include "application/protocol/midi/midi.h"
#include "lib/sysexconf/sysexconf.h"
#include "application/util/conversion/conversion.h"
#include "application/system/system.h"
#include "application/system/config.h"

#ifdef PROJECT_TARGET_HW_TESTS_SUPPORTED
#include <hw_test_defines.h>
#endif

#include <glog/logging.h>

#include <cinttypes>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>

using namespace protocol;

namespace test
{
    class MIDIHelper
    {
        public:
        MIDIHelper()
            : USE_HARDWARE(false)
        {}

        MIDIHelper(bool useHardware)
            : USE_HARDWARE(useHardware)
        {}

        MIDIHelper(sys::BuilderTest& system)
            : _system(&system)
            , USE_HARDWARE(false)
        {}

        std::vector<midi::UsbPacket> rawSysExToUSBPackets(std::vector<uint8_t>& raw)
        {
            messaging::Event event = {};
            event.sysEx            = &raw[0];
            event.sysExLength      = raw.size();
            event.message          = midi::messageType_t::SYS_EX;

            return midiToUsbPackets(event);
        }

        std::vector<midi::UsbPacket> midiToUsbPackets(messaging::Event event)
        {
            class HWAWriteToUSB : public lib::midi::usb::Hwa
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

                bool read(midi::UsbPacket& packet) override
                {
                    return false;
                }

                bool write(midi::UsbPacket& packet) override
                {
                    _buffer.push_back(packet);
                    return true;
                }

                std::vector<midi::UsbPacket> _buffer;
            } hwaWriteToUSB;

            lib::midi::usb::Usb writeToUsb(hwaWriteToUSB);
            writeToUsb.init();

            switch (event.message)
            {
            case midi::messageType_t::NOTE_OFF:
            {
                writeToUsb.sendNoteOff(event.index, event.value, event.channel);
            }
            break;

            case midi::messageType_t::NOTE_ON:
            {
                writeToUsb.sendNoteOff(event.index, event.value, event.channel);
            }
            break;

            case midi::messageType_t::CONTROL_CHANGE:
            {
                writeToUsb.sendControlChange(event.index, event.value, event.channel);
            }
            break;

            case midi::messageType_t::PROGRAM_CHANGE:
            {
                writeToUsb.sendProgramChange(event.index, event.channel);
            }
            break;

            case midi::messageType_t::AFTER_TOUCH_CHANNEL:
            {
                writeToUsb.sendAfterTouch(event.value, event.channel);
            }
            break;

            case midi::messageType_t::AFTER_TOUCH_POLY:
            {
                writeToUsb.sendAfterTouch(event.value, event.channel, event.index);
            }
            break;

            case midi::messageType_t::PITCH_BEND:
            {
                writeToUsb.sendPitchBend(event.value, event.channel);
            }
            break;

            case midi::messageType_t::SYS_REAL_TIME_CLOCK:
            {
                writeToUsb.sendRealTime(event.message);
            }
            break;

            case midi::messageType_t::SYS_REAL_TIME_START:
            {
                writeToUsb.sendRealTime(event.message);
            }
            break;

            case midi::messageType_t::SYS_REAL_TIME_CONTINUE:
            {
                writeToUsb.sendRealTime(event.message);
            }
            break;

            case midi::messageType_t::SYS_REAL_TIME_STOP:
            {
                writeToUsb.sendRealTime(event.message);
            }
            break;

            case midi::messageType_t::SYS_REAL_TIME_ACTIVE_SENSING:
            {
                writeToUsb.sendRealTime(event.message);
            }
            break;

            case midi::messageType_t::SYS_REAL_TIME_SYSTEM_RESET:
            {
                writeToUsb.sendRealTime(event.message);
            }
            break;

            case midi::messageType_t::MMC_PLAY:
            {
                writeToUsb.sendMMC(event.index, event.message);
            }
            break;

            case midi::messageType_t::MMC_STOP:
            {
                writeToUsb.sendMMC(event.index, event.message);
            }
            break;

            case midi::messageType_t::MMC_PAUSE:
            {
                writeToUsb.sendMMC(event.index, event.message);
            }
            break;

            case midi::messageType_t::MMC_RECORD_START:
            {
                writeToUsb.sendMMC(event.index, event.message);
            }
            break;

            case midi::messageType_t::MMC_RECORD_STOP:
            {
                writeToUsb.sendMMC(event.index, event.message);
            }
            break;

            case midi::messageType_t::NRPN_7BIT:
            {
                writeToUsb.sendNRPN(event.index, event.value, event.channel, false);
            }
            break;

            case midi::messageType_t::NRPN_14BIT:
            {
                writeToUsb.sendNRPN(event.index, event.value, event.channel, true);
            }
            break;

            case midi::messageType_t::CONTROL_CHANGE_14BIT:
            {
                writeToUsb.sendControlChange14bit(event.index, event.value, event.channel);
            }
            break;

            case midi::messageType_t::SYS_EX:
            {
                writeToUsb.sendSysEx(event.sysExLength, event.sysEx, true);
            }
            break;

            default:
                break;
            }

            return hwaWriteToUSB._buffer;
        }

        template<typename S, typename I>
        std::vector<uint8_t> generateSysExGetReq(S section, I index)
        {
            using namespace sys;

            auto blockIndex = BLOCK(section);
            auto split      = util::Conversion::Split14Bit(static_cast<size_t>(index));

            std::vector<uint8_t> request = {
                0xF0,
                Config::SYSEX_MANUFACTURER_ID_0,
                Config::SYSEX_MANUFACTURER_ID_1,
                Config::SYSEX_MANUFACTURER_ID_2,
                static_cast<uint8_t>(lib::sysexconf::status_t::REQUEST),    // status
                0,                                                          // part
                static_cast<uint8_t>(lib::sysexconf::wish_t::GET),          // wish
                static_cast<uint8_t>(lib::sysexconf::amount_t::SINGLE),     // amount
                static_cast<uint8_t>(blockIndex),                           // block
                static_cast<uint8_t>(section),                              // section
                split.high(),                                               // index high byte
                split.low(),                                                // index low byte
                0x00,                                                       // new value high byte
                0x00,                                                       // new value low byte
                0xF7
            };

            return request;
        }

        template<typename S, typename I, typename V>
        std::vector<uint8_t> generateSysExSetReq(S section, I index, V value)
        {
            using namespace sys;

            auto blockIndex = BLOCK(section);
            auto splitIndex = util::Conversion::Split14Bit(static_cast<uint16_t>(index));
            auto splitValue = util::Conversion::Split14Bit(static_cast<uint16_t>(value));

            std::vector<uint8_t> request = {
                0xF0,
                Config::SYSEX_MANUFACTURER_ID_0,
                Config::SYSEX_MANUFACTURER_ID_1,
                Config::SYSEX_MANUFACTURER_ID_2,
                static_cast<uint8_t>(lib::sysexconf::status_t::REQUEST),
                0,
                static_cast<uint8_t>(lib::sysexconf::wish_t::SET),
                static_cast<uint8_t>(lib::sysexconf::amount_t::SINGLE),
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

        template<typename S, typename I>
        uint16_t databaseReadFromSystemViaSysEx(S section, I index)
        {
            auto request = generateSysExGetReq(section, index);

#ifdef PROJECT_TARGET_HW_TESTS_SUPPORTED
            if (USE_HARDWARE)
            {
                return sendRequestToDevice(request, lib::sysexconf::wish_t::GET);
            }
            else
            {
                return sendRequestToStub(request, lib::sysexconf::wish_t::GET);
            }
#else
            return sendRequestToStub(request, lib::sysexconf::wish_t::GET);
#endif
        }

        template<typename S, typename I, typename V>
        bool databaseWriteToSystemViaSysEx(S section, I index, V value)
        {
            auto request = generateSysExSetReq(section, index, value);

#ifdef PROJECT_TARGET_HW_TESTS_SUPPORTED
            if (USE_HARDWARE)
            {
                return sendRequestToDevice(request, lib::sysexconf::wish_t::SET);
            }
            else
            {
                return sendRequestToStub(request, lib::sysexconf::wish_t::SET);
            }
#else
            return sendRequestToStub(request, lib::sysexconf::wish_t::SET);
#endif
        }

#ifdef PROJECT_TARGET_HW_TESTS_SUPPORTED
        static void flush()
        {
            LOG(INFO) << "Flushing all incoming data from the OpenDeck device";
            std::string cmdResponse;

            std::string cmd = std::string("amidi -p ") + amidiPort(HW_TEST_USB_DEVICE_NAME_APP) + std::string(" -d -t 3");
            test::wsystem(cmd, cmdResponse);

// do the same for din interface if present
#ifdef HW_TEST_DIN_MIDI_SUPPORTED
            LOG(INFO) << "Flushing all incoming data from the DIN MIDI interface on device";
            cmd = std::string("amidi -p ") + amidiPort(HW_TEST_DIN_MIDI_OUT_PORT) + std::string(" -d -t 3");
            test::wsystem(cmd, cmdResponse);
#endif
        }

        static std::vector<uint8_t> sendRawSysExToDevice(std::vector<uint8_t> request, bool expectResponse = true)
        {
            std::vector<uint8_t> ret;
            std::string          cmdResponse;
            std::string          lastResponseFileLocation = "/tmp/midi_in_data.txt";
            auto                 hexRequest               = test::vectorToHexString(request);

            test::wsystem("rm -f " + lastResponseFileLocation);
            LOG(INFO) << "req: " << hexRequest;

            std::string cmd = std::string("stdbuf -i0 -o0 -e0 amidi -p ") + amidiPort(HW_TEST_USB_DEVICE_NAME_APP) + std::string(" -S '") + hexRequest + "' -d | stdbuf -i0 -o0 -e0 tr -d '\\n' > " + lastResponseFileLocation + " &";
            test::wsystem(cmd, cmdResponse);

            static constexpr uint32_t STOP_WAIT_AFTER_MS = 3000;

            if (expectResponse)
            {
                // parse the response with midi library
                // convert the response to u8 vector first
                protocol::midi::HwaSerialTest _hwaMidiSerial;

                cmd                                  = "cat " + lastResponseFileLocation + " | tr -d '[:space:]'";
                std::vector<uint8_t> responsePattern = { 0xF0, 0x00, 0x53, 0x43, 0x01 };

                // copy message part and wish to pattern as well
                responsePattern.push_back(request.at(5));
                responsePattern.push_back(request.at(6));

                auto startTime = test::millis();

                while (true)
                {
                    auto check = [&]()
                    {
                        bool matched = false;
                        test::wsystem(cmd, cmdResponse);

                        auto packetVector = [&]()
                        {
                            auto                                   vector  = test::hexStringToVector(test::trimWhitespace(cmdResponse));
                            std::vector<lib::midi::serial::Packet> packets = {};

                            for (size_t i = 0; i < vector.size(); i++)
                            {
                                packets.push_back(lib::midi::serial::Packet{ vector.at(i) });
                            }

                            return packets;
                        };

                        _hwaMidiSerial._readPackets       = packetVector();
                        lib::midi::serial::Serial _serial = lib::midi::serial::Serial(_hwaMidiSerial);

                        while (_hwaMidiSerial._readPackets.size())
                        {
                            if (!_serial.read())
                            {
                                continue;
                            }

                            if (_serial.type() == lib::midi::messageType_t::SYS_EX)
                            {
                                std::vector<uint8_t> sysex(_serial.sysExArray(), _serial.sysExArray() + _serial.length());

                                // verify if it matches pattern
                                if (sysex.size() > responsePattern.size())
                                {
                                    bool patternMatch = true;

                                    for (size_t i = 0; i < responsePattern.size(); i++)
                                    {
                                        if (sysex.at(i) != responsePattern.at(i))
                                        {
                                            patternMatch = false;
                                            break;
                                        }
                                    }

                                    if (patternMatch)
                                    {
                                        ret     = sysex;
                                        matched = true;
                                        break;
                                    }
                                }
                            }
                        }

                        return matched;
                    };

                    if ((test::millis() - startTime) > STOP_WAIT_AFTER_MS)
                    {
                        LOG(ERROR)
                            << "Failed to find valid response to request. Printing raw response:\n"
                            << cmdResponse;

                        break;
                    }
                    else if (check())
                    {
                        break;
                    }
                }
            }

            test::wsystem("killall amidi > /dev/null 2>&1");
            LOG(INFO) << "res: " << test::vectorToHexString(ret);
            return ret;
        }

        static bool devicePresent(bool silent, bool bootloader = false)
        {
            if (!silent)
            {
                LOG(INFO) << "Checking if OpenDeck MIDI device is available";
            }

            auto port = bootloader ? amidiPort(HW_TEST_USB_DEVICE_NAME_BOOT) : amidiPort(HW_TEST_USB_DEVICE_NAME_APP);

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

        static std::string amidiPort(std::string midiDevice)
        {
            std::string cmd = "amidi -l | grep \"" + midiDevice + "\" | grep -Eo 'hw:\\S*'";
            std::string cmdResponse;

            test::wsystem(cmd, cmdResponse);
            return test::trimWhitespace(cmdResponse);
        }
#endif

        std::vector<uint8_t> sendRawSysExToStub(std::vector<uint8_t> request)
        {
            LOG(INFO) << "Sending request to system: ";

            for (size_t i = 0; i < request.size(); i++)
            {
                std::cout
                    << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
                    << static_cast<int>(request.at(i)) << " ";
            }

            std::cout << std::endl;

            messaging::Event event = {};
            event.sysEx            = &request[0];
            event.sysExLength      = request.size();
            event.message          = midi::messageType_t::SYS_EX;

            processIncoming(event);

            auto response     = _system->_components._builderMidi._hwaUsb._writeParser.writtenMessages().at(0).sysexArray;
            auto responseSize = _system->_components._builderMidi._hwaUsb._writeParser.writtenMessages().at(0).length;

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

        void processIncoming(messaging::Event event)
        {
            if (_system == nullptr)
            {
                return;
            }

            LOG(INFO) << "Processing incoming messages to system";

            _system->_components._builderMidi._hwaUsb.clear();
            _system->_components._builderMidi._hwaSerial.clear();

            _system->_components._builderMidi._hwaUsb._readPackets = midiToUsbPackets(event);

            // don't care about hwa calls here
            EXPECT_CALL(_system->_components._builderButtons._hwa, state(_, _, _))
                .Times(AnyNumber());

            EXPECT_CALL(_system->_components._builderAnalog._hwa, value(_, _))
                .Times(AnyNumber());

            // now just call system which will call midi.read which in turn will read the filled packets
            while (_system->_components._builderMidi._hwaUsb._readPackets.size())
            {
                _system->_instance.run();
            }
        }

        size_t totalChannelMessages(const std::string& input)
        {
            LOG(INFO) << "Checking total number of channel messages for the following input:\n"
                      << input;

            auto packetVector = [&]()
            {
                // for a given input, extract all channel midi messages and return total count
                auto                                   vector  = test::hexStringToVector(test::trimWhitespace(input));
                std::vector<lib::midi::serial::Packet> packets = {};

                for (size_t i = 0; i < vector.size(); i++)
                {
                    packets.push_back(lib::midi::serial::Packet{ vector.at(i) });
                }

                return packets;
            };

            // now count total number of channel messages
            size_t                        totalChannelMessages = 0;
            protocol::midi::HwaSerialTest _hwaMidiSerial;
            protocol::midi::Serial        _serial = protocol::midi::Serial(_hwaMidiSerial);
            _hwaMidiSerial._readPackets           = packetVector();

            while (_hwaMidiSerial._readPackets.size())
            {
                if (!_serial.read())
                {
                    continue;
                }

                if (lib::midi::IS_CHANNEL_MESSAGE(_serial.type()))
                {
                    totalChannelMessages++;
                }
            }

            return totalChannelMessages;
        }

        private:
#ifdef PROJECT_TARGET_HW_TESTS_SUPPORTED
        int32_t sendRequestToDevice(std::vector<uint8_t>& request, lib::sysexconf::wish_t wish)
        {
            auto response = sendRawSysExToDevice(request);

            if (response.empty())
            {
                return -1;    // invalid response
            }

            if (wish == lib::sysexconf::wish_t::GET)
            {
                // last two bytes are result
                auto merged = util::Conversion::Merge14Bit(response.at(response.size() - 3), response.at(response.size() - 2));
                return merged.value();
            }

            // read status byte
            return response.at(4);
        }
#endif

        uint16_t sendRequestToStub(std::vector<uint8_t>& request, lib::sysexconf::wish_t wish, bool customReq = false)
        {
            auto response = sendRawSysExToStub(request);

            if (wish == lib::sysexconf::wish_t::GET)
            {
                // last two bytes are result
                auto merged = util::Conversion::Merge14Bit(response.at(response.size() - 3), response.at(response.size() - 2));
                return merged.value();
            }

            // read status byte
            return response.at(4);
        }

        static constexpr sys::Config::block_t BLOCK(sys::Config::Section::global_t section)
        {
            return sys::Config::block_t::GLOBAL;
        }

        static constexpr sys::Config::block_t BLOCK(sys::Config::Section::button_t section)
        {
            return sys::Config::block_t::BUTTONS;
        }

        static constexpr sys::Config::block_t BLOCK(sys::Config::Section::encoder_t section)
        {
            return sys::Config::block_t::ENCODERS;
        }

        static constexpr sys::Config::block_t BLOCK(sys::Config::Section::analog_t section)
        {
            return sys::Config::block_t::ANALOG;
        }

        static constexpr sys::Config::block_t BLOCK(sys::Config::Section::leds_t section)
        {
            return sys::Config::block_t::LEDS;
        }

        static constexpr sys::Config::block_t BLOCK(sys::Config::Section::i2c_t section)
        {
            return sys::Config::block_t::I2C;
        }

        static constexpr sys::Config::block_t BLOCK(sys::Config::Section::touchscreen_t section)
        {
            return sys::Config::block_t::TOUCHSCREEN;
        }

        sys::BuilderTest* _system = nullptr;

        [[maybe_unused]] const bool USE_HARDWARE;
    };
}    // namespace test
