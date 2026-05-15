/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "tests/helpers/misc.h"
#include "firmware/src/system/builder.h"
#include "firmware/src/system/config.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/util/conversion/conversion.h"
#include "firmware/src/protocol/midi/common.h"
#include "firmware/src/protocol/midi/midi.h"

#include "zlibs/utils/midi/midi_common.h"

#include <zephyr/kernel.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>
#include <span>
#include <string>
#include <vector>

namespace opendeck::tests
{
    /**
     * @brief Describes one MIDI event fed into the test helper.
     */
    struct MIDIInputEvent
    {
        uint8_t                     channel       = 0;
        uint16_t                    index         = 0;
        uint16_t                    value         = 0;
        protocol::midi::MessageType message       = protocol::midi::MessageType::Invalid;
        const uint8_t*              sys_ex        = nullptr;
        size_t                      sys_ex_length = 0;
    };

    /**
     * @brief High-level helper used by tests to interact with firmware MIDI and SysEx flows.
     */
    class MIDIHelper
    {
        public:
        /** @brief Expected USB MIDI device name used by hardware-backed tests. */
        static constexpr const char* USB_MIDI_DEVICE_NAME = "Group 1 (OpenDeck)";
        /** @brief Startup delay used after launching MIDI receive helpers. */
        static constexpr uint32_t RECEIVE_STARTUP_DELAY_MS = 100;
        /** @brief Timeout used while waiting for `receivemidi` to connect. */
        static constexpr uint32_t RECEIVE_CONNECT_TIMEOUT_MS = 1000;
        /** @brief Delay used to flush pending MIDI traffic. */
        static constexpr uint32_t RECEIVE_FLUSH_DELAY_MS = 100;
        /** @brief Poll delay used while waiting for helper-process output. */
        static constexpr uint32_t RECEIVE_POLL_DELAY_MS = 10;
        /** @brief Stability window used to decide when streamed responses are complete. */
        static constexpr uint32_t RECEIVE_STABLE_TIME_MS = 60;
        /** @brief Delay used after write-only hardware requests. */
        static constexpr uint32_t WRITE_SETTLE_DELAY_MS = 15;
        /** @brief Retry delay used by read-side helper loops. */
        static constexpr uint32_t READ_RETRY_DELAY_MS = 20;
        /** @brief Delay used before reading from a newly started MIDI receive helper. */
        static constexpr uint32_t RECEIVE_SETTLE_DELAY_MS = 100;
        /** @brief Timeout used while collecting streamed stub responses. */
        static constexpr uint32_t STREAM_RESPONSE_TIMEOUT_MS = 5000;

        /**
         * @brief Constructs the helper for hardware-backed or stub-backed operation.
         *
         * @param use_hardware `true` to target a real device when supported, otherwise `false`.
         */
        explicit MIDIHelper(bool use_hardware)
            : _use_hardware(use_hardware)
        {}

        /**
         * @brief Constructs the helper around a system-test builder.
         *
         * @param system System test builder used for stub-backed interaction.
         */
        explicit MIDIHelper(sys::Builder& system)
            : _system(&system)
            , _use_hardware(false)
        {}

