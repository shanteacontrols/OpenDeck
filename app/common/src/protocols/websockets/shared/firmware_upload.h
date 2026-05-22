/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace opendeck::common::protocols::websockets
{
    /** @brief Bytes in a firmware-upload ACK frame. */
    constexpr inline size_t FIRMWARE_UPLOAD_ACK_SIZE = 7;

    /** @brief Firmware-upload ACK frame bytes. */
    using FirmwareUploadAck = std::array<uint8_t, FIRMWARE_UPLOAD_ACK_SIZE>;

    /**
     * @brief Result of one handled firmware-upload command frame.
     */
    struct FirmwareUploadCommandResult
    {
        FirmwareUploadAck response = {};
        bool              finished = false;
    };

    /** @brief Maximum dfu.bin payload bytes carried by one upload chunk frame. */
    constexpr inline size_t FIRMWARE_UPLOAD_CHUNK_SIZE = 2048;

    /** @brief Maximum firmware-upload command frame size accepted by OpenDeck WebSockets. */
    constexpr inline size_t FIRMWARE_UPLOAD_FRAME_SIZE = 1 + FIRMWARE_UPLOAD_CHUNK_SIZE;

    /** @brief Commands used to upload dfu.bin over OpenDeck WebSockets. */
    enum class FirmwareUploadCommand : uint8_t
    {
        Begin  = 0x01,
        Chunk  = 0x02,
        Finish = 0x03,
        Abort  = 0x04,
    };

    /** @brief Responses emitted by the firmware-upload command handler. */
    enum class FirmwareUploadResponse : uint8_t
    {
        Ack = 0x81,
    };

    /** @brief Result of one firmware-upload command. */
    enum class FirmwareUploadStatus : uint8_t
    {
        Ok          = 0x00,
        Failed      = 0x01,
        Unsupported = 0x02,
        BadRequest  = 0x03,
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
    constexpr inline FirmwareUploadAck make_firmware_upload_ack(FirmwareUploadCommand command,
                                                                FirmwareUploadStatus  status,
                                                                uint32_t              bytes_written)
    {
        FirmwareUploadAck response = {
            static_cast<uint8_t>(FirmwareUploadResponse::Ack),
            static_cast<uint8_t>(command),
            static_cast<uint8_t>(status),
            static_cast<uint8_t>((bytes_written >> 0U) & 0xFFU),
            static_cast<uint8_t>((bytes_written >> 8U) & 0xFFU),
            static_cast<uint8_t>((bytes_written >> 16U) & 0xFFU),
            static_cast<uint8_t>((bytes_written >> 24U) & 0xFFU),
        };

        return response;
    }
}    // namespace opendeck::common::protocols::websockets
