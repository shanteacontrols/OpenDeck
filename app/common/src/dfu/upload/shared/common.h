/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/staged_update/shared/common.h"

#include "zlibs/utils/misc/bit.h"

#include <zephyr/sys/byteorder.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace opendeck::common::dfu::upload
{
    /** @brief Bytes in the fixed command field. */
    constexpr inline size_t COMMAND_SIZE = sizeof(uint8_t);

    /** @brief Bytes in the chunk-payload length field. */
    constexpr inline size_t CHUNK_LENGTH_SIZE = sizeof(uint16_t);

    /** @brief Bytes in one command-only firmware-upload frame. */
    constexpr inline size_t COMMAND_FRAME_SIZE = COMMAND_SIZE;

    /** @brief Maximum dfu.bin payload bytes carried by one upload chunk frame. */
    constexpr inline size_t CHUNK_SIZE = staged_update::HEADER_STORAGE_SIZE;

    /** @brief Bytes before chunk payload bytes in one chunk frame. */
    constexpr inline size_t CHUNK_FRAME_OVERHEAD = COMMAND_SIZE + CHUNK_LENGTH_SIZE;

    /** @brief Maximum firmware-upload command frame size accepted by OpenDeck transports. */
    constexpr inline size_t FRAME_SIZE = CHUNK_FRAME_OVERHEAD + CHUNK_SIZE;

    /** @brief Commands used to upload dfu.bin over OpenDeck transports. */
    enum class Command : uint8_t
    {
        Begin  = 0x01,
        Chunk  = 0x02,
        Finish = 0x03,
        Abort  = 0x04,
    };

    /** @brief Responses emitted by the firmware-upload flow. */
    enum class Response : uint8_t
    {
        Ack = 0x81,
    };

    /** @brief Result of one firmware-upload command. */
    enum class Status : uint8_t
    {
        Ok          = 0x00,
        Failed      = 0x01,
        Unsupported = 0x02,
        BadRequest  = 0x03,
    };

    /** @brief Decoded firmware-upload acknowledgement. */
    struct __packed Ack
    {
        Response response      = Response::Ack;
        Command  command       = Command::Begin;
        Status   status        = Status::Ok;
        uint32_t bytes_written = 0;
    };

    static_assert(sizeof(Ack) == (sizeof(Response) + sizeof(Command) + sizeof(Status) + sizeof(uint32_t)));

    /**
     * @brief Result of one handled firmware-upload command frame.
     */
    struct CommandResult
    {
        Ack  response = {};
        bool finished = false;
    };

    /**
     * @brief Parsed information about one firmware-upload frame prefix.
     */
    struct Frame
    {
        Command                  command  = Command::Begin;
        std::span<const uint8_t> payload  = {};
        size_t                   size     = 0;
        bool                     complete = false;
    };

    /**
     * @brief Builds a firmware-upload ACK frame.
     *
     * @param command Command being acknowledged.
     * @param status Result of the command.
     * @param bytes_written Number of accepted firmware payload bytes.
     *
     * @return Complete ACK frame.
     */
    constexpr inline Ack make_ack(Command command, Status status, uint32_t bytes_written)
    {
        return Ack{
            .response      = Response::Ack,
            .command       = command,
            .status        = status,
            .bytes_written = bytes_written,
        };
    }

    /**
     * @brief Encodes an ACK as raw transport bytes.
     *
     * @param ack ACK fields.
     *
     * @return Raw ACK frame bytes.
     */
    constexpr inline std::array<uint8_t, sizeof(Ack)> ack_to_bytes(const Ack& ack)
    {
        return {
            static_cast<uint8_t>(ack.response),
            static_cast<uint8_t>(ack.command),
            static_cast<uint8_t>(ack.status),
            static_cast<uint8_t>((ack.bytes_written >> (0U * zlibs::utils::misc::BYTE_BIT_COUNT)) & zlibs::utils::misc::BYTE_MASK),
            static_cast<uint8_t>((ack.bytes_written >> (1U * zlibs::utils::misc::BYTE_BIT_COUNT)) & zlibs::utils::misc::BYTE_MASK),
            static_cast<uint8_t>((ack.bytes_written >> (2U * zlibs::utils::misc::BYTE_BIT_COUNT)) & zlibs::utils::misc::BYTE_MASK),
            static_cast<uint8_t>((ack.bytes_written >> (3U * zlibs::utils::misc::BYTE_BIT_COUNT)) & zlibs::utils::misc::BYTE_MASK),
        };
    }

    /**
     * @brief Decodes a raw ACK frame.
     *
     * @param data Raw ACK bytes.
     *
     * @return Decoded ACK when the payload size matches, otherwise `std::nullopt`.
     */
    inline std::optional<Ack> ack_from_bytes(std::span<const uint8_t> data)
    {
        if (data.size() != sizeof(Ack))
        {
            return std::nullopt;
        }

        return Ack{
            .response      = static_cast<Response>(data[0]),
            .command       = static_cast<Command>(data[1]),
            .status        = static_cast<Status>(data[2]),
            .bytes_written = sys_get_le32(&data[3]),
        };
    }

    /**
     * @brief Returns the complete frame size for a chunk payload length.
     *
     * @param payload_size Bytes carried by the chunk.
     *
     * @return Complete chunk frame size.
     */
    constexpr inline size_t chunk_frame_size(size_t payload_size)
    {
        return CHUNK_FRAME_OVERHEAD + payload_size;
    }
}    // namespace opendeck::common::dfu::upload