        /**
         * @brief Converts one logical MIDI input event into USB UMP packets.
         *
         * @param event Event to convert.
         *
         * @return Generated UMP packet sequence.
         */
        std::vector<midi_ump> midi_to_usb_packets(const MIDIInputEvent& event)
        {
            std::vector<midi_ump> packets;

            switch (event.message)
            {
            case protocol::midi::MessageType::NoteOff:
            {
                packets.push_back(zlibs::utils::midi::midi1::note_off(0, event.channel - 1, event.index, event.value));
            }
            break;

            case protocol::midi::MessageType::NoteOn:
            {
                packets.push_back(zlibs::utils::midi::midi1::note_on(0, event.channel - 1, event.index, event.value));
            }
            break;

            case protocol::midi::MessageType::ControlChange:
            {
                packets.push_back(zlibs::utils::midi::midi1::control_change(0, event.channel - 1, event.index, event.value));
            }
            break;

            case protocol::midi::MessageType::ProgramChange:
            {
                packets.push_back(zlibs::utils::midi::midi1::program_change(0, event.channel - 1, event.index));
            }
            break;

            case protocol::midi::MessageType::SysEx:
            {
                size_t index = 0;

                while (index < event.sys_ex_length)
                {
                    if (event.sys_ex[index] != zlibs::utils::midi::SYS_EX_START)
                    {
                        break;
                    }

                    size_t frame_end = index + 1;

                    while ((frame_end < event.sys_ex_length) && (event.sys_ex[frame_end] != zlibs::utils::midi::SYS_EX_END))
                    {
                        frame_end++;
                    }

                    if (frame_end >= event.sys_ex_length)
                    {
                        break;
                    }

                    const size_t frame_size = frame_end - index + 1;

                    if (frame_size >= 2)
                    {
                        const auto payload = std::span<const uint8_t>(event.sys_ex + index + 1, frame_size - 2);

                        zlibs::utils::midi::write_sysex7_payload_as_ump_packets(
                            zlibs::utils::midi::DEFAULT_RX_GROUP,
                            payload,
                            [&packets](const midi_ump& packet)
                            {
                                packets.push_back(packet);
                                return true;
                            });
                    }

                    index += frame_size;
                }
            }
            break;

            default:
                break;
            }

            return packets;
        }

        /**
         * @brief Builds a SysEx configuration read request.
         *
         * @tparam S Section enum type.
         * @tparam I Index type.
         *
         * @param section Section to read.
         * @param index Item index within the section.
         *
         * @return Serialized SysEx read request.
         */
        template<typename S, typename I>
        std::vector<uint8_t> generate_sysex_get_req(S section, I index)
        {
            const auto block_index = block(section);
            const auto split       = util::Conversion::Split14Bit(static_cast<size_t>(index));

            return {
                0xF0,
                sys::Config::SYSEX_MANUFACTURER_ID_0,
                sys::Config::SYSEX_MANUFACTURER_ID_1,
                sys::Config::SYSEX_MANUFACTURER_ID_2,
                0x00,
                0x00,
                0x00,
                0x00,
                static_cast<uint8_t>(block_index),
                static_cast<uint8_t>(section),
                split.high(),
                split.low(),
                0x00,
                0x00,
                0xF7
            };
        }

        /**
         * @brief Builds a SysEx configuration write request.
         *
         * @tparam S Section enum type.
         * @tparam I Index type.
         * @tparam V Value type.
         *
         * @param section Section to update.
         * @param index Item index within the section.
         * @param value Value to write.
         *
         * @return Serialized SysEx write request.
         */
        template<typename S, typename I, typename V>
        std::vector<uint8_t> generate_sysex_set_req(S section, I index, V value)
        {
            const auto block_index = block(section);
            const auto split_index = util::Conversion::Split14Bit(static_cast<uint16_t>(index));
            const auto split_value = util::Conversion::Split14Bit(static_cast<uint16_t>(value));

            return {
                0xF0,
                sys::Config::SYSEX_MANUFACTURER_ID_0,
                sys::Config::SYSEX_MANUFACTURER_ID_1,
                sys::Config::SYSEX_MANUFACTURER_ID_2,
                0x00,
                0x00,
                0x01,
                0x00,
                static_cast<uint8_t>(block_index),
                static_cast<uint8_t>(section),
                split_index.high(),
                split_index.low(),
                split_value.high(),
                split_value.low(),
                0xF7
            };
        }

