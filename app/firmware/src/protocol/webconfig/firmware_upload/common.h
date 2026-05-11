/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace opendeck::protocol::webconfig
{
    /** @brief Bytes in a firmware-upload ACK frame. */
    constexpr inline size_t FIRMWARE_UPLOAD_ACK_SIZE = 7;

    /** @brief Commands used to upload dfu.bin over WebConfig. */
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
}    // namespace opendeck::protocol::webconfig