        /**
         * @brief Reads one configuration value through the SysEx test path.
         *
         * @tparam S Section enum type.
         * @tparam I Index type.
         *
         * @param section Section to read.
         * @param index Item index within the section.
         *
         * @return Decoded configuration value.
         */
        template<typename S, typename I>
        uint16_t database_read_from_system_via_sysex(S section, I index)
        {
            auto request = generate_sysex_get_req(section, index);

#ifdef PROJECT_TARGET_HW_TESTS_SUPPORTED
            if (_use_hardware)
            {
                return send_request_to_device(request, true);
            }
#endif

            return send_request_to_stub(request, true);
        }

        /**
         * @brief Writes one configuration value through the SysEx test path.
         *
         * @tparam S Section enum type.
         * @tparam I Index type.
         * @tparam V Value type.
         *
         * @param section Section to update.
         * @param index Item index within the section.
         * @param value Value to write.
         *
         * @return `true` if the write was acknowledged, otherwise `false`.
         */
        template<typename S, typename I, typename V>
        bool database_write_to_system_via_sysex(S section, I index, V value)
        {
            auto request = generate_sysex_set_req(section, index, value);

#ifdef PROJECT_TARGET_HW_TESTS_SUPPORTED
            if (_use_hardware)
            {
                return send_write_request_to_device(request);
            }
#endif

            return send_request_to_stub(request, false) == static_cast<uint16_t>(sys::Config::Status::Ack);
        }

#ifdef PROJECT_TARGET_HW_TESTS_SUPPORTED
        /**
         * @brief Sends a SysEx request to the device and captures every matching response frame.
         *
         * @param request Serialized SysEx request.
         * @param results Filled with every parsed response frame.
         * @param timeout_ms Maximum time to wait.
         *
         * @return `true` if at least one response was captured, otherwise `false`.
         */
        static bool capture_sysex_responses(const std::vector<uint8_t>&        request,
                                            std::vector<std::vector<uint8_t>>& results,
                                            uint32_t                           timeout_ms)
        {
            auto       output_file = temporary_file_path("capture_sysex_responses");
            const auto pid         = start_receive_midi(output_file);
            sleep_ms(RECEIVE_SETTLE_DELAY_MS);

            if (pid <= 0)
            {
                return false;
            }

            if (!send_midi(send_midi_payload(request)))
            {
                stop_receive_midi(pid);
                return false;
            }

            auto output = wait_for_receive_midi_stream(output_file, timeout_ms);
            stop_receive_midi(pid);

            if (output.empty())
            {
                return false;
            }

            results = parse_receive_midi_outputs(output);

            if (results.empty())
            {
                return false;
            }

            return true;
        }

        /**
         * @brief Captures textual `receivemidi` output for voice messages after a request.
         *
         * @param request Serialized SysEx request to send first.
         * @param timeout_ms Maximum capture duration.
         *
         * @return Raw `receivemidi` output.
         */
        static std::string capture_midi_voice_messages_dump_after_request(const std::vector<uint8_t>& request, uint32_t timeout_ms = 1000)
        {
            const auto output_file = temporary_file_path("receivemidi-dump");
            const auto pid         = start_receive_midi(output_file, "voice");
            sleep_ms(RECEIVE_SETTLE_DELAY_MS);

            if (pid <= 0)
            {
                return "";
            }

            if (!send_midi(send_midi_payload(request)))
            {
                stop_receive_midi(pid);
                return "";
            }

            const auto output = wait_for_receive_midi_stream(output_file, timeout_ms);
            stop_receive_midi(pid);
            return output;
        }

        /**
         * @brief Flushes pending traffic from the host MIDI input helper.
         */
        static void flush()
        {
            const auto temp_file = temporary_file_path("receivemidi-flush");
            const auto pid       = start_receive_midi(temp_file);

            if (pid > 0)
            {
                if (wait_for_receive_midi_ready(temp_file))
                {
                    sleep_ms(RECEIVE_FLUSH_DELAY_MS);
                }

                stop_receive_midi(pid);
            }

            std::remove(temp_file.c_str());
        }

        /**
         * @brief Sends a raw SysEx message to a real device and optionally waits for a response.
         *
         * @param request Serialized SysEx request.
         * @param expect_response `true` to wait for a response, otherwise `false`.
         *
         * @return Received response frame, or an empty vector on failure.
         */
        static std::vector<uint8_t> send_raw_sysex_to_device(std::vector<uint8_t> request, bool expect_response = true)
        {
            std::vector<uint8_t> response;

            std::cout << "req: " << tests::vector_to_hex_string(request) << std::endl;

            if (expect_response)
            {
                const auto temp_file = temporary_file_path("receivemidi-response");
                const auto pid       = start_receive_midi(temp_file);

                if (pid <= 0)
                {
                    return response;
                }

                sleep_ms(RECEIVE_STARTUP_DELAY_MS);

                if (send_midi(send_midi_payload(request)))
                {
                    response = wait_for_receive_midi_response(temp_file, request, 3000);
                }

                stop_receive_midi(pid);
                std::remove(temp_file.c_str());
            }
            else
            {
                [[maybe_unused]] auto ret = send_midi(send_midi_payload(request));
            }

            std::cout << "res: " << tests::vector_to_hex_string(response) << std::endl;
            return response;
        }

        /**
         * @brief Returns whether the expected hardware MIDI device is present.
         *
         * @param silent `true` to suppress status logging, otherwise `false`.
         * @param bootloader Unused bootloader selector kept for compatibility.
         *
         * @return `true` if the device is present, otherwise `false`.
         */
        static bool device_present(bool silent, [[maybe_unused]] bool bootloader = false)
        {
            if (!silent)
            {
                std::cout << "Checking if OpenDeck MIDI device is available" << std::endl;
            }

            std::string response;
            const auto  result = tests::wsystem("sendmidi list | grep -F \"" + std::string(USB_MIDI_DEVICE_NAME) + "\"",
                                                response);

            if (result == 0)
            {
                if (!silent)
                {
                    std::cout << "Device found" << std::endl;
                }

                return true;
            }

            if (!silent)
            {
                std::cerr << "Device not found" << std::endl;
            }

            return false;
        }
#endif

        /**
         * @brief Sends a raw SysEx request through the system stub and returns the first response.
         *
         * @param request Serialized SysEx request.
         *
         * @return First response frame, or an empty vector when none was produced.
         */
        std::vector<uint8_t> send_raw_sysex_to_stub(std::vector<uint8_t> request)
        {
            MIDIInputEvent event = {};
            event.sys_ex         = request.data();
            event.sys_ex_length  = request.size();
            event.message        = protocol::midi::MessageType::SysEx;

            process_incoming(event);

            constexpr uint32_t   MAX_WAIT_MS = 250;
            std::vector<uint8_t> response;

            for (uint32_t elapsed = 0; elapsed < MAX_WAIT_MS; elapsed++)
            {
                auto responses = collect_stub_sysex_responses();

                if (!responses.empty())
                {
                    response = responses.front();
                    break;
                }

                k_msleep(1);
            }

            if (response.empty())
            {
                std::cerr << "No SysEx response received" << std::endl;
                return response;
            }

            return response;
        }

        /**
         * @brief Sends a raw SysEx request through the system stub and returns the full response stream.
         *
         * @param request Serialized SysEx request.
         *
         * @return All response frames captured from the stub.
         */
        std::vector<std::vector<uint8_t>> send_raw_sysex_stream_to_stub(std::vector<uint8_t> request)
        {
            std::cout << "Sending request to system: ";

            for (const auto byte : request)
            {
                std::cout << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
                          << static_cast<int>(byte) << " ";
            }

            std::cout << std::dec << std::endl;

            MIDIInputEvent event = {};
            event.sys_ex         = request.data();
            event.sys_ex_length  = request.size();
            event.message        = protocol::midi::MessageType::SysEx;

            process_incoming(event);

            std::vector<std::vector<uint8_t>> responses;
            size_t                            last_count    = 0;
            uint32_t                          stable_for_ms = 0;

            for (uint32_t elapsed = 0; elapsed < STREAM_RESPONSE_TIMEOUT_MS; elapsed++)
            {
                responses = collect_stub_sysex_responses();

                if (responses.size() == last_count)
                {
                    stable_for_ms++;

                    if (!responses.empty() && stable_for_ms >= RECEIVE_STABLE_TIME_MS)
                    {
                        break;
                    }
                }
                else
                {
                    last_count    = responses.size();
                    stable_for_ms = 0;
                }

                k_msleep(1);
            }

            for (const auto& response : responses)
            {
                std::cout << "Stream response: ";

                for (const auto byte : response)
                {
                    std::cout << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
                              << static_cast<int>(byte) << " ";
                }

                std::cout << std::dec << std::endl;
            }

            return responses;
        }

        /**
         * @brief Injects one logical MIDI input event into the system stub.
         *
         * @param event Event to process.
         */
        void process_incoming(const MIDIInputEvent& event)
        {
            if (_system == nullptr)
            {
                return;
            }

            _system->_hwa._builder_midi._hwaUsb.clear();
            _system->_hwa._builder_midi._hwaSerial.clear();

            _system->_hwa._builder_midi._hwaUsb._readPackets = midi_to_usb_packets(event);

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_SWITCHES
            EXPECT_CALL(_system->_hwa._builder_digital._builderSwitches._hwa, state(_))
                .Times(AnyNumber());
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ADC
            _system->_hwa._builder_analog._hwa.clear_read_count();
#endif

            k_poll_signal_raise(_system->_hwa._builder_midi._hwaUsb.data_available_signal(), 0);

            constexpr uint32_t MAX_WAIT_MS = 100;

            for (uint32_t elapsed = 0; elapsed < MAX_WAIT_MS; elapsed++)
            {
                if (_system->_hwa._builder_midi._hwaUsb._readPackets.empty())
                {
                    break;
                }

                k_msleep(1);
            }

            k_msleep(1);
        }

#ifdef PROJECT_TARGET_HW_TESTS_SUPPORTED
        /**
         * @brief Sends a SysEx request to a real device and decodes the response payload.
         *
         * @param request Serialized SysEx request.
         * @param is_get `true` when the response should be decoded as a read value.
         *
         * @return Decoded response value, or `-1` on failure.
         */
        int32_t send_request_to_device(std::vector<uint8_t>& request, bool is_get)
        {
            const auto response = send_raw_sysex_to_device(request);

            if (response.empty())
            {
                return -1;
            }

            if (is_get)
            {
                const auto merged = util::Conversion::Merge14Bit(response.at(response.size() - 3),
                                                                 response.at(response.size() - 2));
                return merged.value();
            }

            return response.at(4);
        }

        /**
         * @brief Sends a write-only SysEx request to a real device.
         *
         * @param request Serialized SysEx write request.
         *
         * @return `true` if the request was sent successfully, otherwise `false`.
         */
        bool send_write_request_to_device(std::vector<uint8_t>& request)
        {
            std::cout << "req: " << tests::vector_to_hex_string(request) << std::endl;

            if (!send_midi(send_midi_payload(request)))
            {
                std::cerr << "Failed to send write request" << std::endl;
                return false;
            }

            sleep_ms(WRITE_SETTLE_DELAY_MS);
            return true;
        }
#endif

        /**
         * @brief Sends a SysEx request to the system stub and decodes the response payload.
         *
         * @param request Serialized SysEx request.
         * @param is_get `true` when the response should be decoded as a read value.
         *
         * @return Decoded response value, or `0` when no response was received.
         */
        uint16_t send_request_to_stub(std::vector<uint8_t>& request, bool is_get)
        {
            const auto response = send_raw_sysex_to_stub(request);

            if (response.empty())
            {
                return 0;
            }

            if (is_get)
            {
                const auto merged = util::Conversion::Merge14Bit(response.at(response.size() - 3),
                                                                 response.at(response.size() - 2));
                return merged.value();
            }

            return response.at(4);
        }

        /** @brief Maps a global section enum to its SysEx configuration block. */
        static constexpr sys::Config::Block block(sys::Config::Section::Global)
        {
            return sys::Config::Block::Global;
        }

        /** @brief Maps a switch section enum to its SysEx configuration block. */
        static constexpr sys::Config::Block block(sys::Config::Section::Switch)
        {
            return sys::Config::Block::Switches;
        }

        /** @brief Maps an encoder section enum to its SysEx configuration block. */
        static constexpr sys::Config::Block block(sys::Config::Section::Encoder)
        {
            return sys::Config::Block::Encoders;
        }

        /** @brief Maps an analog section enum to its SysEx configuration block. */
        static constexpr sys::Config::Block block(sys::Config::Section::Analog)
        {
            return sys::Config::Block::Analog;
        }

        /** @brief Maps an OUTPUT section enum to its SysEx configuration block. */
        static constexpr sys::Config::Block block(sys::Config::Section::Outputs)
        {
            return sys::Config::Block::Outputs;
        }

        /** @brief Maps an I2C section enum to its SysEx configuration block. */
        static constexpr sys::Config::Block block(sys::Config::Section::I2c)
        {
            return sys::Config::Block::I2c;
        }

        /** @brief Maps a touchscreen section enum to its SysEx configuration block. */
        static constexpr sys::Config::Block block(sys::Config::Section::Touchscreen)
        {
            return sys::Config::Block::Touchscreen;
        }

        /**
         * @brief Collects all complete SysEx responses currently emitted by the system stub.
         *
         * @return Parsed SysEx response frames.
         */
        std::vector<std::vector<uint8_t>> collect_stub_sysex_responses()
        {
            std::vector<std::vector<uint8_t>> responses;
            std::vector<uint8_t>              current_response;

            if (_system == nullptr)
            {
                return responses;
            }

            for (const auto& packet : _system->_hwa._builder_midi._hwaUsb._writePackets)
            {
                if (!zlibs::utils::midi::is_sysex7_packet(packet))
                {
                    continue;
                }

                zlibs::utils::midi::write_ump_as_midi1_bytes(
                    packet,
                    0,
                    [&responses, &current_response](uint8_t byte)
                    {
                        current_response.push_back(byte);

                        if (!current_response.empty() && current_response.back() == 0xF7)
                        {
                            responses.push_back(current_response);
                            current_response.clear();
                        }

                        return true;
                    });
            }

            return responses;
        }

        sys::Builder*               _system = nullptr;
        [[maybe_unused]] const bool _use_hardware;

#ifdef PROJECT_TARGET_HW_TESTS_SUPPORTED
        /**
         * @brief Serializes a SysEx request payload for the `sendmidi` CLI.
         *
         * @param request Serialized SysEx request including `F0`/`F7`.
         *
         * @return CLI-ready payload string without framing bytes.
         */
        static std::string send_midi_payload(const std::vector<uint8_t>& request)
        {
            std::stringstream payload;
            payload << std::hex << std::setfill('0') << std::uppercase;

            for (size_t i = 1; i + 1 < request.size(); i++)
            {
                if (i != 1)
                {
                    payload << " ";
                }

                payload << std::setw(2) << static_cast<int>(request.at(i));
            }

            return payload.str();
        }

        /**
         * @brief Returns a unique temporary log-file path for helper processes.
         *
         * @param tag Path tag used to identify the file purpose.
         *
         * @return Temporary file path.
         */
        static std::string temporary_file_path(const std::string& tag)
        {
            return "/tmp/" + tag + "-" + std::to_string(tests::millis()) + ".log";
        }

        /**
         * @brief Trims leading and trailing ASCII whitespace.
         *
         * @param input Input string to trim.
         *
         * @return Trimmed string.
         */
        static std::string trim(const std::string& input)
        {
            const auto start = input.find_first_not_of(" \t\r\n");

            if (start == std::string::npos)
            {
                return "";
            }

            const auto end = input.find_last_not_of(" \t\r\n");
            return input.substr(start, end - start + 1);
        }

        /**
         * @brief Reads the full contents of a text file.
         *
         * @param path File path to read.
         *
         * @return File contents, or an empty string on failure.
         */
        static std::string read_file(const std::string& path)
        {
            std::ifstream stream(path);

            if (!stream.is_open())
            {
                return "";
            }

            std::stringstream buffer;
            buffer << stream.rdbuf();
            return buffer.str();
        }

        /**
         * @brief Sends a SysEx payload to the external MIDI device through `sendmidi`.
         *
         * @param payload CLI-ready payload string.
         *
         * @return `true` if the command succeeded, otherwise `false`.
         */
        static bool send_midi(const std::string& payload)
        {
            const auto cmd = "sendmidi dev \"" + std::string(USB_MIDI_DEVICE_NAME) + "\" hex syx " + payload;
            return tests::wsystem(cmd) == 0;
        }

        /**
         * @brief Starts a background `receivemidi` process writing to a log file.
         *
         * @param output_file Destination log file.
         * @param mode `receivemidi` output mode.
         *
         * @return Process ID, or `-1` on failure.
         */
        static int start_receive_midi(const std::string& output_file, const std::string& mode = "syx")
        {
            const auto cmd = "sh -c 'stdbuf -oL -eL receivemidi dev \"" + std::string(USB_MIDI_DEVICE_NAME) +
                             "\" " + mode + " > \"" + output_file + "\" 2>&1 & echo $!'";

            std::string pid_string;

            if (tests::wsystem(cmd, pid_string) != 0)
            {
                return -1;
            }

            const auto trimmed = trim(pid_string);

            if (trimmed.empty())
            {
                return -1;
            }

            return std::stoi(trimmed);
        }

        /**
         * @brief Stops a background `receivemidi` process.
         *
         * @param pid Process ID to terminate.
         */
        static void stop_receive_midi(int pid)
        {
            if (pid > 0)
            {
                tests::wsystem("kill " + std::to_string(pid) + " >/dev/null 2>&1");
            }
        }

        /**
         * @brief Waits until the receive helper reports that it is connected.
         *
         * @param output_file Log file produced by the helper.
         *
         * @return `true` once the helper is considered ready.
         */
        static bool wait_for_receive_midi_ready(const std::string& output_file)
        {
            constexpr const char* CONNECTED_MESSAGE = "Connected to MIDI input port";

            for (uint32_t elapsed = 0; elapsed < RECEIVE_CONNECT_TIMEOUT_MS; elapsed += RECEIVE_POLL_DELAY_MS)
            {
                const auto output = read_file(output_file);

                if (output.find(CONNECTED_MESSAGE) != std::string::npos)
                {
                    return true;
                }

                sleep_ms(RECEIVE_POLL_DELAY_MS);
            }

            sleep_ms(RECEIVE_STARTUP_DELAY_MS);
            return true;
        }

        /**
         * @brief Parses one `receivemidi` output line into a framed SysEx byte vector.
         *
         * @param input Output line to parse.
         *
         * @return Parsed SysEx frame.
         */
        static std::vector<uint8_t> parse_receive_midi_output_line(const std::string& input)
        {
            std::vector<uint8_t> result;

            // Start byte
            result.push_back(0xF0);

            // Regex for exactly 1–2 hex digits
            std::regex hex_byte_regex(R"(\b[0-9A-Fa-f]{1,2}\b)");

            auto begin = std::sregex_iterator(input.begin(), input.end(), hex_byte_regex);
            auto end   = std::sregex_iterator();

            for (auto it = begin; it != end; ++it)
            {
                std::string byte_str = it->str();

                uint32_t value = std::stoul(byte_str, nullptr, 16);
                result.push_back(static_cast<uint8_t>(value));
            }

            // End byte
            result.push_back(0xF7);

            return result;
        }

        /**
         * @brief Parses all `receivemidi` output lines into framed SysEx responses.
         *
         * @param output Raw `receivemidi` output.
         *
         * @return Parsed SysEx response frames.
         */
        static std::vector<std::vector<uint8_t>> parse_receive_midi_outputs(const std::string& output)
        {
            std::vector<std::vector<uint8_t>> responses;
            std::stringstream                 stream(output);
            std::string                       line;

            while (std::getline(stream, line))
            {
                responses.push_back(parse_receive_midi_output_line(line));
            }

            return responses;
        }

        /**
         * @brief Waits for any valid receive-helper response.
         *
         * @param output_file Helper log file.
         * @param timeout_ms Maximum wait time.
         *
         * @return First matching response frame, or an empty vector on timeout.
         */
        static std::vector<uint8_t> wait_for_receive_midi_response(const std::string& output_file, uint32_t timeout_ms)
        {
            return wait_for_receive_midi_response(output_file, {}, timeout_ms);
        }

        /**
         * @brief Waits for a receive-helper response matching the original request.
         *
         * @param output_file Helper log file.
         * @param request Original request used for correlation.
         * @param timeout_ms Maximum wait time.
         *
         * @return First matching response frame, or an empty vector on timeout.
         */
        static std::vector<uint8_t> wait_for_receive_midi_response(const std::string&          output_file,
                                                                   const std::vector<uint8_t>& request,
                                                                   uint32_t                    timeout_ms)
        {
            for (uint32_t elapsed = 0; elapsed < timeout_ms; elapsed += RECEIVE_POLL_DELAY_MS)
            {
                const auto output = read_file(output_file);

                if (!output.empty())
                {
                    const auto parsed = parse_receive_midi_outputs(output);

                    for (const auto& response : parsed)
                    {
                        if (matches_response_to_request(response, request))
                        {
                            return response;
                        }
                    }
                }

                sleep_ms(RECEIVE_POLL_DELAY_MS);
            }

            return {};
        }

        /**
         * @brief Waits for streamed helper output and returns it verbatim.
         *
         * @param output_file Helper log file.
         * @param timeout_ms Duration to wait before reading the file.
         *
         * @return Raw helper output.
         */
        static std::string wait_for_receive_midi_stream(const std::string& output_file, uint32_t timeout_ms)
        {
            sleep_ms(timeout_ms);
            return read_file(output_file);
        }

        /**
         * @brief Returns whether a parsed response matches the original request metadata.
         *
         * @param response Parsed SysEx response frame.
         * @param request Original request frame.
         *
         * @return `true` if the response matches, otherwise `false`.
         */
        static bool matches_response_to_request(const std::vector<uint8_t>& response, const std::vector<uint8_t>& request)
        {
            if (response.empty())
            {
                return false;
            }

            if (request.empty())
            {
                return true;
            }

            if (response.size() < 8 || request.size() < 8)
            {
                return false;
            }

            if ((response[0] != 0xF0) || (response[1] != request[1]) || (response[2] != request[2]) || (response[3] != request[3]))
            {
                return false;
            }

            if ((response[5] != request[5]) || (response[6] != request[6]))
            {
                return false;
            }

            if ((request.size() > 7) && (response[7] != request[7]))
            {
                return false;
            }

            if ((request.size() > 11) && (response.size() > 11))
            {
                return response[8] == request[8] &&
                       response[9] == request[9] &&
                       response[10] == request[10] &&
                       response[11] == request[11];
            }

            return true;
        }
#endif
    };
}    // namespace opendeck::tests
